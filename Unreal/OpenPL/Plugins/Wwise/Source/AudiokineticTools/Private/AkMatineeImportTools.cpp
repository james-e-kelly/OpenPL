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


#include "AkMatineeImportTools.h"
#include "AkAudioDevice.h"
#include "InterpTrackAkAudioRTPC.h"
#include "InterpTrackAkAudioEvent.h"
#include "MovieSceneAkAudioRTPCSection.h"
#include "MovieSceneAkAudioRTPCTrack.h"
#include "MovieSceneAkAudioEventSection.h"
#include "MovieSceneAkAudioEventTrack.h"
#include "AkUEFeatures.h"
#include "MatineeImportTools.h"

#include "MovieSceneCommonHelpers.h"
#include "ScopedTransaction.h"


#define LOCTEXT_NAMESPACE "AkAudio"

#if UE_4_26_OR_LATER
using namespace UE;
#endif

/** Copies keys from a matinee AkAudioRTPC track to a sequencer AkAudioRTPC track. */
ECopyInterpAkAudioResult FAkMatineeImportTools::CopyInterpAkAudioRTPCTrack(const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCTrack* AkAudioRTPCTrack)
{
	ECopyInterpAkAudioResult Result = ECopyInterpAkAudioResult::NoChange;
	const FScopedTransaction Transaction(LOCTEXT("PasteMatineeAkAudioRTPCTrack", "Paste Matinee AkAudioRtpc Track"));

	AkAudioRTPCTrack->Modify();

	// Get the name of the RTPC used on the Matinee track
	const FString& RTPCName = MatineeAkAudioRTPCTrack->Param;

	float MatineeTime = MatineeAkAudioRTPCTrack->GetKeyframeTime(0);
	FFrameRate FrameRate = AkAudioRTPCTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
	FFrameNumber KeyTime = FrameRate.AsFrameNumber(MatineeTime);

	UMovieSceneSection* Section = MovieSceneHelpers::FindSectionAtTime(AkAudioRTPCTrack->GetAllSections(), KeyTime);

	if (Section == nullptr)
	{
		Section = AkAudioRTPCTrack->CreateNewSection();
		AkAudioRTPCTrack->AddSection(*Section);
		Section->SetRange(TRange<FFrameNumber>::All());
		Result = ECopyInterpAkAudioResult::SectionAdded;
	}

	// if this cast fails, UMovieSceneAkAudioRTPCTrack must not be creating UMovieSceneAkAudioRTPCSection's - BIG FAIL
	UMovieSceneAkAudioRTPCSection* RtpcSection = Cast<UMovieSceneAkAudioRTPCSection>(Section);
	RtpcSection->SetRTPCName(RTPCName);

	if (Section->TryModify())
	{
		CopyInterpTrackToFloatChannel(MatineeAkAudioRTPCTrack, RtpcSection, Result);
	}

	return Result;
}
void FAkMatineeImportTools::CopyInterpTrackToFloatChannel(const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCSection* RtpcSection, ECopyInterpAkAudioResult& Result)
{
	float SectionMin = RtpcSection->GetStartTime();
	float SectionMax = RtpcSection->GetEndTime();

	FMovieSceneFloatChannel FloatChannel = RtpcSection->GetChannel();
	auto& Points = MatineeAkAudioRTPCTrack->FloatTrack.Points;

	FFrameRate FrameRate = RtpcSection->GetTypedOuter<UMovieScene>()->GetTickResolution();
	FFrameNumber FirstKeyTime = (MatineeAkAudioRTPCTrack->GetKeyframeTime(0) * FrameRate).RoundToFrame();;

	if (ECopyInterpAkAudioResult::NoChange == Result && Points.Num() > 0)
		Result = ECopyInterpAkAudioResult::KeyModification;

	TMovieSceneChannelData<FMovieSceneFloatValue> ChannelData = FloatChannel.GetData();
	TRange<FFrameNumber> KeyRange = TRange<FFrameNumber>::Empty();
	for (const auto& Point : Points)
	{
		FFrameNumber KeyTime = (Point.InVal * FrameRate).RoundToFrame();

#if UE_4_21_OR_LATER
		FMatineeImportTools::SetOrAddKey(ChannelData, KeyTime, Point.OutVal, Point.ArriveTangent, Point.LeaveTangent, Point.InterpMode, FrameRate);
#else
		FMatineeImportTools::SetOrAddKey(ChannelData, KeyTime, Point.OutVal, Point.ArriveTangent, Point.LeaveTangent, Point.InterpMode);
#endif

		KeyRange = TRange<FFrameNumber>::Hull(KeyRange, TRange<FFrameNumber>(KeyTime));
	}

	FKeyDataOptimizationParams Params;
	Params.bAutoSetInterpolation = true;

	MovieScene::Optimize(&FloatChannel, Params);

	if (!KeyRange.IsEmpty())
	{
		RtpcSection->SetRange(KeyRange);
	}
}

/** Copies keys from a matinee AkAudioEvent track to a sequencer AkAudioEvent track. */
ECopyInterpAkAudioResult FAkMatineeImportTools::CopyInterpAkAudioEventTrack(const UInterpTrackAkAudioEvent* MatineeAkAudioEventTrack, UMovieSceneAkAudioEventTrack* AkAudioEventTrack)
{
	ECopyInterpAkAudioResult Result = ECopyInterpAkAudioResult::NoChange;
	const FScopedTransaction Transaction(LOCTEXT("PasteMatineeAkAudioEventTrack", "Paste Matinee AkAudioEvent Track"));

	AkAudioEventTrack->Modify();

	auto& Events = MatineeAkAudioEventTrack->Events;
	for (const auto& Event : Events)
	{
		FFrameRate FrameRate = AkAudioEventTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
		FFrameNumber EventTime = FrameRate.AsFrameNumber(Event.Time);
		if (AkAudioEventTrack->AddNewEvent(EventTime, Event.AkAudioEvent, Event.EventName))
		{
			Result = ECopyInterpAkAudioResult::SectionAdded;
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE

