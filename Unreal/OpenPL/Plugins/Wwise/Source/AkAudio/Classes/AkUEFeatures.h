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

// Defines which features of the Wwise-Unreal integration are supported in which version of UE.

#pragma once

#include "Runtime/Launch/Resources/Version.h"

#define UE_4_21_OR_LATER (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 21)
#define UE_4_22_OR_LATER (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 22)
#define UE_4_23_OR_LATER (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 23)
#define UE_4_24_OR_LATER (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 24)
#define UE_4_25_OR_LATER (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 25)
#define UE_4_26_OR_LATER (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26)

#if UE_4_22_OR_LATER
#define AK_DEPRECATED UE_DEPRECATED
#else
#define AK_DEPRECATED DEPRECATED
#endif

