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

#include "AkMediaAsset.h"

#include "AkAudioDevice.h"
#include "AkUnrealIOHook.h"
#include "HAL/PlatformProperties.h"
#include "Core/Public/Modules/ModuleManager.h"
#include "Misc/ScopeExit.h"

#if WITH_EDITOR
#include "Platforms/AkPlatformInfo.h"
#include "Interfaces/ITargetPlatform.h"
#endif


FAkMediaDataChunk::FAkMediaDataChunk() { }

#if WITH_EDITOR
FAkMediaDataChunk::FAkMediaDataChunk(IFileHandle* FileHandle, int64 BytesToRead, uint32 BulkDataFlags, FCriticalSection* DataWriteLock, bool IsPrefetch)
	: IsPrefetch(IsPrefetch)
{
	FScopeLock DataLock(DataWriteLock);
	Data.SetBulkDataFlags(BulkDataFlags);
	Data.Lock(EBulkDataLockFlags::LOCK_READ_WRITE);
	FileHandle->Read(reinterpret_cast<uint8*>(Data.Realloc(BytesToRead)), BytesToRead);
	Data.Unlock();
}
#endif

void FAkMediaDataChunk::Serialize(FArchive& Ar, UObject* Owner)
{
	Ar << IsPrefetch;
	Data.Serialize(Ar, Owner);
}

void UAkMediaAssetData::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	int32 numChunks = DataChunks.Num();
	Ar << numChunks;

	if (Ar.IsLoading())
	{
		DataChunks.Empty();
		for (int32 i = 0; i < numChunks; ++i)
		{
			DataChunks.Add(new FAkMediaDataChunk());
		}
	}

	for (int32 i = 0; i < numChunks; ++i)
	{
		DataChunks[i].Serialize(Ar, this);
	}

	if (Ar.IsLoading())
	{
		if (NeedsAutoLoading())
		{
			Load(true);
		}
	}
}

bool UAkMediaAssetData::IsReadyForAsyncPostLoad() const
{
	if (NeedsAutoLoading())
	{
		return State == LoadState::Loaded || State == LoadState::Error;
	}

	return true;
}

void UAkMediaAssetData::Load(bool LoadAsync, const MediaAssetDataLoadAsyncCallback& LoadedCallback)
{
	if (DataChunks.Num() > 0 && !LoadedMediaData)
	{
#if UE_4_25_OR_LATER
		FBulkDataIORequestCallBack LoadAsyncCompleted = [this, LoadedCallback](bool bWasCancelled, IBulkDataIORequest* ReadRequest)
#else
		FAsyncFileCallBack LoadAsyncCompleted = [this, LoadedCallback](bool bWasCancelled, IAsyncReadRequest* ReadRequest)
#endif
		{
			if (bWasCancelled)
			{
				State = LoadState::Error;
				freeMediaMemory(ReadRequest->GetReadResults());
				FString mediaName;
#if WITH_EDITORONLY_DATA
				if (Parent)
				{
					mediaName = Parent->MediaName;
				}
#endif
				uint32 mediaID = Parent ? Parent->Id : 0;
				UE_LOG(LogAkAudio, Error, TEXT("Bulk data streaming request for '%s' (ID: %u) was cancelled. Media will be unavailable."), *mediaName, mediaID);
				return;
			}

			LoadedMediaData = ReadRequest->GetReadResults();
			State = LoadState::Loaded;

			if (LoadedCallback)
			{
				LoadedCallback(LoadedMediaData, DataChunks[0].Data.GetBulkDataSize());
			}
		};

#if !WITH_EDITOR
		if (DataChunks[0].Data.CanLoadFromDisk() && LoadAsync)
		{
			uint8* tempReadMediaMemory = allocateMediaMemory();
			State = LoadState::Loading;
			DataChunks[0].Data.CreateStreamingRequest(EAsyncIOPriorityAndFlags::AIOP_High, &LoadAsyncCompleted, tempReadMediaMemory);
		}
		else
#endif
		{
			LoadedMediaData = allocateMediaMemory();
			auto bulkMediaDataSize = DataChunks[0].Data.GetBulkDataSize();
			DataChunks[0].Data.GetCopy((void**)&LoadedMediaData, false);
			State = LoadState::Loaded;

			if (LoadedCallback)
			{
				LoadedCallback(LoadedMediaData, bulkMediaDataSize);
			}
		}
	}
}

void UAkMediaAssetData::Unload()
{
	if (LoadedMediaData)
	{
		freeMediaMemory(LoadedMediaData);
		LoadedMediaData = nullptr;
		State = LoadState::Unloaded;
	}
}

bool UAkMediaAssetData::NeedsAutoLoading() const
{
	if (DataChunks.Num() > 0)
	{
		if (!IsStreamed || (IsStreamed && DataChunks.Num() > 1 && DataChunks[0].IsPrefetch))
		{
			if (Parent && Parent->AutoLoad)
			{
				return true;
			}
		}
	}

	return false;
}

uint8* UAkMediaAssetData::allocateMediaMemory()
{
	if (DataChunks.Num() > 0)
	{
		auto dataBulkSize = DataChunks[0].Data.GetBulkDataSize();
#if AK_SUPPORT_DEVICE_MEMORY
		if (UseDeviceMemory)
		{
			return (AkUInt8*)AKPLATFORM::AllocDevice(dataBulkSize, 0);
		}
		else
#endif
		{
			return static_cast<uint8*>(FMemory::Malloc(dataBulkSize));
		}
	}

	return nullptr;
}

void  UAkMediaAssetData::freeMediaMemory(uint8* mediaMemory)
{
	if (!mediaMemory)
	{
		return;
	}

#if AK_SUPPORT_DEVICE_MEMORY
	if (UseDeviceMemory)
	{
		AKPLATFORM::FreeDevice(mediaMemory, DataChunks[0].Data.GetBulkDataSize(), 0, true);
	}
	else
#endif
	{
		FMemory::Free(mediaMemory);
	}
}

void UAkMediaAsset::Serialize(FArchive& Ar)
{
	SerializeHasBeenCalled = true;
	Super::Serialize(Ar);
	if (Ar.IsLoading())
	{
		ForceAutoLoad = AutoLoad | LoadFromSerialize;
	}

#if WITH_EDITORONLY_DATA
	if (Ar.IsFilterEditorOnly())
	{
		if (Ar.IsSaving())
		{
			FString PlatformName = Ar.CookingTarget()->IniPlatformName();
			if (UAkPlatformInfo::UnrealNameToWwiseName.Contains(PlatformName))
			{
				PlatformName = *UAkPlatformInfo::UnrealNameToWwiseName.Find(PlatformName);
			}
			auto currentMediaData = MediaAssetDataPerPlatform.Find(*PlatformName);
			CurrentMediaAssetData = currentMediaData ? *currentMediaData : nullptr;
		}

		Ar << CurrentMediaAssetData;
	}
	else
	{
		Ar << MediaAssetDataPerPlatform;
	}
#else
	Ar << CurrentMediaAssetData;
#endif

	if (Ar.IsLoading())
	{
		if (auto assetData = getMediaAssetData())
		{
			assetData->Parent = this;
		}
	}
}

void UAkMediaAsset::PostLoad()
{
	static FName AkAudioName = TEXT("AkAudio");
	Super::PostLoad();
	if (FModuleManager::Get().IsModuleLoaded(AkAudioName))
	{
		if (AutoLoad || ForceAutoLoad)
		{
			Load();
		}
	}
	else
	{
		FAkAudioDevice::DelayMediaLoad(this);
	}

}

void UAkMediaAsset::FinishDestroy()
{
	unloadMedia(true);
	Super::FinishDestroy();
}

#if WITH_EDITOR
#include "Async/Async.h"
void UAkMediaAsset::Reset()
{
	if (LoadRefCount.GetValue() > 0)
	{
		LoadRefCount.Set(1);
		Unload();
	}

	MediaAssetDataPerPlatform.Empty();
	MediaName.Empty();
	CurrentMediaAssetData = nullptr;
	AsyncTask(ENamedThreads::GameThread, [this] {
		MarkPackageDirty();
	});
}

UAkMediaAssetData* UAkMediaAsset::FindOrAddMediaAssetData(const FString& platform)
{
	auto platformData = MediaAssetDataPerPlatform.Find(platform);
	if (platformData)
	{
		return *platformData;
	}

	auto newPlatformData = NewObject<UAkMediaAssetData>(this);
	MediaAssetDataPerPlatform.Add(platform, newPlatformData);
	return newPlatformData;
}
#endif

void UAkMediaAsset::Load(bool FromSerialize /*= false*/)
{
	if (UNLIKELY(FromSerialize && !SerializeHasBeenCalled))
	{
		LoadFromSerialize = true;
	}
	else
	{
		loadAndSetMedia(true);
	}
}

void UAkMediaAsset::Unload()
{
	unloadMedia();
}

bool UAkMediaAsset::IsReadyForAsyncPostLoad() const
{
	if (auto assetData = getMediaAssetData())
	{
		return assetData->IsReadyForAsyncPostLoad();
	}

	return true;
}

FAkMediaDataChunk const* UAkMediaAsset::GetStreamedChunk() const
{
	auto mediaData = getMediaAssetData();
	if (!mediaData || mediaData->DataChunks.Num() <= 0)
	{
		return nullptr;
	}

	if (!mediaData->DataChunks[0].IsPrefetch)
	{
		return &mediaData->DataChunks[0];
	}

	if (mediaData->DataChunks.Num() >= 2)
	{
		return &mediaData->DataChunks[1];
	}

	return nullptr;
}

void UAkMediaAsset::loadAndSetMedia(bool LoadAsync)
{
	auto assetData = getMediaAssetData();
	if (!assetData || assetData->DataChunks.Num() <= 0)
	{
#if WITH_EDITOR
		ForceAutoLoad = true;
#endif
		return;
	}

	auto& DataChunk = assetData->DataChunks[0];
	if (assetData->IsStreamed && !DataChunk.IsPrefetch)
	{
		FAkUnrealIOHook::AddStreamingMedia(this);
		return;
	}

#if !WITH_EDITOR
	if (DataChunk.Data.GetBulkDataSize() <= 0)
	{
		return;
	}
#endif
	
	if (LoadRefCount.GetValue() == 0)
	{
		MediaAssetDataLoadAsyncCallback DoSetMedia = [this, assetData](uint8* MediaData, int64 MediaDataSize)
		{
			AkSourceSettings sourceSettings
			{
				Id, reinterpret_cast<AkUInt8*>(MediaData), static_cast<AkUInt32>(MediaDataSize)
			};

			if (auto akAudioDevice = FAkAudioDevice::Get())
			{
				if (akAudioDevice->SetMedia(&sourceSettings, 1) != AK_Success)
				{
					UE_LOG(LogAkAudio, Log, TEXT("SetMedia failed for ID: %u"), Id);
				}
			}

			if (assetData->IsStreamed)
			{
				FAkUnrealIOHook::AddStreamingMedia(this);
			}

			LoadRefCount.Increment();
		};

		if (assetData->IsLoaded())
		{
			DoSetMedia(assetData->LoadedMediaData, assetData->DataChunks[0].Data.GetBulkDataSize());
		}
		else
		{
			assetData->Load(LoadAsync, DoSetMedia);
		}
	}
	else
	{
		LoadRefCount.Increment();
	}
}

void UAkMediaAsset::unloadMedia(bool ForceUnload /* = false */)
{
	auto assetData = getMediaAssetData();
	if (!assetData)
	{
		return;
	}

	if (LoadRefCount.GetValue() > 0)
	{
		LoadRefCount.Decrement();

		if (LoadRefCount.GetValue() <= 0 || ForceUnload)
		{
			if (assetData && assetData->IsStreamed)
			{
				FAkUnrealIOHook::RemoveStreamingMedia(this);
			}

			if (assetData && assetData->DataChunks.Num() > 0)
			{
				if (auto audioDevice = FAkAudioDevice::Get())
				{
					AkSourceSettings sourceSettings
					{
						Id, reinterpret_cast<AkUInt8*>(assetData->LoadedMediaData), static_cast<AkUInt32>(assetData->DataChunks[0].Data.GetBulkDataSize())
					};
					audioDevice->UnsetMedia(&sourceSettings, 1);
				}

				assetData->Unload();
			}

			LoadRefCount.Set(0);
		}
	}
}

UAkMediaAssetData* UAkMediaAsset::getMediaAssetData() const
{
#if !WITH_EDITORONLY_DATA
	return CurrentMediaAssetData;
#else
	const FString runningPlatformName(FPlatformProperties::IniPlatformName());
	if (auto platformMediaData = MediaAssetDataPerPlatform.Find(runningPlatformName))
	{
		return *platformMediaData;
	}

	return nullptr;
#endif
}

UAkExternalMediaAsset::UAkExternalMediaAsset()
{
	AutoLoad = false;
}

TTuple<void*, int64> UAkExternalMediaAsset::GetExternalSourceData()
{
	auto* mediaData = getMediaAssetData();

	if (mediaData && mediaData->DataChunks.Num() > 0)
	{
		loadAndSetMedia(false);
		auto result = MakeTuple(static_cast<void*>(mediaData->LoadedMediaData), mediaData->DataChunks[0].Data.GetBulkDataSize());
		return result;
	}

	return {};
}

void UAkExternalMediaAsset::AddPlayingID(uint32 EventID, uint32 PlayingID)
{
	auto& PlayingIDArray = ActiveEventToPlayingIDMap.FindOrAdd(EventID);
	PlayingIDArray.Add(PlayingID);
}

bool UAkExternalMediaAsset::HasActivePlayingIDs()
{
	AK::SoundEngine::RenderAudio();
	if (auto* AudioDevice = FAkAudioDevice::Get())
	{
		for (auto pair : ActiveEventToPlayingIDMap)
		{
			uint32 EventID = pair.Key;
			for (auto PlayingID : pair.Value)
			{
				if (AudioDevice->IsPlayingIDActive(EventID, PlayingID))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void UAkExternalMediaAsset::BeginDestroy()
{
	if (auto* AudioDevice = FAkAudioDevice::Get())
	{
		for (auto pair : ActiveEventToPlayingIDMap)
		{
			uint32 EventID = pair.Key;
			for (auto PlayingID : pair.Value)
			{
				if (AudioDevice->IsPlayingIDActive(EventID, PlayingID))
				{
					UE_LOG(LogAkAudio, Warning, TEXT("Stopping PlayingID %u because media file %s is being unloaded."), PlayingID, *GetName());
					AudioDevice->StopPlayingID(PlayingID);
				}
			}
		}

		AudioDevice->Update(0.0);
	}
	
	Super::BeginDestroy();
}


bool UAkExternalMediaAsset::IsReadyForFinishDestroy()
{
	bool IsReady = true;
	if (auto* AudioDevice = FAkAudioDevice::Get())
	{
		IsReady = !HasActivePlayingIDs();
		if (!IsReady)
		{
			// Give time for the sounds what were stopped in BeginDestroy to finish processing
			FPlatformProcess::Sleep(0.05);
			AudioDevice->Update(0.0);
		}
	}

	return IsReady;
}

void UAkExternalMediaAsset::PinInGarbageCollector(uint32 PlayingID)
{
	if (TimesPinnedToGC.GetValue() == 0)
	{
		AddToRoot();
	}
	TimesPinnedToGC.Increment();

	if (auto* AudioDevice = FAkAudioDevice::Get())
	{
		AudioDevice->AddToPinnedMediasMap(PlayingID, this);
	}
}

void UAkExternalMediaAsset::UnpinFromGarbageCollector(uint32 PlayingID)
{
	TimesPinnedToGC.Decrement();
	if (TimesPinnedToGC.GetValue() == 0)
	{
		RemoveFromRoot();
	}
}
