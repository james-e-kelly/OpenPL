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

#include "BlueprintNodes/PostEventAsync.h"

#include "AkGameplayTypes.h"
#include "AkAudioEvent.h"
#include "AkMediaAsset.h"
#include "Engine/Public/TimerManager.h"

UPostEventAsync* UPostEventAsync::PostEventAsync(
	const UObject* WorldContextObject,
	UAkAudioEvent* AkEvent,
	AActor* Actor,
	int32 CallbackMask,
	const FOnAkPostEventCallback& PostEventCallback,
	const TArray<FAkExternalSourceInfo>& ExternalSources,
	bool bStopWhenAttachedToDestroyed
)
{
	UPostEventAsync* newNode = NewObject<UPostEventAsync>();
	newNode->WorldContextObject = WorldContextObject;
	newNode->AkEvent = AkEvent;
	newNode->Actor = Actor;
	newNode->CallbackMask = CallbackMask;
	newNode->PostEventCallback = PostEventCallback;
	newNode->ExternalSources = ExternalSources;
	newNode->bStopWhenAttachedToDestroyed = bStopWhenAttachedToDestroyed;
	return newNode;
}

void UPostEventAsync::Activate()
{
	if (AkEvent == nullptr)
	{
		UE_LOG(LogScript, Warning, TEXT("PostEventAsync: No Event specified!"));
		Completed.Broadcast(AK_INVALID_PLAYING_ID);
		return;
	}

	if (Actor == nullptr)
	{
		UE_LOG(LogScript, Warning, TEXT("PostEventAsync: NULL Actor specified!"));
		Completed.Broadcast(AK_INVALID_PLAYING_ID);
		return;
	}

	AkDeviceAndWorld DeviceAndWorld(Actor);
	if (DeviceAndWorld.IsValid())
	{
		AkCallbackType AkCallbackMask = AkCallbackTypeHelpers::GetCallbackMaskFromBlueprintMask(CallbackMask);
		if (ExternalSources.Num() > 0)
		{
			FAkSDKExternalSourceArray SDKExternalSrcInfo(ExternalSources);
			playingIDFuture = DeviceAndWorld.AkAudioDevice->PostEventAsync(AkEvent, Actor, PostEventCallback, AkCallbackMask, false, SDKExternalSrcInfo.ExternalSourceArray);
		}
		else
		{
			playingIDFuture = DeviceAndWorld.AkAudioDevice->PostEventAsync(AkEvent, Actor, PostEventCallback, AkCallbackMask);
		}

		WorldContextObject->GetWorld()->GetTimerManager().SetTimer(Timer, this, &UPostEventAsync::PollPostEventFuture, 1.f / 60.f, true);
	}
	else
	{
		Completed.Broadcast(AK_INVALID_PLAYING_ID);
	}
}

void UPostEventAsync::PollPostEventFuture()
{
	if (playingIDFuture.IsReady())
	{
		AkPlayingID playingID = playingIDFuture.Get();
		if (playingID != AK_INVALID_PLAYING_ID)
		{
			AkDeviceAndWorld DeviceAndWorld(Actor);
			for (auto ExtSrc : ExternalSources)
			{
				if (ExtSrc.ExternalSourceAsset)
				{
					ExtSrc.ExternalSourceAsset->AddPlayingID(AkEvent->ShortID, playingID);
					if (ExtSrc.ExternalSourceAsset)
					{
						ExtSrc.ExternalSourceAsset->AddPlayingID(AkEvent->ShortID, playingID);
						if (!bStopWhenAttachedToDestroyed)
						{
							ExtSrc.ExternalSourceAsset->PinInGarbageCollector(playingID);
						}
					}
				}
			}

			AkEvent->PinInGarbageCollector(playingID);
		}

		WorldContextObject->GetWorld()->GetTimerManager().ClearTimer(Timer);
		Timer.Invalidate();
		Completed.Broadcast(playingID);
	}
}