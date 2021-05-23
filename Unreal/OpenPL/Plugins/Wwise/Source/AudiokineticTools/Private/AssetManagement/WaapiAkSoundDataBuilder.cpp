/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#include "WaapiAkSoundDataBuilder.h"

#include "AkAssetDatabase.h"
#include "AkAudioBank.h"
#include "AkAudioBankGenerationHelpers.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkInitBank.h"
#include "AkUnrealHelper.h"
#include "AkWaapiClient.h"
#include "AkWaapiUtils.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "AssetTools/Public/AssetToolsModule.h"
#include "Async/Async.h"
#include "Async/ParallelFor.h"
#include "HAL/Event.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Base64.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "WwiseProject/WwiseSoundBankInfoCache.h"

DECLARE_CYCLE_STAT(TEXT("AkSoundData - Waapi List Event"), STAT_WaapiListEvent, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Waapi List AuxBus"), STAT_WaapiListAuxBus, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Waapi Call"), STAT_WaapiCall, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Waapi Parse JSON response"), STAT_WaapiParseJSONResponse, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Waapi DoWork"), STAT_WaapiDoWork, STATGROUP_AkSoundDataSource);

DECLARE_CYCLE_STAT(TEXT("AkSoundData - Waapi Gather Platform Data"), STAT_WaapiGatherPlatformData, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Waapi Process JSON"), STAT_WaapiProcessJSON, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Waapi Bank data"), STAT_WaapiBankData, STATGROUP_AkSoundDataSource);

WaapiAkSoundDataBuilder::WaapiAkSoundDataBuilder(const InitParameters& InitParameter)
: AkSoundDataBuilder(InitParameter)
{
}

WaapiAkSoundDataBuilder::~WaapiAkSoundDataBuilder()
{
	TSharedPtr<FJsonObject> result;
	FAkWaapiClient::Get()->Unsubscribe(_generatedSubscriptionId, result);
	FAkWaapiClient::Get()->Unsubscribe(_generatedDoneSubscriptionId, result);

	FGenericPlatformProcess::ReturnSynchEventToPool(waitForGenerationDoneEvent);
}

void WaapiAkSoundDataBuilder::Init()
{
	AkSoundDataBuilder::Init();
	
	TSharedPtr<FJsonObject> result;
	TSharedRef<FJsonObject> generatedOptions = MakeShared<FJsonObject>();
	generatedOptions->SetBoolField(WwiseWaapiHelper::INFO_FILE, true);
	generatedOptions->SetBoolField(WwiseWaapiHelper::BANK_DATA, true);
	generatedOptions->SetBoolField(WwiseWaapiHelper::PLUGININFO_OPTIONS, true);
	auto soundBankGeneratedCallback = WampEventCallback::CreateRaw(this, &WaapiAkSoundDataBuilder::onSoundBankGenerated);
	FAkWaapiClient::Get()->Subscribe(ak::wwise::core::soundbank::generated, generatedOptions, soundBankGeneratedCallback, _generatedSubscriptionId, result);

	TSharedRef<FJsonObject> doneOptions = MakeShared<FJsonObject>();
	auto soundBankGenerationDoneCallback = WampEventCallback::CreateRaw(this, &WaapiAkSoundDataBuilder::onSoundBankGenerationDone);
	FAkWaapiClient::Get()->Subscribe(ak::wwise::core::soundbank::generationDone, doneOptions, soundBankGenerationDoneCallback, _generatedDoneSubscriptionId, result);

	waitForGenerationDoneEvent = FGenericPlatformProcess::GetSynchEventFromPool();
}

void WaapiAkSoundDataBuilder::DoWork()
{
	if (!_generatedSubscriptionId)
	{
		return;
	}

	createNotificationItem();

	loadAndWaitForAssetToLoad();

	auto& akAssetDatabase = AkAssetDatabase::Get();

	TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
	TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> result;

	if (FAkWaapiClient::Get()->Call(ak::wwise::core::remote::getConnectionStatus, args, options, result, -1))
	{
		bool isConnected = false;
		if (result->TryGetBoolField(WwiseWaapiHelper::IS_CONNECTED, isConnected) && isConnected)
		{
			notifyProfilingInProgress();
			return;
		}
	}

	AutoSetIsBuilding autoSetIsBuilding;

	auto start = FPlatformTime::Cycles64();

	auto cacheFile = AkUnrealHelper::GetWwiseSoundBankInfoCachePath();

	WwiseSoundBankInfoCache infoCache;

	if (FPaths::FileExists(cacheFile))
	{
		infoCache.Load(cacheFile);
	}

	SCOPE_CYCLE_COUNTER(STAT_WaapiDoWork);

	TArray<TSharedPtr<FJsonValue>> platformJsonArray;
	for (auto& platform : initParameters.Platforms)
	{
		platformJsonArray.Add(MakeShared<FJsonValueString>(platform));
	}
	args->SetArrayField(WwiseWaapiHelper::PLATFORMS, platformJsonArray);

	TSet<FString> platformsToBuild(initParameters.Platforms);
	TSet<FString> languagesToBuild;

	if (initParameters.Languages.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> languageJsonArray;
		for (auto& language : initParameters.Languages)
		{
			languageJsonArray.Add(MakeShared<FJsonValueString>(language.ID.ToString(EGuidFormats::DigitsWithHyphensInBraces)));
			languagesToBuild.Add(language.Name);
		}
		args->SetArrayField(WwiseWaapiHelper::LANGUAGES, languageJsonArray);
	}
	
	TArray<TSharedPtr<FJsonValue>> soundBanks;
	
	{
		SCOPE_CYCLE_COUNTER(STAT_WaapiListEvent);

		TArray<TSharedPtr<FJsonValue>> eventInclusionArray
		{
			MakeShared<FJsonValueString>(WwiseWaapiHelper::EVENT),
			MakeShared<FJsonValueString>(WwiseWaapiHelper::STRUCTURE)
		};

		AudioBankInfoMap audioBankInfoMap;

		auto future = Async(EAsyncExecution::TaskGraphMainThread, [this, &audioBankInfoMap, &platformsToBuild, &languagesToBuild, &infoCache] {
			fillAudioBankInfoMap(audioBankInfoMap, FillAudioBankInfoKind::GUID, platformsToBuild, languagesToBuild, &infoCache);
		});

		future.Wait();
		
		{
			FScopeLock autoEventLock(&akAssetDatabase.EventMap.CriticalSection);

			for (auto& eventEntry : akAssetDatabase.EventMap.TypeMap)
			{
				if (auto eventInstance = Cast<UAkAudioEvent>(eventEntry.Value.GetAsset()))
				{
					if (!eventInstance->RequiredBank && eventInstance->ID.IsValid())
					{
						if (!initParameters.SkipLanguages || (initParameters.SkipLanguages && eventInstance->LocalizedPlatformAssetDataMap.Num() == 0))
						{
							if (initParameters.BanksToGenerate.Num() == 0 || initParameters.BanksToGenerate.Contains(eventInstance->GetFName().ToString()))
							{
								TSharedPtr<FJsonObject> soundBankEntry = MakeShareable(new FJsonObject());
								soundBankEntry->SetStringField(WwiseWaapiHelper::NAME, AkUnrealHelper::GuidToBankName(eventEntry.Key));
								soundBankEntry->SetArrayField(WwiseWaapiHelper::EVENTS, { MakeShared<FJsonValueString>(eventInstance->ID.ToString(EGuidFormats::DigitsWithHyphensInBraces)) });
								soundBankEntry->SetArrayField(WwiseWaapiHelper::INCLUSIONS, eventInclusionArray);
								soundBankEntry->SetBoolField(WwiseWaapiHelper::REBUILD, eventInstance->NeedsRebuild(platformsToBuild, languagesToBuild, &infoCache));
								soundBanks.Add(MakeShared<FJsonValueObject>(soundBankEntry));
							}
						}
					}
				}
			}
		}

		for (auto& audioBankEntry : audioBankInfoMap)
		{
			TSharedPtr<FJsonObject> soundBankEntry = MakeShareable(new FJsonObject());
			soundBankEntry->SetStringField(WwiseWaapiHelper::NAME, audioBankEntry.Key);

			TArray<TSharedPtr<FJsonValue>> eventArray;
			for (auto& eventName : audioBankEntry.Value.Events)
			{
				eventArray.Add(MakeShared<FJsonValueString>(eventName));
			}
			soundBankEntry->SetArrayField(WwiseWaapiHelper::EVENTS, eventArray);

			TArray<TSharedPtr<FJsonValue>> auxBusArray;
			for (auto& auxBusName : audioBankEntry.Value.AuxBusses)
			{
				auxBusArray.Add(MakeShared<FJsonValueString>(auxBusName));
			}
			soundBankEntry->SetArrayField(WwiseWaapiHelper::AUX_BUSSES, auxBusArray);

			soundBankEntry->SetArrayField(WwiseWaapiHelper::INCLUSIONS, eventInclusionArray);
			soundBankEntry->SetBoolField(WwiseWaapiHelper::REBUILD, audioBankEntry.Value.NeedsRebuild);
			soundBanks.Add(MakeShared<FJsonValueObject>(soundBankEntry));
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_WaapiListAuxBus);
	
		TArray<TSharedPtr<FJsonValue>> auxBusInclusionArray
		{
			MakeShared<FJsonValueString>(WwiseWaapiHelper::STRUCTURE)
		};

		FScopeLock autoAuxBusLock(&akAssetDatabase.AuxBusMap.CriticalSection);

		for (auto& auxBusEntry : akAssetDatabase.AuxBusMap.TypeMap)
		{
			if (auto auxBusInstance = Cast<UAkAuxBus>(auxBusEntry.Value.GetAsset()))
			{
				if (!auxBusInstance->RequiredBank && auxBusInstance->ID.IsValid())
				{
					if (initParameters.BanksToGenerate.Num() == 0 || initParameters.BanksToGenerate.Contains(auxBusInstance->GetFName().ToString()))
					{
						TSharedPtr<FJsonObject> soundBankEntry = MakeShareable(new FJsonObject());
						soundBankEntry->SetStringField(WwiseWaapiHelper::NAME, AkUnrealHelper::GuidToBankName(auxBusEntry.Key));
						soundBankEntry->SetArrayField(WwiseWaapiHelper::AUX_BUSSES, { MakeShared<FJsonValueString>(auxBusInstance->ID.ToString(EGuidFormats::DigitsWithHyphensInBraces)) });
						soundBankEntry->SetArrayField(WwiseWaapiHelper::INCLUSIONS, auxBusInclusionArray);
						soundBankEntry->SetBoolField(WwiseWaapiHelper::REBUILD, auxBusInstance->NeedsRebuild(platformsToBuild, languagesToBuild, &infoCache));
						soundBanks.Add(MakeShared<FJsonValueObject>(soundBankEntry));
					}
				}
			}
		}
	}

	args->SetArrayField(WwiseWaapiHelper::SOUNDBANKS, soundBanks);
	args->SetBoolField(WwiseWaapiHelper::SKIP_LANGUAGES, initParameters.SkipLanguages);
	args->SetBoolField(WwiseWaapiHelper::REBUILD_INIT_BANK, akAssetDatabase.InitBank->NeedsRebuild(platformsToBuild, languagesToBuild, &infoCache));

	bool waapiCallSuccess = false;
	{
		SCOPE_CYCLE_COUNTER(STAT_WaapiCall);
		waapiCallSuccess = FAkWaapiClient::Get()->Call(ak::wwise::core::soundbank::generate, args, options, result, -1);
		if (!waapiCallSuccess)
		{
			auto message = result->GetStringField(WwiseWaapiHelper::MESSSAGE);
			UE_LOG(LogAkAudio, Error, TEXT("WAAPI Sound Data generation failed: %s"), *message);
		}
	}

	if (waapiCallSuccess)
	{
		waitForGenerationDoneEvent->Wait(FTimespan::FromSeconds(60));
	}

	FTaskGraphInterface::Get().WaitUntilTasksComplete(allParseTask);

	dispatchAndWaitMediaCookTasks();

	AutoSaveAssetsBlocking();

	bool totalSuccess = _generationSuccess && waapiCallSuccess;

	auto end = FPlatformTime::Cycles64();

	const TCHAR* message = nullptr;

	if (totalSuccess)
	{
		message = TEXT("WAAPI Sound Data Builder task was successful");
		notifyGenerationSucceeded();
	}
	else
	{
		message = TEXT("WAAPI Sound Data Builder task failed");
		notifyGenerationFailed();
	}

	UE_LOG(LogAkSoundData, Display, TEXT("%s and took %f seconds."), message, FPlatformTime::ToSeconds64(end - start));

	if (!IsRunningCommandlet())
	{
		AsyncTask(ENamedThreads::Type::GameThread, []
		{
			if (auto audioDevice = FAkAudioDevice::Get())
			{
				audioDevice->ReloadAllSoundData();
			}
		});
	}
}

void WaapiAkSoundDataBuilder::onSoundBankGenerationDone(uint64_t id, TSharedPtr<FJsonObject> responseJson)
{
	const TArray<TSharedPtr<FJsonValue>>* logs = nullptr;
	if (responseJson->TryGetArrayField(TEXT("logs"), logs))
	{
		for (auto& entry : *logs)
		{
			const TSharedPtr<FJsonObject>* jsonEntry = nullptr;
			if (entry->TryGetObject(jsonEntry))
			{
				const auto severity = jsonEntry->Get()->GetStringField(TEXT("severity"));
				const auto message = jsonEntry->Get()->GetStringField(WwiseWaapiHelper::MESSSAGE);

				FString platform = "";

				const TSharedPtr<FJsonObject>* jsonPlatform = nullptr;
				if (jsonEntry->Get()->TryGetObjectField(TEXT("platform"), jsonPlatform))
				{
					jsonPlatform->Get()->TryGetStringField(WwiseWaapiHelper::NAME, platform);
				}

				if (severity == TEXT("Message"))
				{
					UE_LOG(LogAkSoundData, Display, TEXT("%s: %s"), *platform, *message);
				}
				else if (severity == TEXT("Warning"))
				{
					UE_LOG(LogAkSoundData, Warning, TEXT("%s: %s"), *platform, *message);
				}
				else if (severity == TEXT("Error") || severity == TEXT("Fatal Error"))
				{
					_generationSuccess = false;
					UE_LOG(LogAkSoundData, Error, TEXT("%s: %s"), *platform, *message);
				}
			}
		}
	}
	else
	{
		FString outError;
		if (responseJson->TryGetStringField(TEXT("error"), outError))
		{
			_generationSuccess = false;

			UE_LOG(LogAkSoundData, Error, TEXT("%s"), *outError);
		}
	}

	waitForGenerationDoneEvent->Trigger();
}

void WaapiAkSoundDataBuilder::onSoundBankGenerated(uint64_t id, TSharedPtr<FJsonObject> responseJson)
{
	SCOPE_CYCLE_COUNTER(STAT_WaapiParseJSONResponse);

	auto& akAssetDatabase = AkAssetDatabase::Get();

	FString soundBankName;
	FString platformName;
	FString language;

	if (responseJson->HasField(WwiseWaapiHelper::SOUNDBANK))
	{
		TSharedPtr<FJsonObject> soundBankJson = responseJson->GetObjectField(WwiseWaapiHelper::SOUNDBANK);
		soundBankName = soundBankJson->GetStringField(WwiseWaapiHelper::NAME);
	}

	if (responseJson->HasField(WwiseWaapiHelper::PLATFORM))
	{
		TSharedPtr<FJsonObject> platformJson = responseJson->GetObjectField(WwiseWaapiHelper::PLATFORM);

		platformName = platformJson->GetStringField(WwiseWaapiHelper::NAME);
	}

	if (responseJson->HasField(WwiseWaapiHelper::LANGUAGE))
	{
		TSharedPtr<FJsonObject> languageJson = responseJson->GetObjectField(WwiseWaapiHelper::LANGUAGE);

		language = languageJson->GetStringField(WwiseWaapiHelper::NAME);

		{
			FScopeLock autoLock(&akAssetDatabase.InitBankLock);
			akAssetDatabase.InitBank->AvailableAudioCultures.AddUnique(language);
		}
	}

	if (responseJson->HasField(WwiseWaapiHelper::BANK_INFO))
	{
		auto infoJsonArray = responseJson->GetArrayField(WwiseWaapiHelper::BANK_INFO);
		if (infoJsonArray.Num() == 0)
			return;

		auto bankGuid = AkUnrealHelper::BankNameToGuid(soundBankName);

		auto infoJson = infoJsonArray[0]->AsObject();
		auto akAudioEventIt = akAssetDatabase.EventMap.Find(bankGuid);
		auto akAuxBusIt = akAssetDatabase.AuxBusMap.Find(bankGuid);
		auto akAudioBankIt = akAssetDatabase.BankMap.Find(bankGuid);

		auto sharedThis = SharedThis(this);

		if (akAudioEventIt)
		{
			struct EventRequiredData
			{
				UAkAudioEvent* akAudioEvent = nullptr;
				UAkAssetData* eventPlatformData = nullptr;
			};

			TSharedPtr<EventRequiredData> requiredData = MakeShared<EventRequiredData>();
			requiredData->akAudioEvent = Cast<UAkAudioEvent>(akAudioEventIt->GetAsset());

			auto fetchPlatformDataTask = FFunctionGraphTask::CreateAndDispatchWhenReady([requiredData, language, platformName]
			{
				if (requiredData->akAudioEvent)
				{
					requiredData->eventPlatformData = requiredData->akAudioEvent->FindOrAddAssetData(platformName, language);
				}
			}, GET_STATID(STAT_WaapiGatherPlatformData), nullptr, ENamedThreads::GameThread);

			auto parseTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, requiredData, platformName, language, responseJson, infoJson]
			{
				if (requiredData->akAudioEvent && requiredData->eventPlatformData)
				{
					bool changed = false;

					changed |= sharedThis->parseBankData(requiredData->eventPlatformData, responseJson, &(requiredData->akAudioEvent->BulkDataWriteLock));
					changed |= sharedThis->parseSoundBankInfo(requiredData->akAudioEvent, requiredData->eventPlatformData, platformName, language, infoJson, false);

					if (changed)
					{
						sharedThis->markAssetDirty(requiredData->akAudioEvent);
						sharedThis->markAssetDirty(requiredData->eventPlatformData);
					}
				}
			}, GET_STATID(STAT_WaapiProcessJSON), fetchPlatformDataTask);

			{
				FScopeLock autoLock(&parseTasksLock);
				allParseTask.Add(parseTask);
			}
		}
		else if (akAuxBusIt)
		{
			auto akAuxBus = Cast<UAkAuxBus>(akAuxBusIt->GetAsset());
			auto parseTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, akAuxBus, platformName, responseJson, infoJson] {
				if (akAuxBus)
				{
					UAkAssetData* auxBusPlatformData = akAuxBus->FindOrAddAssetData(platformName, FString());

					if (auxBusPlatformData)
					{
						bool changed = false;

						changed |= sharedThis->parseBankData(auxBusPlatformData, responseJson, &(akAuxBus->BulkDataWriteLock));
						changed |= sharedThis->parseSoundBankInfo(akAuxBus, auxBusPlatformData, platformName, FString(), infoJson, false);

						if (changed)
						{
							sharedThis->markAssetDirty(akAuxBus);
						}
					}
				}
			}, GET_STATID(STAT_WaapiProcessJSON));

			{
				FScopeLock autoLock(&parseTasksLock);
				allParseTask.Add(parseTask);
			}
		}
		else if (akAudioBankIt)
		{
			struct AudioBankRequiredData
			{
				UAkAudioBank* akAudioBank = nullptr;
				UAkAssetData* audioBankPlatformData = nullptr;
			};

			TSharedPtr<AudioBankRequiredData> requiredData = MakeShared<AudioBankRequiredData>();
			requiredData->akAudioBank = Cast<UAkAudioBank>(akAudioBankIt->GetAsset());

			auto fetchPlatformDataTask = FFunctionGraphTask::CreateAndDispatchWhenReady([requiredData, language, platformName]
			{
				if (requiredData->akAudioBank)
				{
					requiredData->audioBankPlatformData = requiredData->akAudioBank->FindOrAddAssetData(platformName, language);
				}
			}, GET_STATID(STAT_WaapiGatherPlatformData), nullptr, ENamedThreads::GameThread);

			auto parseTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, requiredData, platformName, language, responseJson, infoJson]
			{
				if (requiredData->akAudioBank && requiredData->audioBankPlatformData)
				{
					bool changed = false;

					changed |= sharedThis->parseBankData(requiredData->audioBankPlatformData, responseJson, &(requiredData->akAudioBank->BulkDataWriteLock));
					changed |= sharedThis->parseSoundBankInfo(requiredData->akAudioBank, requiredData->audioBankPlatformData, platformName, language, infoJson, false);

					if (changed)
					{
						sharedThis->markAssetDirty(requiredData->akAudioBank);
						sharedThis->markAssetDirty(requiredData->audioBankPlatformData);
					}
				}
			}, GET_STATID(STAT_WaapiProcessJSON), fetchPlatformDataTask);

			{
				FScopeLock autoLock(&parseTasksLock);
				allParseTask.Add(parseTask);
			}
		}
		else if (soundBankName == AkInitBankName)
		{
			auto parseTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, platformName, responseJson, infoJson, InitBank = akAssetDatabase.InitBank] {
				UAkAssetData* initBankPlatformData = InitBank->FindOrAddAssetData(platformName, FString());

				if (initBankPlatformData)
				{
					bool changed = false;

					changed |= sharedThis->parseBankData(initBankPlatformData, responseJson, &(InitBank->BulkDataWriteLock));
					changed |= sharedThis->parseSoundBankInfo(InitBank, initBankPlatformData, platformName, FString(), infoJson, true);

					if (responseJson->HasField(WwiseWaapiHelper::PLUGININFO_RESPONSE))
					{
						changed |= sharedThis->parsePluginInfo(InitBank, platformName, responseJson->GetObjectField(WwiseWaapiHelper::PLUGININFO_RESPONSE));
					}

					if (changed)
					{
						sharedThis->markAssetDirty(InitBank);
					}
				}
			}, GET_STATID(STAT_WaapiProcessJSON));

			{
				FScopeLock autoLock(&parseTasksLock);
				allParseTask.Add(parseTask);
			}
		}
	}
}

bool WaapiAkSoundDataBuilder::parseBankData(UAkAssetData* AssetData, TSharedPtr<FJsonObject> ResponseJson, FCriticalSection* DataLock)
{
	bool changed = false;
	if (ResponseJson->HasField(WwiseWaapiHelper::BANK_DATA))
	{
		SCOPE_CYCLE_COUNTER(STAT_WaapiBankData);

		auto& bankDataJson = ResponseJson->GetObjectField(WwiseWaapiHelper::BANK_DATA);

		int32 dataSize = bankDataJson->GetIntegerField(WwiseWaapiHelper::SIZE);

		FString stringData = bankDataJson->GetStringField(WwiseWaapiHelper::DATA);

		TArray<uint8> newBankData;
		newBankData.SetNumUninitialized(dataSize);
		FBase64::Decode(*stringData, stringData.Len(), reinterpret_cast<uint8*>(newBankData.GetData()));

		return fillBankDataInAsset(AssetData, newBankData, DataLock);
	}

	return false;
}
