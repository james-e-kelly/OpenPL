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
	InterpTrackAkEventHelper.cpp: 
=============================================================================*/
#include "Sequencer/InterpTrackAkAudioEventHelper.h"

#include "Editor.h"
#include "EditorModeInterpolation.h"
#include "EditorModeManager.h"
#include "EditorModes.h"
#include "Engine/Selection.h"
#include "Framework/Application/MenuStack.h"
#include "Framework/Application/SlateApplication.h"
#include "IMatinee.h"
#include "InterpTrackAkAudioEvent.h"
#include "MatineeModule.h"
#include "PropertyCustomizationHelpers.h"
#include "SMatineeAkEventKeyFrameAdder.h"

#define LOCTEXT_NAMESPACE "Audiokinetic"

static TWeakPtr< class IMenu > EntryMenu;

/*-----------------------------------------------------------------------------
	UInterpTrackAkAudioEventHelper
-----------------------------------------------------------------------------*/

UInterpTrackAkAudioEventHelper::UInterpTrackAkAudioEventHelper(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Property initialization
}

bool UInterpTrackAkAudioEventHelper::PreCreateKeyframe(UInterpTrack * Track, float KeyTime) const
{
	bool bResult = false;

	FEdModeInterpEdit* Mode = (FEdModeInterpEdit*)GLevelEditorModeTools().GetActiveMode(FBuiltinEditorModes::EM_InterpEdit);
	check(Mode != NULL);

	IMatineeBase* InterpEd = Mode->InterpEd;
	check(InterpEd != NULL);

	UAkAudioEvent* SelectedEvent = GEditor->GetSelectedObjects()->GetTop<UAkAudioEvent>();

	TSharedRef<SWidget> PropWidget = SNew(SMatineeAkEventKeyFrameAdder)
		.SelectedAkEvent(SelectedEvent)
		.OnAkEventSet(FOnAkEventSet::CreateUObject(this, &UInterpTrackAkAudioEventHelper::OnAkEventSet, InterpEd, Track));

	TSharedPtr< SWindow > Parent = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (Parent.IsValid())
	{
		EntryMenu = FSlateApplication::Get().PushMenu(
			Parent.ToSharedRef(),
			FWidgetPath(),
			PropWidget,
			FSlateApplication::Get().GetCursorPos(),
			FPopupTransitionEffect(FPopupTransitionEffect::None)
			);
	}

	return bResult;
}

void UInterpTrackAkAudioEventHelper::OnAkEventSet(UAkAudioEvent * in_SelectedAkEvent, const FString& in_AkEventName, IMatineeBase *InterpEd, UInterpTrack * ActiveTrack) const
{
	if (EntryMenu.IsValid())
	{
		EntryMenu.Pin()->Dismiss();
	}
	SelectedAkEvent = in_SelectedAkEvent;
	SelectedAkEventName = in_AkEventName;
	InterpEd->FinishAddKey(ActiveTrack, true);
}

void  UInterpTrackAkAudioEventHelper::PostCreateKeyframe( UInterpTrack *Track, int32 KeyIndex ) const
{
	UInterpTrackAkAudioEvent* AkEventTrack = CastChecked<UInterpTrackAkAudioEvent>(Track);

	// Assign the chosen AkEventCue to the new key.
	FAkAudioEventTrackKey& NewAkEventKey = AkEventTrack->Events[KeyIndex];
	NewAkEventKey.AkAudioEvent = SelectedAkEvent;
	NewAkEventKey.EventName = SelectedAkEventName;
}

#undef LOCTEXT_NAMESPACE
