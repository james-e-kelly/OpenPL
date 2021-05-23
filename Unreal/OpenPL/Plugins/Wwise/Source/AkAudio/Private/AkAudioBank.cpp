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

#include "AkAudioBank.h"

#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkCustomVersion.h"
#include "AkMediaAsset.h"
#include "AkUnrealHelper.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "IntegrationBehavior/AkIntegrationBehavior.h"

#if WITH_EDITOR
#include "AssetTools/Public/AssetToolsModule.h"
#include "UnrealEd/Public/ObjectTools.h"
#endif

void UAkAudioBank::Load()
{
	AkIntegrationBehavior::Get()->AkAudioBank_Load(this);

#if WITH_EDITOR
	if (!ID.IsValid())
	{
		ID = FGuid::NewGuid();
	}
#endif
}

void UAkAudioBank::Unload()
{
	AkIntegrationBehavior::Get()->AkAudioBank_Unload(this);
}

bool UAkAudioBank::SwitchLanguage(const FString& newAudioCulture, const SwitchLanguageCompletedFunction& Function)
{
	auto localizedAssetSoftPointer = LocalizedPlatformAssetDataMap.Find(newAudioCulture);
	if (localizedAssetSoftPointer)
	{
		auto& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		auto assetData = assetRegistryModule.Get().GetAssetByObjectPath(*localizedAssetSoftPointer->ToSoftObjectPath().ToString(), true);

		if (!assetData.IsValid())
		{
			if (auto* audioDevice = FAkAudioDevice::Get())
			{
				FString pathWithDefaultLanguage = localizedAssetSoftPointer->ToSoftObjectPath().ToString().Replace(*newAudioCulture, *audioDevice->GetDefaultLanguage());
				assetData = assetRegistryModule.Get().GetAssetByObjectPath(FName(*pathWithDefaultLanguage), true);
			}
		}

		if (assetData.IsValid() && !assetData.PackagePath.IsNone())
		{
			unloadLocalizedData();

			loadLocalizedData(newAudioCulture, Function);
			return true;
		}
	}

	return false;
}

void UAkAudioBank::AddAkAudioEvent(UAkAudioEvent* event)
{
	bool AlreadyInSet = false;
	LinkedAkEvents.Add(TSoftObjectPtr<UAkAudioEvent>(event), &AlreadyInSet);
	if (!AlreadyInSet)
	{
		MarkPackageDirty();
	}
}

void UAkAudioBank::RemoveAkAudioEvent(UAkAudioEvent* event)
{
	auto NumRemoved = LinkedAkEvents.Remove(TSoftObjectPtr<UAkAudioEvent>(event));
	if (NumRemoved != 0)
	{
		MarkPackageDirty();
	}
}

void UAkAudioBank::populateAkAudioEvents()
{
	bool changed = false;
	for (auto e : FindObjectsReferencingThis<UAkAudioEvent>())
	{
		if (e->RequiredBank == this)
		{
			bool AlreadyInSet = false;
			LinkedAkEvents.Add(e, &AlreadyInSet);
			changed |= !AlreadyInSet;
		}
	}

	if (changed)
	{
		MarkPackageDirty();
	}
}

bool UAkAudioBank::LegacyLoad()
{
	if (!IsRunningCommandlet())
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice)
		{
			AkBankID BankID;
			AKRESULT eResult = AudioDevice->LoadBank(this, BankID);
			if (eResult == AK_Success)
			{
				return true;
			}
		}
	}

	return false;
}

bool UAkAudioBank::LegacyLoad(FWaitEndBankAction* LoadBankLatentAction)
{
	if (!IsRunningCommandlet())
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice)
		{
			AKRESULT eResult = AudioDevice->LoadBank(this, LoadBankLatentAction);
			if (eResult == AK_Success)
			{
				return true;
			}
		}
	}

	return false;
}

bool UAkAudioBank::LegacyLoadAsync(void* in_pfnBankCallback, void* in_pCookie)
{
	if (!IsRunningCommandlet())
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice)
		{
			AkBankID BankID;
			AKRESULT eResult = AudioDevice->LoadBank(this, (AkBankCallbackFunc)in_pfnBankCallback, in_pCookie, BankID);
			if (eResult == AK_Success)
			{
				return true;
			}
		}
	}

	return false;
}

bool UAkAudioBank::LegacyLoadAsync(const FOnAkBankCallback& BankLoadedCallback)
{
	if (!IsRunningCommandlet())
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice)
		{
			AkBankID BankID;
			AKRESULT eResult = AudioDevice->LoadBankAsync(this, BankLoadedCallback, BankID);
			if (eResult == AK_Success)
			{
				return true;
			}
		}
	}

	return false;
}

void UAkAudioBank::LegacyUnload()
{
	if (!IsRunningCommandlet())
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice)
		{
			AudioDevice->UnloadBank(this);
		}
	}
}

void UAkAudioBank::LegacyUnload(FWaitEndBankAction* UnloadBankLatentAction)
{
	if (!IsRunningCommandlet())
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice)
		{
			AudioDevice->UnloadBank(this, UnloadBankLatentAction);
		}
	}
}

void UAkAudioBank::LegacyUnloadAsync(void* in_pfnBankCallback, void* in_pCookie)
{
	if (!IsRunningCommandlet())
	{
		AKRESULT eResult = AK_Fail;
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice)
		{
			eResult = AudioDevice->UnloadBank(this, (AkBankCallbackFunc)in_pfnBankCallback, in_pCookie);
			if (eResult != AK_Success)
			{
				UE_LOG(LogAkAudio, Warning, TEXT("Failed to unload SoundBank %s"), *GetName());
			}
		}
	}
}

void UAkAudioBank::LegacyUnloadAsync(const FOnAkBankCallback& BankUnloadedCallback)
{
	if (!IsRunningCommandlet())
	{
		AKRESULT eResult = AK_Fail;
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice)
		{
			eResult = AudioDevice->UnloadBankAsync(this, BankUnloadedCallback);
			if (eResult != AK_Success)
			{
				UE_LOG(LogAkAudio, Warning, TEXT("Failed to unload SoundBank %s"), *GetName());
			}
		}
	}
}

UAkAssetData* UAkAudioBank::createAssetData(UObject* parent) const
{
	return NewObject<UAkAssetDataWithMedia>(parent);
}

UAkAssetData* UAkAudioBank::getAssetData() const
{
#if WITH_EDITORONLY_DATA
	if (IsLocalized() && CurrentLocalizedPlatformAssetData)
	{
		const FString runningPlatformName(FPlatformProperties::IniPlatformName());

		if (auto assetData = CurrentLocalizedPlatformAssetData->AssetDataPerPlatform.Find(runningPlatformName))
		{
			return *assetData;
		}
	}

	return Super::getAssetData();
#else
	if (IsLocalized() && CurrentLocalizedPlatformAssetData)
	{
		return CurrentLocalizedPlatformAssetData->CurrentAssetData;
	}

	return Super::getAssetData();
#endif
}

void UAkAudioBank::loadLocalizedData(const FString& audioCulture, const SwitchLanguageCompletedFunction& Function)
{
	if (auto* audioDevice = FAkAudioDevice::Get())
	{
		TSoftObjectPtr<UAkAssetPlatformData>* eventDataSoftObjectPtr = LocalizedPlatformAssetDataMap.Find(audioCulture);
		if (eventDataSoftObjectPtr)
		{
			auto& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

			FSoftObjectPath localizedDataPath = eventDataSoftObjectPtr->ToSoftObjectPath();

			if (!assetRegistryModule.Get().GetAssetByObjectPath(*localizedDataPath.ToString(), true).IsValid())
			{
				FString pathWithDefaultLanguage = eventDataSoftObjectPtr->ToSoftObjectPath().ToString().Replace(*audioCulture, *audioDevice->GetDefaultLanguage());
				auto assetData = assetRegistryModule.Get().GetAssetByObjectPath(FName(*pathWithDefaultLanguage), true);
				if (assetRegistryModule.Get().GetAssetByObjectPath(FName(*pathWithDefaultLanguage), true).IsValid())
				{
					localizedDataPath = FSoftObjectPath(pathWithDefaultLanguage);
				}
			}

			TWeakObjectPtr< UAkAudioBank > weakThis(this);
			localizedStreamHandle = audioDevice->GetStreamableManager().RequestAsyncLoad(localizedDataPath, [weakThis, Function] {
				if (weakThis.IsValid())
				{
					weakThis->onLocalizedDataLoaded();

						if (Function)
						{
							Function(weakThis->localizedStreamHandle.IsValid());
						}
				}
			});
		}
	}
}

void UAkAudioBank::onLocalizedDataLoaded()
{
	if (localizedStreamHandle.IsValid())
	{
		CurrentLocalizedPlatformAssetData = Cast<UAkAssetPlatformData>(localizedStreamHandle->GetLoadedAsset());

		Super::Load();
	}
}

void UAkAudioBank::unloadLocalizedData()
{
	if (localizedStreamHandle.IsValid())
	{
		Super::Unload();

		CurrentLocalizedPlatformAssetData = nullptr;

		localizedStreamHandle->CancelHandle();
		localizedStreamHandle.Reset();
	}
}

void UAkAudioBank::superLoad()
{
	Super::Load();
}

void UAkAudioBank::superUnload()
{
	Super::Unload();
}

void UAkAudioBank::PostLoad()
{
	Super::PostLoad();

	const int32 AkVersion = GetLinkerCustomVersion(FAkCustomVersion::GUID);
	if (AkVersion < FAkCustomVersion::SounbanksLinkedToEvents)
	{
		auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().OnFilesLoaded().AddLambda([this] {this->populateAkAudioEvents(); });
	}

	LoadedBankName = GetName();
}

#if WITH_EDITOR
UAkAssetData* UAkAudioBank::FindOrAddAssetData(const FString& platform, const FString& language)
{
	check(IsInGameThread());

	UAkAssetPlatformData* eventData = nullptr;
	UObject* parent = this;

	if (language.Len() > 0)
	{
		auto* languageIt = LocalizedPlatformAssetDataMap.Find(language);
		if (languageIt)
		{
			eventData = languageIt->LoadSynchronous();

			if (eventData)
			{
				parent = eventData;
			}
			else
			{
				LocalizedPlatformAssetDataMap.Remove(language);
			}
		}

		if (!eventData)
		{
			auto& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			auto& assetToolModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

			FSoftObjectPath objectPath(this);

			auto basePackagePath = AkUnrealHelper::GetBaseAssetPackagePath();

			auto packagePath = objectPath.GetLongPackageName();
			packagePath.RemoveFromStart(basePackagePath);
			packagePath.RemoveFromEnd(objectPath.GetAssetName());
			packagePath = ObjectTools::SanitizeObjectPath(FPaths::Combine(AkUnrealHelper::GetLocalizedAssetPackagePath(), language, packagePath));

			auto assetName = GetName();

			auto foundAssetData = assetRegistryModule.Get().GetAssetByObjectPath(*FPaths::Combine(packagePath, FString::Printf(TEXT("%s.%s"), *assetName, *assetName)));
			if (foundAssetData.IsValid())
			{
				eventData = Cast<UAkAssetPlatformData>(foundAssetData.GetAsset());
			}
			else
			{
				eventData = Cast<UAkAssetPlatformData>(assetToolModule.Get().CreateAsset(assetName, packagePath, UAkAssetPlatformData::StaticClass(), nullptr));
			}

			if (eventData)
			{
				parent = eventData;

				LocalizedPlatformAssetDataMap.Add(language, eventData);
			}
		}

		if (eventData)
		{
			return internalFindOrAddAssetData(eventData, platform, parent);
		}
	}

	return Super::FindOrAddAssetData(platform, language);
}

void UAkAudioBank::Reset()
{
	LocalizedPlatformAssetDataMap.Empty();

	Super::Reset();
}

bool UAkAudioBank::NeedsRebuild(const TSet<FString>& PlatformsToBuild, const TSet<FString>& LanguagesToBuild, const ISoundBankInfoCache* SoundBankInfoCache) const
{
	bool result = Super::NeedsRebuild(PlatformsToBuild, LanguagesToBuild, SoundBankInfoCache);

	if (!PlatformAssetData)
	{
		result = false;

		TArray<TSoftObjectPtr<UAkMediaAsset>> mediaList;
		GetMediaList(mediaList);

		for (auto& media : mediaList)
		{
			if (media.ToSoftObjectPath().IsValid() && !media.IsValid())
			{
				result = true;
			}
		}
	}

	if (!result)
	{
		TSet<FString> availableLanguages;
		for (auto& entry : LocalizedPlatformAssetDataMap)
		{
			availableLanguages.Add(entry.Key);

			if (LanguagesToBuild.Contains(entry.Key))
			{
				if (auto assetData = entry.Value.Get())
				{
					result |= assetData->NeedsRebuild(PlatformsToBuild, entry.Key, ID, SoundBankInfoCache);
				}
				else
				{
					result = true;
				}
			}
		}

		if (!PlatformAssetData && !availableLanguages.Includes(LanguagesToBuild))
		{
			result = true;
		}
	}

	return result;
}
#endif
