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

#include "AkToolBehavior.h"

#include "AkUnrealHelper.h"
#include "AkEventBasedToolBehavior.h"
#include "AkLegacyToolBehavior.h"

AkToolBehavior* AkToolBehavior::s_Instance = nullptr;

AkToolBehavior* AkToolBehavior::Get()
{
	if (!s_Instance)
	{
		if (AkUnrealHelper::IsUsingEventBased())
		{
			s_Instance = new AkEventBasedToolBehavior;
		}
		else
		{
			s_Instance = new AkLegacyToolBehavior;
		}
	}

	return s_Instance;
}

bool AkToolBehavior::AkAssetManagementManager_ModifyProjectSettings(FString& ProjectContent)
{
	static const TArray<PropertyToChange> PropertiesToAdd = {
		{ TEXT("GenerateMultipleBanks"), TEXT("True"), TEXT("<Property Name=\"GenerateMultipleBanks\" Type=\"bool\" Value=\"True\"/>") },
		{ TEXT("GenerateSoundBankJSON"), TEXT("True"), TEXT("<Property Name=\"GenerateSoundBankJSON\" Type=\"bool\" Value=\"True\"/>") },
		{ TEXT("SoundBankGenerateEstimatedDuration"), TEXT("True"), TEXT("<Property Name=\"SoundBankGenerateEstimatedDuration\" Type=\"bool\" Value=\"True\"/>") },
		{ TEXT("SoundBankGenerateMaxAttenuationInfo"), TEXT("True"), TEXT("<Property Name=\"SoundBankGenerateMaxAttenuationInfo\" Type=\"bool\" Value=\"True\"/>") },
		{ TEXT("SoundBankGeneratePrintGUID"), TEXT("True"), TEXT("<Property Name=\"SoundBankGeneratePrintGUID\" Type=\"bool\" Value=\"True\"/>") },
	};

	return InsertProperties(PropertiesToAdd, ProjectContent);
}

bool AkToolBehavior::InsertProperties(const TArray<PropertyToChange>& PropertiesToChange, FString& ProjectContent)
{
	static const auto PropertyListStart = TEXT("<PropertyList>");
	static const FString ValueText = TEXT("Value=\"");
	bool modified = false;

	int32 propertyListPosition = ProjectContent.Find(PropertyListStart);
	if (propertyListPosition != -1)
	{
		int32 insertPosition = propertyListPosition + FCString::Strlen(PropertyListStart);

		for (auto itemToAdd : PropertiesToChange)
		{
			auto idx = ProjectContent.Find(itemToAdd.Name);
			if (idx == -1)
			{
				ProjectContent.InsertAt(insertPosition, FString::Printf(TEXT("\n\t\t\t\t%s"), *itemToAdd.Xml));
				modified = true;
			}
			else
			{
				auto valueIdx = ProjectContent.Find(ValueText, ESearchCase::IgnoreCase, ESearchDir::FromStart, idx);
				auto EndTagIdx = ProjectContent.Find(TEXT(">"), ESearchCase::IgnoreCase, ESearchDir::FromStart, idx);
				if (valueIdx != -1 && valueIdx > idx && valueIdx < EndTagIdx)
				{
					valueIdx += ValueText.Len();
					auto valueEndIdx = ProjectContent.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, valueIdx);
					if (valueEndIdx != -1)
					{
						FString value = ProjectContent.Mid(valueIdx, valueEndIdx - valueIdx);
						if (value != itemToAdd.Value)
						{
							ProjectContent.RemoveAt(valueIdx, valueEndIdx - valueIdx, false);
							ProjectContent.InsertAt(valueIdx, itemToAdd.Value);
							modified = true;
						}
					}
				}
			}
		}
	}

	return modified;
}
