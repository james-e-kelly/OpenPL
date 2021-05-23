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

#include "WwiseConsoleAkSoundDataBuilder.h"

#include "AkAudioBank.h"
#include "AkAudioBankGenerationHelpers.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkGroupValue.h"
#include "AkInitBank.h"
#include "AkRtpc.h"
#include "AkTrigger.h"
#include "AkUnrealHelper.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "Async/Async.h"
#include "Async/ParallelFor.h"
#include "Core/Public/Modules/ModuleManager.h"
#include "DirectoryWatcherModule.h"
#include "HAL/PlatformFileManager.h"
#include "IDirectoryWatcher.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ToolBehavior/AkToolBehavior.h"
#include "WwiseProject/WwiseSoundBankInfoCache.h"

DECLARE_CYCLE_STAT(TEXT("AkSoundDataBuilder - Watcher ProcessFile"), STAT_WatcherProcessFile, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundDataBuilder - Fetch Event Platform Data"), STAT_FetchEventPlatformData, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundDataBuilder - Read bank data"), STAT_ReadBankData, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundDataBuilder - Read bank definition file"), STAT_ReadBankDefinitionFile, STATGROUP_AkSoundDataSource);

namespace
{
	constexpr auto AkPluginInfoFileName = TEXT("PluginInfo");
}

WwiseConsoleAkSoundDataBuilder::WwiseConsoleAkSoundDataBuilder(const InitParameters& InitParameter)
: AkSoundDataBuilder(InitParameter)
{
	if (!AkUnrealHelper::IsUsingEventBased())
	{
		 auto AssetDatabaseInitTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		 	auto& AssetDatabase = AkAssetDatabase::Get();
		 	AssetDatabase.Clear();
		 	AssetDatabase.Init();
		 }, GET_STATID(STAT_WatcherProcessFile), nullptr, ENamedThreads::GameThread);

		 FTaskGraphInterface::Get().WaitUntilTaskCompletes(AssetDatabaseInitTask);
	}
}

WwiseConsoleAkSoundDataBuilder::~WwiseConsoleAkSoundDataBuilder()
{
}

void WwiseConsoleAkSoundDataBuilder::Init()
{
	AkSoundDataBuilder::Init();

	watchDirectory = AkUnrealHelper::GetGeneratedSoundBanksFolder();
	FPaths::NormalizeDirectoryName(watchDirectory);

	if (!platformFile->DirectoryExists(*watchDirectory))
	{
		platformFile->CreateDirectory(*watchDirectory);
	}

	directoryWatcherModule = &FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));

	if (!IsRunningCommandlet())
	{
		directoryWatcherModule->Get()->RegisterDirectoryChangedCallback_Handle(
			watchDirectory
			, IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &WwiseConsoleAkSoundDataBuilder::onDirectoryWatcher)
			, directoryWatcherDelegateHandle
		);
	}
}

void WwiseConsoleAkSoundDataBuilder::DoWork()
{
	AutoSetIsBuilding autoSetIsBuilding;

	auto now = FDateTime::UtcNow();

	auto start = FPlatformTime::Cycles64();

	if (!IsRunningCommandlet())
	{
		createNotificationItem();
	}

	loadAndWaitForAssetToLoad();

	bool wwiseConsoleSucces = runWwiseConsole();

	if (!IsRunningCommandlet())
	{
		directoryWatcherModule->Get()->UnregisterDirectoryChangedCallback_Handle(watchDirectory, directoryWatcherDelegateHandle);
	}

	FTaskGraphInterface::Get().WaitUntilTasksComplete(allWatcherTasks);

	FGraphEventArray allProcessTasks;

	for (auto& currentPlatform : initParameters.Platforms)
	{
		auto folderToParse = FPaths::Combine(watchDirectory, currentPlatform);

		TArray<FString> filesToParse;
		platformFile->FindFilesRecursively(filesToParse, *folderToParse, TEXT(".bnk"));
		platformFile->FindFilesRecursively(filesToParse, *folderToParse, TEXT(".json"));

		for (auto& fullPath : filesToParse)
		{
			if (!processedPaths.Contains(fullPath))
			{
				auto processFileTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this, fullPath]() {
						processFile(fullPath, watchDirectory);
				}, GET_STATID(STAT_WatcherProcessFile), nullptr, ENamedThreads::AnyThread);

				allProcessTasks.Add(processFileTask);
			}
		}
	}

	if (AkUnrealHelper::IsUsingEventBased())
	{
		notifyCookingData();
	}

	FTaskGraphInterface::Get().WaitUntilTasksComplete(allProcessTasks);

	FTaskGraphInterface::Get().WaitUntilTasksComplete(allReadTasks);

	dispatchAndWaitMediaCookTasks();

	AutoSaveAssetsBlocking();

	if (!wwiseConsoleSucces)
	{
		notifyGenerationFailed();
	}
	else
	{
		notifyGenerationSucceeded();
	}

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

	auto end = FPlatformTime::Cycles64();

	UE_LOG(LogAkSoundData, Display, TEXT("WwiseConsole Sound Data Builder task took %f seconds."), FPlatformTime::ToSeconds64(end - start));
}

void WwiseConsoleAkSoundDataBuilder::onDirectoryWatcher(const TArray<struct FFileChangeData>& ChangedFiles)
{
	TSet<FString> uniqueFiles;
	for (auto& File : ChangedFiles)
	{
		if (File.Action == FFileChangeData::FCA_Added || File.Action == FFileChangeData::FCA_Modified)
		{
			if (FPaths::GetExtension(File.Filename) == TEXT("json"))
			{
				uniqueFiles.Add(File.Filename);
			}

			if (FPaths::GetExtension(File.Filename) == TEXT("txt"))
			{
				uniqueFiles.Add(FPaths::ChangeExtension(File.Filename, TEXT(".bnk")));
			}
		}
	}

	for (auto& fullPath : uniqueFiles)
	{
		FPaths::NormalizeDirectoryName(fullPath);

		auto processFileTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis = SharedThis(this), fullPath]() {
			sharedThis->processFile(fullPath, sharedThis->watchDirectory);
		},
			GET_STATID(STAT_WatcherProcessFile),
			nullptr,
			ENamedThreads::AnyThread
		);

		processedPaths.Add(fullPath);

		allWatcherTasks.Add(processFileTask);
	}
}

bool WwiseConsoleAkSoundDataBuilder::runWwiseConsole()
{
	bool success = true;

	auto cacheFile = AkUnrealHelper::GetWwiseSoundBankInfoCachePath();

	WwiseSoundBankInfoCache infoCache;

	if (FPaths::FileExists(cacheFile))
	{
		infoCache.Load(cacheFile);
	}

	bool isUsingNewAssetManagament = AkUnrealHelper::IsUsingEventBased();

	auto tempDefinitionFile = FPaths::CreateTempFilename(FPlatformProcess::UserTempDir(), TEXT("CookUAkAudioEvent"), TEXT(".txt"));
	ON_SCOPE_EXIT{
		platformFile->DeleteFile(*tempDefinitionFile);
	};

	auto tempBankFile = FPaths::CreateTempFilename(FPlatformProcess::UserTempDir(), TEXT("WwiseConsoleBankList"), TEXT(".txt"));
	ON_SCOPE_EXIT{
		platformFile->DeleteFile(*tempBankFile);
	};

	FString wwiseConsoleCommand = !overrideWwiseConsolePath.IsEmpty() ? overrideWwiseConsolePath : AkAudioBankGenerationHelper::GetWwiseConsoleApplicationPath();

	FString wwiseConsoleArguments;
#if PLATFORM_MAC
	wwiseConsoleArguments = wwiseConsoleCommand + TEXT(" ");
	wwiseConsoleCommand = TEXT("/bin/sh");
#endif
	wwiseConsoleArguments += FString::Printf(TEXT("generate-soundbank \"%s\" --use-stable-guid --import-definition-file \"%s\""),
		*platformFile->ConvertToAbsolutePathForExternalAppForWrite(*AkUnrealHelper::GetWwiseProjectPath()),
		*platformFile->ConvertToAbsolutePathForExternalAppForWrite(*tempDefinitionFile)
	);

	auto generatedSoundBanksPath = AkUnrealHelper::GetGeneratedSoundBanksFolder();

	TSet<FString> platformsToBuild;
	TSet<FString> languagesToBuild;

	for (auto& platform : initParameters.Platforms)
	{
		wwiseConsoleArguments += FString::Printf(TEXT(" --platform \"%s\""), *platform);
		wwiseConsoleArguments += FString::Printf(TEXT(" --soundbank-path %s \"%s\""), *platform, *FPaths::Combine(generatedSoundBanksPath, platform));
		platformsToBuild.Add(platform);
	}

	if (initParameters.SkipLanguages)
	{
		wwiseConsoleArguments += TEXT(" --skip-languages");
	}
	else
	{
		for (auto& language : initParameters.Languages)
		{
			wwiseConsoleArguments += FString::Printf(TEXT(" --language \"%s\""), *language.Name);
			languagesToBuild.Add(language.Name);
		}
	}

	AudioBankInfoMap audioBankInfoMap;

	auto future = Async(EAsyncExecution::TaskGraphMainThread, [this, &audioBankInfoMap, &platformsToBuild, &languagesToBuild, &infoCache] {
		return fillAudioBankInfoMap(audioBankInfoMap, FillAudioBankInfoKind::AssetName, platformsToBuild, languagesToBuild, &infoCache).Replace(TEXT(" "), LINE_TERMINATOR);
	});

	FString BankNamesToGenerate = future.Get();

	if (!isUsingNewAssetManagament && !BankNamesToGenerate.IsEmpty())
	{
		TUniquePtr<IFileHandle> fileWriter(platformFile->OpenWrite(*tempBankFile));
		FTCHARToUTF8 utf8(*BankNamesToGenerate);
		fileWriter->Write(reinterpret_cast<const uint8*>(utf8.Get()), utf8.Length());
		fileWriter->Flush();

		wwiseConsoleArguments += FString::Printf(TEXT(" --bank \"%s\""), *tempBankFile);
	}

	UE_LOG(LogAkSoundData, Display, TEXT("Running %s %s"), *wwiseConsoleCommand, *wwiseConsoleArguments);

	auto& akAssetDatabase = AkAssetDatabase::Get();

	{
		TUniquePtr<IFileHandle> fileWriter(platformFile->OpenWrite(*tempDefinitionFile));

		if (isUsingNewAssetManagament)
		{
			FScopeLock autoEventLock(&akAssetDatabase.EventMap.CriticalSection);

			for (auto& eventMapEntry : akAssetDatabase.EventMap.TypeMap)
			{
				auto eventInstance = Cast<UAkAudioEvent>(eventMapEntry.Value.GetAsset());
				if (eventInstance && !eventInstance->RequiredBank)
				{
					if (initParameters.BanksToGenerate.Num() == 0 || initParameters.BanksToGenerate.Contains(eventInstance->GetFName().ToString()))
					{
						auto bankName = AkUnrealHelper::GuidToBankName(eventMapEntry.Key);
						auto line = bankName + TEXT("\t\"") + eventInstance->GetName() + TEXT("\"\tEvent\tStructure") + LINE_TERMINATOR;
						FTCHARToUTF8 utf8(*line);
						fileWriter->Write(reinterpret_cast<const uint8*>(utf8.Get()), utf8.Length());

						if (eventInstance->NeedsRebuild(platformsToBuild, languagesToBuild, &infoCache))
						{
							prepareRebuild(bankName, generatedSoundBanksPath);
						}
					}
				}
			}
		}

		const auto audioBankEventIncludes = AkToolBehavior::Get()->WwiseConsoleAkSoundDataBuilder_AudioBankEventIncludes();
		const auto audioBankAuxBusIncludes = AkToolBehavior::Get()->WwiseConsoleAkSoundDataBuilder_AudioBankAuxBusIncludes();

		for (auto& audioBankEntry : audioBankInfoMap)
		{
			for (auto& eventEntry : audioBankEntry.Value.Events)
			{
				auto line = audioBankEntry.Key + TEXT("\t\"") + eventEntry + audioBankEventIncludes + LINE_TERMINATOR;
				FTCHARToUTF8 utf8(*line);
				fileWriter->Write(reinterpret_cast<const uint8*>(utf8.Get()), utf8.Length());
			}

			for (auto& auxBusEntry : audioBankEntry.Value.AuxBusses)
			{
				auto line = audioBankEntry.Key + TEXT("\t-AuxBus\t\"") + auxBusEntry + audioBankAuxBusIncludes + LINE_TERMINATOR;
				FTCHARToUTF8 utf8(*line);
				fileWriter->Write(reinterpret_cast<const uint8*>(utf8.Get()), utf8.Length());
			}

			if (isUsingNewAssetManagament && audioBankEntry.Value.NeedsRebuild)
			{
				prepareRebuild(audioBankEntry.Key, generatedSoundBanksPath);
			}
		}

		if (isUsingNewAssetManagament)
		{
			FScopeLock autoAuxBusLock(&akAssetDatabase.AuxBusMap.CriticalSection);

			for (auto& auxBusMapEntry : akAssetDatabase.AuxBusMap.TypeMap)
			{
				auto auxBusInstance = Cast<UAkAuxBus>(auxBusMapEntry.Value.GetAsset());
				if (auxBusInstance && !auxBusInstance->RequiredBank)
				{
					if (initParameters.BanksToGenerate.Num() == 0 || initParameters.BanksToGenerate.Contains(auxBusInstance->GetFName().ToString()))
					{
						auto bankName = AkUnrealHelper::GuidToBankName(auxBusMapEntry.Key);
						auto line = bankName + TEXT("\t-AuxBus\t\"") + auxBusInstance->GetName() + TEXT("\"\tStructure") + LINE_TERMINATOR;
						FTCHARToUTF8 utf8(*line);
						fileWriter->Write(reinterpret_cast<const uint8*>(utf8.Get()), utf8.Length());

						if (auxBusInstance->NeedsRebuild(platformsToBuild, languagesToBuild, &infoCache))
						{
							prepareRebuild(bankName, generatedSoundBanksPath);
						}
					}
				}
			}
		}
		fileWriter->Flush();
	}

	// Create a pipe for the child process's STDOUT.
	int32 ReturnCode = 0;
	void* WritePipe = nullptr;
	void* ReadPipe = nullptr;
	FPlatformProcess::CreatePipe(ReadPipe, WritePipe);
	ON_SCOPE_EXIT {
		FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
	};

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*wwiseConsoleCommand, *wwiseConsoleArguments, true, true, true, nullptr, 0, nullptr, WritePipe);
	if (ProcHandle.IsValid())
	{
		FString NewLine;
		FPlatformProcess::Sleep(0.1f);
		// Wait for it to finish and get return code
		while (FPlatformProcess::IsProcRunning(ProcHandle))
		{
			NewLine = FPlatformProcess::ReadPipe(ReadPipe);
			if (NewLine.Len() > 0)
			{
				UE_LOG(LogAkSoundData, Display, TEXT("%s"), *NewLine);
				NewLine.Empty();
			}
			FPlatformProcess::Sleep(0.25f);
		}

		NewLine = FPlatformProcess::ReadPipe(ReadPipe);
		if (NewLine.Len() > 0)
		{
			UE_LOG(LogAkSoundData, Display, TEXT("%s"), *NewLine);
		}

		FPlatformProcess::GetProcReturnCode(ProcHandle, &ReturnCode);

		switch (ReturnCode)
		{
		case 2:
			UE_LOG(LogAkSoundData, Warning, TEXT("WwiseConsole completed with warnings."));
			break;
		case 0:
			UE_LOG(LogAkSoundData, Display, TEXT("WwiseConsole successfully completed."));
			break;
		default:
			UE_LOG(LogAkSoundData, Error, TEXT("WwiseConsole failed with error %d."), ReturnCode);
			success = false;
			break;
		}
	}
	else
	{
		ReturnCode = -1;
		// Most chances are the path to the .exe or the project were not set properly in GEditorIni file.
		UE_LOG(LogAkSoundData, Error, TEXT("Failed to run WwiseConsole: %s %s"), *wwiseConsoleCommand, *wwiseConsoleArguments);
	}

	return success;
}

void WwiseConsoleAkSoundDataBuilder::prepareRebuild(const FString& BankName, const FString& GeneratedSoundBanksPath)
{
	auto fileName = FString::Printf(TEXT("%s.json"), *BankName);

	for (auto& platform : initParameters.Platforms)
	{
		auto basePath = FPaths::Combine(GeneratedSoundBanksPath, platform);
		FPaths::NormalizeDirectoryName(basePath);

		auto pathToDelete = FPaths::Combine(basePath, fileName);
		FPaths::NormalizeDirectoryName(pathToDelete);
		platformFile->DeleteFile(*pathToDelete);

		auto& languagesArray = initParameters.Languages.Num() > 0 ? initParameters.Languages : wwiseProjectInfo.SupportedLanguages();

		for (auto& language : languagesArray)
		{
			pathToDelete = FPaths::Combine(basePath, language.Name, fileName);
			FPaths::NormalizeDirectoryName(pathToDelete);
			platformFile->DeleteFile(*pathToDelete);
		}
	}
}

TSharedPtr<FJsonObject> WwiseConsoleAkSoundDataBuilder::readJsonFile(const FString& JsonFileName)
{
	TSharedPtr<FJsonObject> jsonObject;

	FString jsonFileContent;
	if (!FFileHelper::LoadFileToString(jsonFileContent, *JsonFileName))
	{
		UE_LOG(LogAkSoundData, Warning, TEXT("Unable to parse JSON file: %s"), *JsonFileName);
		return jsonObject;
	}

	auto jsonReader = TJsonReaderFactory<>::Create(jsonFileContent);
	if (!FJsonSerializer::Deserialize(jsonReader, jsonObject) || !jsonObject.IsValid())
	{
		UE_LOG(LogAkSoundData, Warning, TEXT("Unable to parse JSON file: %s"), *JsonFileName);
		return jsonObject;
	}

	return jsonObject;
}

bool WwiseConsoleAkSoundDataBuilder::readBankData(UAkAssetData* AssetData, const FString& BankFile, IPlatformFile& FileManager, FCriticalSection* DataLock)
{
	TUniquePtr<IFileHandle> bankFileReader(FileManager.OpenRead(*BankFile, true));
	if (!bankFileReader)
	{
		return false;
	}

	auto fileSize = bankFileReader->Size();
	if (fileSize == 0)
	{
		return false;
	}


	TArray<uint8> newBankData;
	newBankData.SetNumUninitialized(fileSize);
	bankFileReader->Read(newBankData.GetData(), fileSize);

	return fillBankDataInAsset(AssetData, newBankData, DataLock);
}

template<typename MainAsset, typename PlatformAsset>
bool WwiseConsoleAkSoundDataBuilder::readBankDefinitionFile(MainAsset* mainAsset, PlatformAsset* platformAsset, const FString& platform, const FString& language, const FString& jsonFile, bool isInitBank)
{
	TSharedPtr<FJsonObject> jsonObject = readJsonFile(jsonFile);

	if (!jsonObject.IsValid())
	{
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> soundBankArray = jsonObject->GetObjectField("SoundBanksInfo")->GetArrayField("SoundBanks");
	TSharedPtr<FJsonObject> soundBankData = soundBankArray[0]->AsObject();

	return parseSoundBankInfo(mainAsset, platformAsset, platform, language, soundBankData, isInitBank);
}

bool WwiseConsoleAkSoundDataBuilder::readPluginInfo(UAkInitBank* InitBank, const FString& Platform, const FString& PluginInfoFileName)
{
	TSharedPtr<FJsonObject> jsonObject = readJsonFile(PluginInfoFileName);

	if (!jsonObject.IsValid())
	{
		return false;
	}

	auto& pluginInfoJsonObject = jsonObject->GetObjectField("PluginInfo");

	return parsePluginInfo(InitBank, Platform, pluginInfoJsonObject);
}


void WwiseConsoleAkSoundDataBuilder::processFile(const FString& FullPath, const FString& GeneratedSoundBanksFolder)
{
	FString relativePath = FullPath;
	FPaths::NormalizeDirectoryName(relativePath);
	relativePath.RemoveFromStart(GeneratedSoundBanksFolder);

	FString directory;
	FString fileName;
	FString extension;

	FPaths::Split(relativePath, directory, fileName, extension);

	bool isBank = (extension == TEXT("bnk"));
	bool isDefinitionFile = (extension == TEXT("json"));
	if (!isBank && !isDefinitionFile)
	{
		return;
	}

	bool isUsingNewAssetManagement = AkUnrealHelper::IsUsingEventBased();

	if (!isUsingNewAssetManagement && isBank)
	{
		return;
	}

	TArray<FString> parts;
	directory.ParseIntoArray(parts, TEXT("/"), true);

	FString language;
	if (isUsingNewAssetManagement && parts.Num() == 2)
	{
		language = parts[1];

		FScopeLock autoLock(&AkAssetDatabase::Get().InitBankLock);
		AkAssetDatabase::Get().InitBank->AvailableAudioCultures.AddUnique(language);
	}

	FString platform = parts[0];

	auto sharedThis = SharedThis(this);

	FAssetData const* akAudioEventIt = nullptr;
	FAssetData const* akAuxBusIt = nullptr;
	FAssetData const* akAudioBankIt = nullptr;

	FAssetData foundAkAudioBank;

	if (isUsingNewAssetManagement)
	{
		auto bankGuid = AkUnrealHelper::BankNameToGuid(fileName);

		akAudioEventIt = AkAssetDatabase::Get().EventMap.Find(bankGuid);
		akAuxBusIt = AkAssetDatabase::Get().AuxBusMap.Find(bankGuid);
		akAudioBankIt = AkAssetDatabase::Get().BankMap.Find(bankGuid);
	}
	else
	{
		auto foundAssets = AkAssetDatabase::Get().BankMap.FindByName(fileName);
		if (foundAssets.Num() > 0)
		{
			foundAkAudioBank = foundAssets[0];
			akAudioBankIt = &foundAkAudioBank;
		}
	}

	if (akAudioEventIt)
	{
		struct EventRequiredData
		{
			UAkAudioEvent* akAudioEvent = nullptr;
			UAkAssetData* eventPlatformData = nullptr;
		};

		TSharedPtr<EventRequiredData> requiredData = MakeShared<EventRequiredData>();
		requiredData->akAudioEvent = Cast<UAkAudioEvent>(akAudioEventIt->GetAsset());

		auto fetchPlatformDataTask = FFunctionGraphTask::CreateAndDispatchWhenReady([requiredData, language, platform]
			{
				if (requiredData->akAudioEvent)
				{
					requiredData->eventPlatformData = requiredData->akAudioEvent->FindOrAddAssetData(platform, language);
				}
			}, GET_STATID(STAT_FetchEventPlatformData), nullptr, ENamedThreads::GameThread);

		if (isBank)
		{
			auto readBankDataTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, FullPath, platform, requiredData]
				{
					if (requiredData->akAudioEvent && requiredData->eventPlatformData)
					{
						if (sharedThis->readBankData(requiredData->eventPlatformData, FullPath, *sharedThis->platformFile, &(requiredData->akAudioEvent->BulkDataWriteLock)))
						{
							sharedThis->markAssetDirty(requiredData->akAudioEvent);
							sharedThis->markAssetDirty(requiredData->eventPlatformData);
						}
					}
				}, GET_STATID(STAT_ReadBankData), fetchPlatformDataTask, ENamedThreads::GameThread);

			{
				FScopeLock autoLock(&readTaskLock);
				allReadTasks.Add(readBankDataTask);
			}
		}
		else
		{
			auto parseJsonTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, FullPath, platform, language, requiredData]
				{
					if (requiredData->akAudioEvent && requiredData->eventPlatformData)
					{
						if (sharedThis->readBankDefinitionFile(requiredData->akAudioEvent, requiredData->eventPlatformData, platform, language, FullPath, false))
						{
							sharedThis->markAssetDirty(requiredData->akAudioEvent);
							sharedThis->markAssetDirty(requiredData->eventPlatformData);
						}
					}
				}, GET_STATID(STAT_ReadBankDefinitionFile), fetchPlatformDataTask);

			{
				FScopeLock autoLock(&readTaskLock);
				allReadTasks.Add(parseJsonTask);
			}
		}
	}
	else if (akAuxBusIt)
	{
		auto akAuxBus = Cast<UAkAuxBus>(akAuxBusIt->GetAsset());

		if (akAuxBus)
		{
			UAkAssetData* auxBusPlatformData = akAuxBus->FindOrAddAssetData(platform, FString());

			if (auxBusPlatformData)
			{
				if (isBank)
				{
					auto readBankTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, akAuxBus, auxBusPlatformData, FullPath] {
						if (sharedThis->readBankData(auxBusPlatformData, FullPath, *sharedThis->platformFile, &(akAuxBus->BulkDataWriteLock)))
						{
							sharedThis->markAssetDirty(akAuxBus);
						}
					}, GET_STATID(STAT_ReadBankData), nullptr, ENamedThreads::GameThread);

					{
						FScopeLock autoLock(&readTaskLock);
						allReadTasks.Add(readBankTask);
					}
				}
				else
				{
					auto parseJsonTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, akAuxBus, auxBusPlatformData, platform, FullPath] {
						if (sharedThis->readBankDefinitionFile(akAuxBus, auxBusPlatformData, platform, FString(), FullPath, false))
						{
							sharedThis->markAssetDirty(akAuxBus);
						}
						}, GET_STATID(STAT_ReadBankDefinitionFile));

					{
						FScopeLock autoLock(&readTaskLock);
						allReadTasks.Add(parseJsonTask);
					}
				}
			}
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

		auto fetchPlatformDataTask = FFunctionGraphTask::CreateAndDispatchWhenReady([requiredData, fileName, language, platform]
			{
				if (requiredData->akAudioBank)
				{
					requiredData->audioBankPlatformData = requiredData->akAudioBank->FindOrAddAssetData(platform, language);
				}
			}, GET_STATID(STAT_FetchEventPlatformData), nullptr, ENamedThreads::GameThread);

		if (isBank)
		{
			auto readBankDataTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, FullPath, platform, requiredData]
				{
					if (requiredData->akAudioBank && requiredData->audioBankPlatformData)
					{
						if (sharedThis->readBankData(requiredData->audioBankPlatformData, FullPath, *sharedThis->platformFile, &(requiredData->akAudioBank->BulkDataWriteLock)))
						{
							sharedThis->markAssetDirty(requiredData->akAudioBank);
							sharedThis->markAssetDirty(requiredData->audioBankPlatformData);
						}
					}
				}, GET_STATID(STAT_ReadBankData), fetchPlatformDataTask, ENamedThreads::GameThread);

			{
				FScopeLock autoLock(&readTaskLock);
				allReadTasks.Add(readBankDataTask);
			}
		}
		else
		{
			auto parseJsonTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, FullPath, platform, language, requiredData]
				{
					if (requiredData->akAudioBank && requiredData->audioBankPlatformData)
					{
						if (sharedThis->readBankDefinitionFile(requiredData->akAudioBank, requiredData->audioBankPlatformData, platform, language, FullPath, false))
						{
							sharedThis->markAssetDirty(requiredData->akAudioBank);
							sharedThis->markAssetDirty(requiredData->audioBankPlatformData);
						}
					}
				}, GET_STATID(STAT_ReadBankDefinitionFile), fetchPlatformDataTask);

			{
				FScopeLock autoLock(&readTaskLock);
				allReadTasks.Add(parseJsonTask);
			}
		}
	}
	else if (fileName == AkInitBankName || fileName == AkPluginInfoFileName)
	{
		auto initBank = AkAssetDatabase::Get().InitBank;

		UAkAssetData* initBankPlatformData = nullptr;
		initBankPlatformData = initBank->FindOrAddAssetData(platform, FString());

		if (isBank)
		{
			auto readBankTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, initBank, initBankPlatformData, FullPath] {
				if (sharedThis->readBankData(initBankPlatformData, FullPath, *sharedThis->platformFile, &(initBank->BulkDataWriteLock)))
				{
					sharedThis->markAssetDirty(initBank);
				}
				}, GET_STATID(STAT_ReadBankData), nullptr, ENamedThreads::GameThread);

			{
				FScopeLock autoLock(&readTaskLock);
				allReadTasks.Add(readBankTask);
			}
		}
		else if (fileName == AkPluginInfoFileName)
		{
			auto parseJsonTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, initBank, platform, FullPath] {
				if (sharedThis->readPluginInfo(initBank, platform, FullPath))
				{
					sharedThis->markAssetDirty(initBank);
				}


				}, GET_STATID(STAT_ReadBankDefinitionFile), nullptr, ENamedThreads::GameThread);

			{
				FScopeLock autoLock(&readTaskLock);
				allReadTasks.Add(parseJsonTask);
			}
		}
		else
		{
			auto parseJsonTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, initBank, initBankPlatformData, platform, FullPath] {
				if (sharedThis->readBankDefinitionFile(initBank, initBankPlatformData, platform, FString(), FullPath, true))
				{
					sharedThis->markAssetDirty(initBank);
				}
				}, GET_STATID(STAT_ReadBankDefinitionFile));

			{
				FScopeLock autoLock(&readTaskLock);
				allReadTasks.Add(parseJsonTask);
			}
		}
	}
}

