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

#include "UObject/Object.h"
#include "AkAudioType.generated.h"

UCLASS()
class AKAUDIO_API UAkAudioType : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, AssetRegistrySearchable, Category = "AkAudioType")
	FGuid ID;
#endif

#if WITH_EDITOR
	virtual void Reset();
#endif

	UPROPERTY(VisibleAnywhere, AssetRegistrySearchable, Category="AkAudioType")
	uint32 ShortID;

	UPROPERTY(EditAnywhere, Category = "AkAudioType")
	TArray<UObject*> UserData;

	void PostLoad() override;

	virtual void Load();

public:
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
};
