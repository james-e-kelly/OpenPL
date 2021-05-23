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

#include "Engine/GameEngine.h"
#include "AkPlatformInfo.generated.h"

UCLASS()
class AKAUDIO_API UAkPlatformInfo : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	static TMap<FString, FString> UnrealNameToWwiseName;
#endif
	static UAkPlatformInfo* GetAkPlatformInfo(const FString& PlatformName)
	{
		FString PlatformInfoClassName = FString::Format(TEXT("Ak{0}PlatformInfo"), { *PlatformName });
		auto* PlatformInfoClass = FindObject<UClass>(ANY_PACKAGE, *PlatformInfoClassName);
		if (!PlatformInfoClass)
		{
			return nullptr;
		}

		return PlatformInfoClass->GetDefaultObject<UAkPlatformInfo>();
	}

	FString WwisePlatform;
	FString Architecture;
	FString LibraryFileNameFormat;
	FString DebugFileNameFormat;
	bool bSupportsUPL = false;
	bool bUsesStaticLibraries = false;
	bool bForceReleaseConfig = false;
};
