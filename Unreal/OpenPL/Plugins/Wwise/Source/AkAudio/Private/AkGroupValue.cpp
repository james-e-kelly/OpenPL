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

#include "AkGroupValue.h"

#include "AkAudioDevice.h"
#include "AkMediaAsset.h"

void UAkGroupValue::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	for (auto& entry : MediaDependencies)
	{
		if (auto* media = entry.Get())
		{
			LoadedMediaDependencies.Add(media);
			media->Load(true);
		}
	}
}

bool UAkGroupValue::IsReadyForAsyncPostLoad() const
{
	for (auto& entry : LoadedMediaDependencies)
	{
		if (entry.IsValid())
		{
			if (!entry->IsReadyForAsyncPostLoad())
			{
				return false;
			}
		}
	}

	return true;
}

void UAkGroupValue::PostLoad()
{
	Super::PostLoad();

	if (GroupShortID == 0)
	{
		if (auto AudioDevice = FAkAudioDevice::Get())
		{
			FString GroupName;
			GetName().Split(TEXT("-"), &GroupName, nullptr);
			auto idFromName = AudioDevice->GetIDFromString(GroupName);
			GroupShortID = idFromName;
		}
	}
}

void UAkGroupValue::BeginDestroy()
{
	Super::BeginDestroy();

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		for (auto& entry : LoadedMediaDependencies)
		{
			if (entry.IsValid(true))
			{
				entry->Unload();
			}

		}
		LoadedMediaDependencies.Empty();
	}
}
