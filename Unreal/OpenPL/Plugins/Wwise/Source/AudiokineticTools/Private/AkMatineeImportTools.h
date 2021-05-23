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
#include "AkUEFeatures.h"
#include "MovieSceneAkAudioRTPCSection.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

class UInterpTrackAkAudioEvent;
class UInterpTrackAkAudioRTPC;

class UMovieSceneAkAudioEventTrack;
class UMovieSceneAkAudioRTPCTrack;

enum class ECopyInterpAkAudioResult
{
	NoChange,
	KeyModification,
	SectionAdded,
};

/**
 * Tools for AkAudio tracks
 */
class FAkMatineeImportTools
{
public:

	/** Attempts to retrieve a specific InterpTrackType from the MatineeCopyPasteBuffer. */
	template<typename InterpTrackType>
	static InterpTrackType* GetTrackFromMatineeCopyPasteBuffer()
	{
		for (auto CopyPasteObject : GUnrealEd->MatineeCopyPasteBuffer)
		{
			auto Track = Cast<InterpTrackType>(CopyPasteObject);
			if (Track != nullptr)
			{
				return Track;
			}
		}

		return nullptr;
	}

	/** Copies keys from a matinee AkAudioRTPC track to a sequencer AkAudioRTPC track. */
	static ECopyInterpAkAudioResult CopyInterpAkAudioRTPCTrack(const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCTrack* AkAudioRTPCTrack);

	/** Copies keys from a matinee AkAudioEvent track to a sequencer AkAudioEvent track. */
	static ECopyInterpAkAudioResult CopyInterpAkAudioEventTrack(const UInterpTrackAkAudioEvent* MatineeAkAudioEventTrack, UMovieSceneAkAudioEventTrack* AkAudioEventTrack);

private:
	static void CopyInterpTrackToFloatChannel(const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCSection* RtpcSection, ECopyInterpAkAudioResult& Result);
};
