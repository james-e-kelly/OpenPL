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


/*------------------------------------------------------------------------------------
	SMatineeAkEventKeyFrameAdder.h
------------------------------------------------------------------------------------*/
#pragma once

/*------------------------------------------------------------------------------------
	SMatineeAkEventKeyFrameAdder
------------------------------------------------------------------------------------*/
#include "AkAudioEvent.h"
#include "Delegates/Delegate.h"
#include "Widgets/Input/SButton.h"
#include "AssetData.h"

/** Delegate used when the user clicks OK */
DECLARE_DELEGATE_TwoParams(FOnAkEventSet, UAkAudioEvent *, const FString&);

class SMatineeAkEventKeyFrameAdder : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SMatineeAkEventKeyFrameAdder )
		: _SelectedAkEvent( NULL )
	{}
		/** Currently selected AkEvent */
		SLATE_ARGUMENT( UAkAudioEvent*, SelectedAkEvent )
		/** Delegate used when the user clicks OK */
		SLATE_EVENT(FOnAkEventSet, OnAkEventSet)
	SLATE_END_ARGS( )

	SMatineeAkEventKeyFrameAdder(void);

	void Construct(const FArguments& InArgs);


private:
	UAkAudioEvent* SelectedAkEvent;
	FString EventName;
	FOnAkEventSet OnAkEventSet;

	void OnEventChanged(const FAssetData& InAssetData);
	FReply OnOKClicked();
	void OnEventNameCommited(const FText& InText, ETextCommit::Type InCommitType);
	FString GetSelectedAkEventPath() const;

	EVisibility IsAdvancedVisible() const;
	FReply OnAdvancedClicked();
	bool bAdvancedVisible;
	TSharedPtr<SButton> AdvancedButton;
	const FSlateBrush* GetAdvancedButtonImage() const;
	float GetAdvancedFeaturesHeight() const;


};