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


/*=============================================================================
	AkEvent.cpp:
=============================================================================*/

#include "AkAudioEvent.h"

#include "AkAudioBank.h"
#include "AkAudioDevice.h"
#include "AkGroupValue.h"
#include "AkMediaAsset.h"
#include "AkUnrealHelper.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "HAL/PlatformProperties.h"
#include "IntegrationBehavior/AkIntegrationBehavior.h"

#if WITH_EDITOR
#include "AssetTools/Public/AssetToolsModule.h"
#include "UnrealEd/Public/ObjectTools.h"
#endif

bool UAkAssetDataSwitchContainerData::IsReadyForAsyncPostLoad() const
{
	for(auto child : Children)
	{
		if (!child->IsReadyForAsyncPostLoad())
		{
			return false;
		}
	}

	for (auto entry : MediaList)
	{
		if (!entry->IsReadyForAsyncPostLoad())
		{
			return false;
		}
	}

	return true;
}

#if WITH_EDITOR
void UAkAssetDataSwitchContainerData::FillTempMediaList()
{
	TempMediaList.Empty();

	for (auto entry : MediaList)
	{
		TempMediaList.Emplace(entry);
	}

	for (auto child : Children)
	{
		child->FillTempMediaList();
	}
}

void UAkAssetDataSwitchContainerData::FillFinalMediaList()
{
	MediaList.Empty();

	for (auto& tempMedia : TempMediaList)
	{
		MediaList.Add(tempMedia.LoadSynchronous());
	}

	for (auto child : Children)
	{
		child->FillFinalMediaList();
	}
}

void UAkAssetDataSwitchContainerData::GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& mediaList) const
{
	internalGetMediaList(this, mediaList);
}

void UAkAssetDataSwitchContainerData::internalGetMediaList(const UAkAssetDataSwitchContainerData* data, TArray<TSoftObjectPtr<UAkMediaAsset>>& mediaList) const
{
	for (auto& media : data->MediaList)
	{
		mediaList.AddUnique(media);
	}

	for (auto* child : data->Children)
	{
		internalGetMediaList(child, mediaList);
	}
}
#endif

void UAkAssetDataSwitchContainer::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	loadSwitchContainer(SwitchContainers);
}

bool UAkAssetDataSwitchContainer::IsReadyForAsyncPostLoad() const
{
	for (auto* switchContainer : SwitchContainers)
	{
		if (!switchContainer->IsReadyForAsyncPostLoad())
		{
			return false;
		}
	}

	return true;
}

void UAkAssetDataSwitchContainer::loadSwitchContainer(const TArray<UAkAssetDataSwitchContainerData*>& switchContainers)
{
	for (auto* container : switchContainers)
	{
		loadSwitchContainer(container);
	}
}

void UAkAssetDataSwitchContainer::loadSwitchContainer(UAkAssetDataSwitchContainerData* switchContainer)
{
	if (switchContainer && IsValid(switchContainer->GroupValue.Get()))
	{
		for (auto& media : switchContainer->MediaList)
		{
			media->Load();
		}

		loadSwitchContainer(switchContainer->Children);
	}
}

void UAkAssetDataSwitchContainer::unloadSwitchContainerMedia(const TArray<UAkAssetDataSwitchContainerData*>& switchContainers)
{
	for (auto* container : switchContainers)
	{
		unloadSwitchContainerMedia(container);
	}
}

void UAkAssetDataSwitchContainer::unloadSwitchContainerMedia(UAkAssetDataSwitchContainerData* switchContainer)
{
	if (switchContainer)
	{
		for (auto media : switchContainer->MediaList)
		{
			media->Unload();
		}

		unloadSwitchContainerMedia(switchContainer->Children);
	}
}

#if WITH_EDITOR
void UAkAssetDataSwitchContainer::FillTempMediaList()
{
	Super::FillTempMediaList();

	for (auto* switchContainer : SwitchContainers)
	{
		switchContainer->FillTempMediaList();
	}
}

void UAkAssetDataSwitchContainer::FillFinalMediaList()
{
	Super::FillFinalMediaList();

	for (auto* switchContainer : SwitchContainers)
	{
		switchContainer->FillFinalMediaList();
	}
}

void UAkAssetDataSwitchContainer::GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& mediaList) const
{
	Super::GetMediaList(mediaList);

	for (auto* switchContainer : SwitchContainers)
	{
		switchContainer->GetMediaList(mediaList);
	}
}
#endif

UAkAssetDataSwitchContainer* UAkAudioEventData::FindOrAddLocalizedData(const FString& language)
{
	FScopeLock autoLock(&LocalizedMediaLock);

	auto* localizedData = LocalizedMedia.Find(language);
	if (localizedData)
	{
		return *localizedData;
	}
	else
	{
		auto* newLocalizedData = NewObject<UAkAssetDataSwitchContainer>(this);
		LocalizedMedia.Add(language, newLocalizedData);
		return newLocalizedData;
	}
}

#if WITH_EDITOR
void UAkAudioEventData::GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& mediaList) const
{
	Super::GetMediaList(mediaList);

	for (auto& entry : LocalizedMedia)
	{
		entry.Value->GetMediaList(mediaList);
	}
}
#endif

float UAkAudioEvent::GetMaxAttenuationRadius() const
{
	if (auto* eventData = Cast<UAkAudioEventData>(getAssetData()))
	{
		return eventData->MaxAttenuationRadius;
	}

	return 0.f;
}

bool UAkAudioEvent::GetIsInfinite() const
{
	if (auto* eventData = Cast<UAkAudioEventData>(getAssetData()))
	{
		return eventData->IsInfinite;
	}

	return false;
}

float UAkAudioEvent::GetMinimumDuration() const
{
	if (auto* eventData = Cast<UAkAudioEventData>(getAssetData()))
	{
		return eventData->MinimumDuration;
	}

	return 0.f;
}

void UAkAudioEvent::SetMinimumDuration(float value)
{
	if (auto* eventData = Cast<UAkAudioEventData>(getAssetData()))
	{
		eventData->MinimumDuration = value;
	}
}

float UAkAudioEvent::GetMaximumDuration() const
{
	if (auto* eventData = Cast<UAkAudioEventData>(getAssetData()))
	{
		return eventData->MaximumDuration;
	}

	return 0.f;
}

void UAkAudioEvent::SetMaximumDuration(float value)
{
	if (auto* eventData = Cast<UAkAudioEventData>(getAssetData()))
	{
		eventData->MaximumDuration = value;
	}
}

bool UAkAudioEvent::SwitchLanguage(const FString& newAudioCulture, const SwitchLanguageCompletedFunction& Function)
{
	bool switchLanguage = false;

	TSoftObjectPtr<UAkAssetPlatformData>* eventDataSoftObjectPtr = LocalizedPlatformAssetDataMap.Find(newAudioCulture);

	if (eventDataSoftObjectPtr)
	{
		auto& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		auto assetData = assetRegistryModule.Get().GetAssetByObjectPath(*eventDataSoftObjectPtr->ToSoftObjectPath().ToString(), true);

		if (assetData.IsValid() && !assetData.PackagePath.IsNone())
		{
			switchLanguage = true;
		}
		else
		{
			if (auto* audioDevice = FAkAudioDevice::Get())
			{
				FString pathWithDefaultLanguage = eventDataSoftObjectPtr->ToSoftObjectPath().ToString().Replace(*newAudioCulture, *audioDevice->GetDefaultLanguage());
				assetData = assetRegistryModule.Get().GetAssetByObjectPath(FName(*pathWithDefaultLanguage), true);
				if (assetData.IsValid() && !assetData.PackagePath.IsNone())
				{
					switchLanguage = true;
				}
			}
		}
	}
	else
	{
		if (auto* eventData = Cast<UAkAudioEventData>(getAssetData()))
		{
			if (eventData->LocalizedMedia.Contains(newAudioCulture))
			{
				switchLanguage = true;
			}
		}
	}

	if (switchLanguage)
	{
		if (auto* audioDevice = FAkAudioDevice::Get())
		{
			if (audioDevice->IsEventIDActive(ShortID))
			{
				UE_LOG(LogAkAudio, Warning, TEXT("Stopping all instances of the \"%s\" event because it is being unloaded during a language change."), *GetName());
				audioDevice->StopEventID(ShortID);
			}
		}

		unloadLocalizedData();

		loadLocalizedData(newAudioCulture, Function);
	}

	return switchLanguage;
}

void UAkAudioEvent::Load()
{
	AkIntegrationBehavior::Get()->AkAudioEvent_Load(this);
}

void UAkAudioEvent::BeginDestroy()
{
	if (auto* audioDevice = FAkAudioDevice::Get())
	{
		if (audioDevice->IsEventIDActive(ShortID))
		{
			UE_LOG(LogAkAudio, Warning, TEXT("Stopping all instances of the \"%s\" event because it is being unloaded."), *GetName());
			audioDevice->StopEventID(ShortID);
		}
	}

	Super::BeginDestroy();
}

bool UAkAudioEvent::IsReadyForFinishDestroy()
{
	bool IsReady = true;
	if (auto* AudioDevice = FAkAudioDevice::Get())
	{
		IsReady = !AudioDevice->IsEventIDActive(ShortID);
		if (!IsReady)
		{
			AudioDevice->Update(0.0);
		}
	}

	return IsReady;
}

void UAkAudioEvent::Unload()
{
	if (IsLocalized())
	{
		unloadLocalizedData();
	}
	else
	{
		Super::Unload();
	}
}

bool UAkAudioEvent::IsLocalized() const
{
	if (RequiredBank)
	{
		return false;
	}

	if (LocalizedPlatformAssetDataMap.Num() > 0)
	{
		return true;
	}

	if (auto* eventData = Cast<UAkAudioEventData>(getAssetData()))
	{
		if (eventData->LocalizedMedia.Num() > 0)
		{
			return true;
		}
	}

	return false;
}

void UAkAudioEvent::PinInGarbageCollector(uint32 PlayingID)
{
	if (TimesPinnedToGC.GetValue() == 0)
	{
		AddToRoot();
	}
	TimesPinnedToGC.Increment();

	if (auto* AudioDevice = FAkAudioDevice::Get())
	{
		AudioDevice->AddToPinnedEventsMap(PlayingID, this);
	}
}

void UAkAudioEvent::UnpinFromGarbageCollector(uint32 PlayingID)
{
	TimesPinnedToGC.Decrement();
	if (TimesPinnedToGC.GetValue() == 0)
	{
		RemoveFromRoot();
	}
}

UAkAssetData* UAkAudioEvent::createAssetData(UObject* parent) const
{
	return NewObject<UAkAudioEventData>(parent);
}

UAkAssetData* UAkAudioEvent::getAssetData() const
{
#if WITH_EDITORONLY_DATA
	if (LocalizedPlatformAssetDataMap.Num() > 0 && CurrentLocalizedPlatformData)
	{
		const FString runningPlatformName(FPlatformProperties::IniPlatformName());

		if (auto platformEventData = CurrentLocalizedPlatformData->AssetDataPerPlatform.Find(runningPlatformName))
		{
			return *platformEventData;
		}
	}

	return Super::getAssetData();
#else
	if (LocalizedPlatformAssetDataMap.Num() > 0 && CurrentLocalizedPlatformData)
	{
		return CurrentLocalizedPlatformData->CurrentAssetData;
	}

	return Super::getAssetData();
#endif
}

void UAkAudioEvent::loadLocalizedData(const FString& audioCulture, const SwitchLanguageCompletedFunction& Function)
{
	if (auto* audioDevice = FAkAudioDevice::Get())
	{
		if (auto* eventData = Cast<UAkAudioEventData>(getAssetData()))
		{
			if (eventData->LocalizedMedia.Num() > 0)
			{
				if (auto* localizedData = eventData->LocalizedMedia.Find(audioCulture))
				{
					(*localizedData)->Load();

					if (Function)
					{
						Function(true);
					}
					return;
				}
			}
		}

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

			TWeakObjectPtr<UAkAudioEvent> weakThis(this);
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

void UAkAudioEvent::onLocalizedDataLoaded()
{
	if (localizedStreamHandle.IsValid())
	{
		CurrentLocalizedPlatformData = Cast<UAkAssetPlatformData>(localizedStreamHandle->GetLoadedAsset());

		Super::Load();
	}
}

void UAkAudioEvent::unloadLocalizedData()
{
	if (auto* eventData = Cast<UAkAudioEventData>(getAssetData()))
	{
		if (eventData->LocalizedMedia.Num() > 0)
		{
			if (auto* audioDevice = FAkAudioDevice::Get())
			{
				if (auto* localizedData = eventData->LocalizedMedia.Find(audioDevice->GetCurrentAudioCulture()))
				{
					(*localizedData)->Unload();
				}
			}
		}
		else
		{
			if (localizedStreamHandle.IsValid())
			{
				Super::Unload();

				CurrentLocalizedPlatformData = nullptr;

				localizedStreamHandle->CancelHandle();
				localizedStreamHandle.Reset();
			}
		}
	}
}

void UAkAudioEvent::superLoad()
{
	Super::Load();
}

#if WITH_EDITOR
UAkAssetData* UAkAudioEvent::FindOrAddAssetData(const FString& platform, const FString& language)
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

	return UAkAssetBase::FindOrAddAssetData(platform, language);
}

void UAkAudioEvent::Reset()
{
	LocalizedPlatformAssetDataMap.Empty();

	Super::Reset();
}

bool UAkAudioEvent::NeedsRebuild(const TSet<FString>& PlatformsToBuild, const TSet<FString>& LanguagesToBuild, const ISoundBankInfoCache* SoundBankInfoCache) const
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

void UAkAudioEvent::PreEditUndo() 
{
	UndoCompareBank = RequiredBank;
	Super::PreEditUndo();
}

void UAkAudioEvent::PostEditUndo()
{
	if (UndoCompareBank != RequiredBank) {
		UndoFlag = true;
	}
	Super::PostEditUndo();
	UndoFlag = false;
}

#if UE_4_25_OR_LATER
void UAkAudioEvent::PreEditChange(FProperty* PropertyAboutToChange)
#else
void UAkAudioEvent::PreEditChange(UProperty* PropertyAboutToChange)
#endif
{
	if (PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UAkAudioEvent, RequiredBank))
	{
		LastRequiredBank = RequiredBank;
	}
	Super::PreEditChange(PropertyAboutToChange);
}

void UAkAudioEvent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
 	if (PropertyChangedEvent.Property)
	{
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAkAudioEvent, RequiredBank))
		{
			UpdateRequiredBanks();
		}
	}
	else if (UndoFlag) {
		LastRequiredBank = UndoCompareBank;
		UpdateRequiredBanks();
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);

}
#endif

void UAkAudioEvent::ClearRequiredBank() 
{
	if (RequiredBank->IsValidLowLevel())
	{
		RequiredBank->RemoveAkAudioEvent(this);
	}
	LastRequiredBank = RequiredBank;
	RequiredBank = nullptr;
}

void UAkAudioEvent::UpdateRequiredBanks() {
	if (LastRequiredBank->IsValidLowLevel())
	{
		LastRequiredBank->RemoveAkAudioEvent(this);
	}

	if (RequiredBank->IsValidLowLevel())
	{
		RequiredBank->AddAkAudioEvent(this);
	}
}

bool UAkAudioEvent::IsReadyForAsyncPostLoad() const
{
	if (auto assetData = getAssetData())
	{
		return assetData->IsReadyForAsyncPostLoad();
	}

	return true;
}

bool UAkAudioEvent::IsLocalizationReady() const
{
	if (localizedStreamHandle)
	{
		return localizedStreamHandle->HasLoadCompleted();
	}

	return true;
}