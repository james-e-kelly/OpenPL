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

#pragma once

#include "Containers/IndirectArray.h"
#include "Core/Public/Templates/Function.h"
#include "Serialization/BulkData.h"
#include "UObject/Object.h"

#include "AkMediaAsset.generated.h"

struct AKAUDIO_API FAkMediaDataChunk
{
	FAkMediaDataChunk();

#if WITH_EDITOR
	FAkMediaDataChunk(IFileHandle* FileHandle, int64 BytesToRead, uint32 BulkDataFlags, FCriticalSection* DataWriteLock, bool IsPrefetch = false);
#endif

	FByteBulkData Data;
	bool IsPrefetch = false;

	void Serialize(FArchive& Ar, UObject* Owner);
};

using MediaAssetDataLoadAsyncCallback = TFunction<void(uint8* MediaData, int64 MediaDataSize)>;

UCLASS()
class AKAUDIO_API UAkMediaAssetData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "AkMediaAsset")
	bool IsStreamed = false;

	UPROPERTY(VisibleAnywhere, Category = "AkMediaAsset")
	bool UseDeviceMemory = false;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	int64 LastWriteTime = 0;

	UPROPERTY(VisibleAnywhere, Category = "AkMediaAsset")
	uint64 MediaContentHash = 0;
#endif

	TIndirectArray<FAkMediaDataChunk> DataChunks;

#if WITH_EDITOR
	FCriticalSection DataLock;
#endif

	friend class UAkMediaAsset;
	friend class UAkExternalMediaAsset;

public:
	void Serialize(FArchive& Ar) override;

	bool IsReadyForAsyncPostLoad() const override;

	void Load(bool LoadAsync, const MediaAssetDataLoadAsyncCallback& LoadedCallback = MediaAssetDataLoadAsyncCallback());
	void Unload();

	bool NeedsAutoLoading() const;
	bool IsLoaded() const { return State == LoadState::Loaded; }
	
private:
	uint8* allocateMediaMemory();
	void freeMediaMemory(uint8* mediaMemory);

private:
	enum class LoadState
	{
		Unloaded,
		Loading,
		Loaded,
		Error,
	} State;

	uint8* LoadedMediaData = nullptr;
	UAkMediaAsset* Parent = nullptr;
};

UCLASS()
class AKAUDIO_API UAkMediaAsset : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "AkMediaAsset")
	uint32 Id;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "AkMediaAsset")
	TMap<FString, UAkMediaAssetData*> MediaAssetDataPerPlatform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AssetRegistrySearchable, Category = "AkMediaAsset")
	FString MediaName;
#endif

	UPROPERTY(EditAnywhere, Category = "AkMediaAsset")
	bool AutoLoad = true;

	UPROPERTY(EditAnywhere, Category = "AkMediaAsset")
	TArray<UObject*> UserData;

private:
	UPROPERTY()
	UAkMediaAssetData* CurrentMediaAssetData;

public:
	void Serialize(FArchive& Ar) override;
	void PostLoad() override;
	void FinishDestroy() override;

	void Load(bool FromSerialize = false);
	void Unload();

	bool IsReadyForAsyncPostLoad() const override;

#if WITH_EDITOR
	UAkMediaAssetData* FindOrAddMediaAssetData(const FString& platform);

	virtual void Reset();

	FCriticalSection BulkDataWriteLock;
#endif

	FAkMediaDataChunk const* GetStreamedChunk() const;

	template<typename T>
	T* GetUserData()
	{
		for (auto entry : UserData)
		{
			if (entry && entry->GetClass()->IsChildOf(T::StaticClass()))
			{
				return entry;
			}
		}

		return nullptr;
	}

protected:
	void loadAndSetMedia(bool LoadAsync);
	void unloadMedia(bool ForceUnload = false);

	UAkMediaAssetData* getMediaAssetData() const;

private:
	FThreadSafeCounter LoadRefCount;
	bool ForceAutoLoad = false;

	bool SerializeHasBeenCalled = false;
	bool LoadFromSerialize = false;
};

UCLASS()
class AKAUDIO_API UAkLocalizedMediaAsset : public UAkMediaAsset
{
	GENERATED_BODY()
};

UCLASS()
class AKAUDIO_API UAkExternalMediaAsset : public UAkMediaAsset
{
	GENERATED_BODY()

public:
	UAkExternalMediaAsset();

	TTuple<void*, int64> GetExternalSourceData();
	FThreadSafeCounter NumStreamingHandles;

	void AddPlayingID(uint32 EventID, uint32 PlayingID);
	bool HasActivePlayingIDs();
	TMap<uint32, TArray<uint32>> ActiveEventToPlayingIDMap;

	bool IsReadyForFinishDestroy() override;
	void BeginDestroy() override;

	void PinInGarbageCollector(uint32 PlayingID);
	void UnpinFromGarbageCollector(uint32 PlayingID);

private:
	FThreadSafeCounter TimesPinnedToGC;
};

