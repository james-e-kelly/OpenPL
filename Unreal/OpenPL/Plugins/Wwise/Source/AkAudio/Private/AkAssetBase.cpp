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

#include "AkAssetBase.h"

#include "Async/Async.h"
#include "AkMediaAsset.h"
#include "AkUnrealHelper.h"
#include "Platforms/AkPlatformInfo.h"
#include "IntegrationBehavior/AkIntegrationBehavior.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"

#if WITH_EDITOR
#include "TargetPlatform/Public/Interfaces/ITargetPlatform.h"
#include "ISoundBankInfoCache.h"
#endif

void UAkAssetData::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Data.Serialize(Ar, this);
}

AKRESULT UAkAssetData::Load()
{
	return AkIntegrationBehavior::Get()->AkAssetData_Load(this);
}

AKRESULT UAkAssetData::Unload()
{
	if (BankID == AK_INVALID_BANK_ID)
		return Data.GetBulkDataSize() == 0 ? AK_Success : AK_Fail;

	if (auto AudioDevice = FAkAudioDevice::Get())
	{
		// DO NOT USE ASYNC if bank has media in it. Wwise needs access to the bank pointer in order to stop all playing sounds
		// contained within the bank. Depending on the timing, this can take up to an audio frame to process, so the memory
		// needs to remain available to the SoundEngine during that time. Since we are currently destroying the UObject, we 
		// can't guarantee that the memory will remain available that long.
		// In our case, though, the SoundBank does NOT contain media, so it is safe to unload the bank in a "fire and forget"
		// fashion.
		AudioDevice->UnloadBankFromMemoryAsync(BankID, RawData, [](AkUInt32 in_bankID, const void* in_pInMemoryBankPtr, AKRESULT in_eLoadResult, void* in_pCookie) {}, nullptr);
	}

	BankID = AK_INVALID_BANK_ID;
	RawData = nullptr;
	return AK_Success;
}

#if WITH_EDITOR
void UAkAssetData::GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& MediaList) const
{
}
#endif

#if WITH_EDITOR
void UAkAssetDataWithMedia::FillTempMediaList()
{
	TempMediaList.Empty();

	for (auto& media : MediaList)
	{
		TempMediaList.Emplace(media);
	}
}

void UAkAssetDataWithMedia::FillFinalMediaList()
{
	check(IsInGameThread());

	MediaList.Empty();

	for (auto& tempMedia : TempMediaList)
	{
		MediaList.Add(tempMedia.LoadSynchronous());
	}
}

void UAkAssetDataWithMedia::GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& MediaListArray) const
{
	Super::GetMediaList(MediaListArray);

	for (auto& media : MediaList)
	{
		MediaListArray.AddUnique(media);
	}
}
#endif

void UAkAssetPlatformData::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

#if WITH_EDITORONLY_DATA
	if (!Ar.IsFilterEditorOnly())
	{
		Ar << AssetDataPerPlatform;
		return;
	}

	if (Ar.IsSaving())
	{

		FString PlatformName = Ar.CookingTarget()->IniPlatformName();
		if (UAkPlatformInfo::UnrealNameToWwiseName.Contains(PlatformName))
		{
			PlatformName = *UAkPlatformInfo::UnrealNameToWwiseName.Find(PlatformName);
		}
		auto cookedAssetData = AssetDataPerPlatform.Find(PlatformName);
		CurrentAssetData = cookedAssetData ? *cookedAssetData : nullptr;
	}
#endif

	Ar << CurrentAssetData;
}

#if WITH_EDITOR
void UAkAssetPlatformData::Reset()
{
	AssetDataPerPlatform.Reset();
}

bool UAkAssetPlatformData::NeedsRebuild(const TSet<FString>& PlatformsToBuild, const FString& Language, const FGuid& ID, const ISoundBankInfoCache* SoundBankInfoCache) const
{
	TSet<FString> avaiablePlatforms;

	for (auto& entry : AssetDataPerPlatform)
	{
		avaiablePlatforms.Add(entry.Key);

		if (PlatformsToBuild.Contains(entry.Key))
		{
			if (!SoundBankInfoCache->IsSoundBankUpToUpdate(ID, entry.Key, Language, entry.Value->CachedHash))
			{
				return true;
			}
		}
	}

	if (!avaiablePlatforms.Includes(PlatformsToBuild))
	{
		return true;
	}

	return false;
}

void UAkAssetPlatformData::GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& MediaList) const
{
	for (auto& entry : AssetDataPerPlatform)
	{
		entry.Value->GetMediaList(MediaList);
	}
}
#endif

void UAkAssetBase::FinishDestroy()
{
	Unload();
	Super::FinishDestroy();
}

void UAkAssetBase::Load()
{
	Super::Load();
	if (!bDelayLoadAssetMedia)
	{
		if (auto assetData = getAssetData())
		{
			assetData->Load();
		}
	}
}

void UAkAssetBase::Unload()
{
	if (auto assetData = getAssetData())
	{
		assetData->Unload();
	}
}

UAkAssetData* UAkAssetBase::getAssetData() const
{
	if (!PlatformAssetData)
		return nullptr;

#if WITH_EDITORONLY_DATA
	if (auto assetData = PlatformAssetData->AssetDataPerPlatform.Find(FPlatformProperties::IniPlatformName()))
		return *assetData;

	return nullptr;
#else
	return PlatformAssetData->CurrentAssetData;
#endif
}

UAkAssetData* UAkAssetBase::createAssetData(UObject* Parent) const
{
	return NewObject<UAkAssetData>(Parent);
}

#if WITH_EDITOR
UAkAssetData* UAkAssetBase::FindOrAddAssetData(const FString& Platform, const FString& Language)
{
	FScopeLock autoLock(&assetDataLock);

	if (!PlatformAssetData)
	{
		PlatformAssetData = NewObject<UAkAssetPlatformData>(this);
	}

	return internalFindOrAddAssetData(PlatformAssetData, Platform, this);
}

UAkAssetData* UAkAssetBase::internalFindOrAddAssetData(UAkAssetPlatformData* Data, const FString& Platform, UObject* Parent)
{
	auto assetData = Data->AssetDataPerPlatform.Find(Platform);
	if (assetData)
		return *assetData;

	auto newAssetData = createAssetData(Parent);
	Data->AssetDataPerPlatform.Add(Platform, newAssetData);
	return newAssetData;
}

void UAkAssetBase::GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& MediaList) const
{
	if (PlatformAssetData)
	{
		PlatformAssetData->GetMediaList(MediaList);
	}
}

bool UAkAssetBase::NeedsRebuild(const TSet<FString>& PlatformsToBuild, const TSet<FString>& LanguagesToBuild, const ISoundBankInfoCache* SoundBankInfoCache) const
{
	bool needsRebuild = false;

	if (PlatformAssetData)
	{
		needsRebuild = PlatformAssetData->NeedsRebuild(PlatformsToBuild, FString(), ID, SoundBankInfoCache);
	}
	else
	{
		needsRebuild = true;
	}

	TArray<TSoftObjectPtr<UAkMediaAsset>> mediaList;
	GetMediaList(mediaList);

	for (auto& media : mediaList)
	{
		if (media.ToSoftObjectPath().IsValid() && !media.IsValid())
		{
			needsRebuild = true;
		}
	}

	return needsRebuild;
}

void UAkAssetBase::Reset()
{
	PlatformAssetData = nullptr;

	Super::Reset();
}
#endif
