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
	InterpTrackAkAudioRTPC.h:
=============================================================================*/

#include "InterpTrackAkAudioRTPC.h"
#include "AkAudioDevice.h"
#include "InterpTrackInstAkAudioEvent.h"
#include "InterpolationHitProxy.h"

/*-----------------------------------------------------------------------------
	UInterpTrackAkAudioRTPC
-----------------------------------------------------------------------------*/

UInterpTrackAkAudioRTPC::UInterpTrackAkAudioRTPC(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Property initialization
	TrackInstClass = UInterpTrackInstAkAudioEvent::StaticClass();
	TrackTitle = TEXT("Ak Audio RTPC");
}

void UInterpTrackAkAudioRTPC::SetTrackToSensibleDefault()
{
	FloatTrack.Points.Empty();

	FloatTrack.AddPoint( 0.0f, 0.0f );
}

int32 UInterpTrackAkAudioRTPC::AddKeyframe(float Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	float DefaultValue = 0.0f;
	float NewFloatValue = FloatTrack.Eval( Time, DefaultValue );

	int32 NewKeyIndex = FloatTrack.AddPoint( Time, NewFloatValue );
	FloatTrack.Points[NewKeyIndex].InterpMode = InitInterpMode;

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

void UInterpTrackAkAudioRTPC::PostLoad()
{
	Super::PostLoad();
	if (FloatTrack.Points.Num() <= 0)
	{
		SetTrackToSensibleDefault();
	}
}

void UInterpTrackAkAudioRTPC::UpdateTrack(float NewPosition, UInterpTrackInst* TrInst, bool bJump)
{
	AActor* Actor = TrInst->GetGroupActor();

	float DefaultValue = 0.0f;
	float NewFloatValue = FloatTrack.Eval( NewPosition, DefaultValue );

	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if( AudioDevice && Param.Len() )
	{
		AudioDevice->SetRTPCValue( *Param, NewFloatValue, 0, Actor );
	}
}

void UInterpTrackAkAudioRTPC::PreviewUpdateTrack(float NewPosition, UInterpTrackInst* TrInst)
{
	UpdateTrack(NewPosition, TrInst, false);
}

const FString UInterpTrackAkAudioRTPC::GetEdHelperClassName() const
{
	return FString( TEXT("AudiokineticTools.InterpTrackAkAudioRTPCHelper") );
}

const FString UInterpTrackAkAudioRTPC::GetSlateHelperClassName() const
{
	return FString( TEXT("AudiokineticTools.InterpTrackAkAudioRTPCHelper") );
}

#if WITH_EDITORONLY_DATA
UTexture2D* UInterpTrackAkAudioRTPC::GetTrackIcon() const
{
	return (UTexture2D*)StaticLoadObject( UTexture2D::StaticClass(), NULL, TEXT("/Engine/EditorMaterials/MatineeGroups/MAT_Groups_Sound.MAT_Groups_Sound"), NULL, LOAD_None, NULL );
}
#endif
