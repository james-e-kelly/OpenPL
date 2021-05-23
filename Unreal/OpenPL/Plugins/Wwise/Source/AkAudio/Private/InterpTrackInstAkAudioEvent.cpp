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
	InterpTrackInstAkEvent.h:
=============================================================================*/

#include "InterpTrackInstAkAudioEvent.h"
#include "AkAudioDevice.h"
#include "Matinee/InterpGroupInst.h"
#include "Matinee/MatineeActor.h"

/*-----------------------------------------------------------------------------
	UInterpTrackInstAkAudioEvent
-----------------------------------------------------------------------------*/

UInterpTrackInstAkAudioEvent::UInterpTrackInstAkAudioEvent(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Property initialization
}

void UInterpTrackInstAkAudioEvent::InitTrackInst(UInterpTrack* Track)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( GetOuter() );
	AMatineeActor* MatineeActor = CastChecked<AMatineeActor>( GrInst->GetOuter() );
	LastUpdatePosition = MatineeActor->InterpPosition;
}

void UInterpTrackInstAkAudioEvent::TermTrackInst(UInterpTrack* Track)
{
}