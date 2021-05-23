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

#include "AkAudioType.h"

#include "Async/Async.h"
#include "AkAudioDevice.h"
#include "AkGroupValue.h"
#include "AkFolder.h"
#include "Core/Public/Modules/ModuleManager.h"

void UAkAudioType::PostLoad()
{
	static FName AkAudioName = TEXT("AkAudio");
	Super::PostLoad();
	if (FModuleManager::Get().IsModuleLoaded(AkAudioName))
	{
		Load();
	}
	else
	{
		FAkAudioDevice::DelayAssetLoad(this);
	}
}

void UAkAudioType::Load()
{
	if (auto AudioDevice = FAkAudioDevice::Get())
	{
		auto idFromName = AudioDevice->GetIDFromString(GetName());
		if (ShortID == 0)
		{
			ShortID = idFromName;
		}
		else if (!IsA<UAkGroupValue>() && !IsA<UAkFolder>() && ShortID != 0 && ShortID != idFromName)
		{
			UE_LOG(LogAkAudio, Error, TEXT("%s - Current Short ID '%u' is different from ID from the name '%u'"), *GetName(), ShortID, idFromName);
		}
	}
}

#if WITH_EDITOR
void UAkAudioType::Reset()
{
	ShortID = 0;

	AsyncTask(ENamedThreads::GameThread, [this] {
		MarkPackageDirty();
	});
}
#endif
