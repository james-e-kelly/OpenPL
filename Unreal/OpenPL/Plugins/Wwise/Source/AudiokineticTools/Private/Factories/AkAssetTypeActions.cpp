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


#include "AkAssetTypeActions.h"

#include "AkAudioBank.h"
#include "AkAudioBankGenerationHelpers.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IAssetTools.h"
#include "Interfaces/IMainFrameModule.h"
#include "Misc/ScopeLock.h"
#include "Toolkits/SimpleAssetEditor.h"
#include "UI/SAkAudioBankPicker.h"

#define LOCTEXT_NAMESPACE "AkAssetTypeActions"

namespace FAkAssetTypeActions_Helpers
{
	FCriticalSection CriticalSection;
	TMap<FString, AkPlayingID> PlayingAkEvents;

	void AkEventPreviewCallback(AkCallbackType in_eType, AkCallbackInfo* in_pCallbackInfo)
	{
		auto EventInfo = static_cast<AkEventCallbackInfo*>(in_pCallbackInfo);
		if (!EventInfo)
			return;

		FScopeLock Lock(&CriticalSection);
		for (auto& PlayingEvent : PlayingAkEvents)
		{
			if (PlayingEvent.Value == EventInfo->playingID)
			{
				PlayingAkEvents.Remove(PlayingEvent.Key);
				return;
			}
		}
	}

	template<bool PlayOne>
	void PlayEvents(const TArray<TWeakObjectPtr<UAkAudioEvent>>& InObjects)
	{
		auto AudioDevice = FAkAudioDevice::Get();
		if (!AudioDevice)
			return;

		for (auto& Obj : InObjects)
		{
			auto Event = Obj.Get();
			if (!Event)
				continue;

			AkPlayingID* foundID;
			{
				FScopeLock Lock(&CriticalSection);
				foundID = PlayingAkEvents.Find(Event->GetName());
			}

			if (foundID)
			{
				AudioDevice->StopPlayingID(*foundID);
			}
			else
			{
				auto CurrentPlayingID = AudioDevice->PostEvent(Event, nullptr, AK_EndOfEvent, &AkEventPreviewCallback);
				if (CurrentPlayingID != AK_INVALID_PLAYING_ID)
				{
					FScopeLock Lock(&CriticalSection);
					PlayingAkEvents.FindOrAdd(Event->GetName()) = CurrentPlayingID;
				}
			}

			if (PlayOne)
				break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// FAssetTypeActions_AkAcousticTexture

void FAssetTypeActions_AkAcousticTexture::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}


//////////////////////////////////////////////////////////////////////////
// FAssetTypeActions_AkAudioBank

void FAssetTypeActions_AkAudioBank::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto Banks = GetTypedWeakObjectPtrs<UAkAudioBank>(InObjects);

	if (Banks.Num() > 1)
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AkAudioBank_GenerateSelectedSoundBanks", "Generate Selected SoundBanks..."),
			LOCTEXT("AkAudioBank_GenerateSelectedSoundBanksTooltip", "Generates the selected SoundBanks."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &FAssetTypeActions_AkAudioBank::CreateGenerateSoundDataWindow, Banks),
				FCanExecuteAction()
			)
		);
	}
	else
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AkAudioBank_GenerateSelectedSoundBank", "Generate Selected SoundBank..."),
			LOCTEXT("AkAudioBank_GenerateSelectedSoundBankTooltip", "Generates the selected SoundBank."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &FAssetTypeActions_AkAudioBank::CreateGenerateSoundDataWindow, Banks),
				FCanExecuteAction()
			)
		);
	}

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AkAudioBank_RefreshAllBanks", "Refresh All Banks"),
		LOCTEXT("AkAudioBank_RefreshAllBanksTooltip", "Refresh all the selected banks."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_AkAudioBank::RefreshAllBanks, Banks),
			FCanExecuteAction()
		)
	);
}

void FAssetTypeActions_AkAudioBank::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}

#if UE_4_24_OR_LATER
bool FAssetTypeActions_AkAudioBank::AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType)
#else
void FAssetTypeActions_AkAudioBank::AssetsActivated(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType)
#endif
{
	if (ActivationType == EAssetTypeActivationMethod::DoubleClicked || ActivationType == EAssetTypeActivationMethod::Opened)
	{
		if (InObjects.Num() == 1)
		{
#if UE_4_24_OR_LATER
			return GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(InObjects[0]);
#else
			FAssetEditorManager::Get().OpenEditorForAsset(InObjects[0]);
#endif
		}
		else if (InObjects.Num() > 1)
		{
#if UE_4_24_OR_LATER
			return GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAssets(InObjects);
#else
			FAssetEditorManager::Get().OpenEditorForAssets(InObjects);
#endif
		}
	}

#if UE_4_24_OR_LATER
	return true;
#endif
}

void FAssetTypeActions_AkAudioBank::CreateGenerateSoundDataWindow(TArray<TWeakObjectPtr<UAkAudioBank>> Objects)
{
	AkAudioBankGenerationHelper::CreateGenerateSoundDataWindow(&Objects);
}

void FAssetTypeActions_AkAudioBank::RefreshAllBanks(TArray<TWeakObjectPtr<UAkAudioBank>> Objects)
{
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->ReloadAllSoundData();
	}
}


//////////////////////////////////////////////////////////////////////////
// FAssetTypeActions_AkAudioEvent

void FAssetTypeActions_AkAudioEvent::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto Events = GetTypedWeakObjectPtrs<UAkAudioEvent>(InObjects);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AkAudioEvent_PlayEvent", "Play Event"),
		LOCTEXT("AkAudioEvent_PlayEventTooltip", "Plays the selected event."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_AkAudioEvent::PlayEvent, Events),
			FCanExecuteAction()
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AkAudioEvent_StopEvent", "Stop Event"),
		LOCTEXT("AkAudioEvent_StopEventTooltip", "Stops the selected event."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_AkAudioEvent::StopEvent, Events),
			FCanExecuteAction()
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AkAudioEvent_GroupIntoSoundBank", "Group Into Sound Bank"),
		LOCTEXT("AkAudioEvent_GroupIntoSoundBankTooltip", "Group the selected events into a sound bank."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_AkAudioEvent::GroupIntoSoundBank, Events),
			FCanExecuteAction()
		)
	);
}

void FAssetTypeActions_AkAudioEvent::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}

#if UE_4_24_OR_LATER
bool FAssetTypeActions_AkAudioEvent::AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType)
#else
void FAssetTypeActions_AkAudioEvent::AssetsActivated(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType)
#endif
{
	if (ActivationType == EAssetTypeActivationMethod::DoubleClicked || ActivationType == EAssetTypeActivationMethod::Opened)
	{
		if (InObjects.Num() == 1)
		{
#if UE_4_24_OR_LATER
			return GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(InObjects[0]);
#else
			FAssetEditorManager::Get().OpenEditorForAsset(InObjects[0]);
#endif
		}
		else if (InObjects.Num() > 1)
		{
#if UE_4_24_OR_LATER
			return GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAssets(InObjects);
#else
			FAssetEditorManager::Get().OpenEditorForAssets(InObjects);
#endif
		}
	}
	else if (ActivationType == EAssetTypeActivationMethod::Previewed)
	{
		auto Events = GetTypedWeakObjectPtrs<UAkAudioEvent>(InObjects);
		FAkAssetTypeActions_Helpers::PlayEvents<true>(Events);
	}

#if UE_4_24_OR_LATER
	return true;
#endif
}

void FAssetTypeActions_AkAudioEvent::PlayEvent(TArray<TWeakObjectPtr<UAkAudioEvent>> Objects)
{
	FAkAssetTypeActions_Helpers::PlayEvents<false>(Objects);
}

void FAssetTypeActions_AkAudioEvent::StopEvent(TArray<TWeakObjectPtr<UAkAudioEvent>> Objects)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->StopGameObject(nullptr);
	}
}

void FAssetTypeActions_AkAudioEvent::GroupIntoSoundBank(TArray<TWeakObjectPtr<UAkAudioEvent>> Objects)
{
	TSharedPtr<SAkAudioBankPicker> WindowContent;

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("WindowTitle", "Select Sound Bank"))
		.SizingRule(ESizingRule::Autosized)
		;

	Window->SetContent
	(
		SAssignNew(WindowContent, SAkAudioBankPicker)
		.WidgetWindow(Window)
	);

	TSharedPtr<SWindow> ParentWindow;

	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		ParentWindow = MainFrame.GetParentWindow();
	}

	FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

	if (WindowContent->SelectedAkEventGroup.IsValid())
	{
		for (auto& weakEventPtr : Objects)
		{
			if (!weakEventPtr.IsValid()) continue;

			auto* bank = Cast<UAkAudioBank>(WindowContent->SelectedAkEventGroup.GetAsset());
			weakEventPtr->LastRequiredBank = weakEventPtr->RequiredBank;
			weakEventPtr->RequiredBank = bank;
			weakEventPtr->UpdateRequiredBanks();
			weakEventPtr->MarkPackageDirty();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// FAssetTypeActions_AkAuxBus

void FAssetTypeActions_AkAuxBus::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}

void FAssetTypeActions_AkAuxBus::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto AuxBusses = GetTypedWeakObjectPtrs<UAkAuxBus>(InObjects);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AkAuxBus_GroupIntoSoundBank", "Group Into Sound Bank"),
		LOCTEXT("AkAuxBus_GroupIntoSoundBankTooltip", "Group the selected aux busses into a sound bank."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_AkAuxBus::GroupIntoSoundBank, AuxBusses),
			FCanExecuteAction()
		)
	);
}

void FAssetTypeActions_AkAuxBus::GroupIntoSoundBank(TArray<TWeakObjectPtr<UAkAuxBus>> Objects)
{
	TSharedPtr<SAkAudioBankPicker> WindowContent;

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("WindowTitle", "Select Sound Bank"))
		.SizingRule(ESizingRule::Autosized)
		;

	Window->SetContent
	(
		SAssignNew(WindowContent, SAkAudioBankPicker)
		.WidgetWindow(Window)
	);

	TSharedPtr<SWindow> ParentWindow;

	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		ParentWindow = MainFrame.GetParentWindow();
	}

	FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

	if (WindowContent->SelectedAkEventGroup.IsValid())
	{
		for (auto& weakEventPtr : Objects)
		{
			weakEventPtr->RequiredBank = Cast<UAkAudioBank>(WindowContent->SelectedAkEventGroup.GetAsset());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// FAssetTypeActions_AkMediaAsset

FText FAssetTypeActions_AkMediaAsset::GetAssetDescription(const FAssetData& AssetData) const
{
	if (auto mediaAsset = Cast<UAkMediaAsset>(AssetData.GetAsset()))
	{
		return FText::FromString(mediaAsset->MediaName);
	}

	return FText();
}

#undef LOCTEXT_NAMESPACE
