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

#include "AkAudioEvent.h"
#include "Matinee/InterpTrack.h"
#include "IMatinee.h"

#include "InterpTrackHelper.h"
#include "InterpTrackAkAudioEventHelper.generated.h"

UCLASS()
class UInterpTrackAkAudioEventHelper : public UInterpTrackHelper
{
	GENERATED_UCLASS_BODY()

	void OnAkEventSet(UAkAudioEvent * in_SelectedAkEvent, const FString& in_AkEventName, IMatineeBase *InterpEd, UInterpTrack * ActiveTrack) const;

	// Begin UInterpTrackHelper Interface
	virtual	bool PreCreateKeyframe( UInterpTrack *Track, float KeyTime ) const override;
	virtual void  PostCreateKeyframe( UInterpTrack *Track, int32 KeyIndex ) const override;
	// End UInterpTrackHelper Interface

private:
	mutable UAkAudioEvent * SelectedAkEvent;
	mutable FString SelectedAkEventName;
};
