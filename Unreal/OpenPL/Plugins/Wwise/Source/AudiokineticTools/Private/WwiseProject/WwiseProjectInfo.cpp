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

#include "WwiseProjectInfo.h"

#include "AkSettings.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "XmlFile.h"

#include <AK/Tools/Common/AkFNVHash.h>

void WwiseProjectInfo::Parse()
{
	supportedPlatforms.Empty();
	supportedLanguages.Empty();
	defaultLanguage.Empty();
	cacheDirectory.Empty();

	FString projectPath = GetProjectPath();
	if (projectPath.Len() > 0)
	{
		FText errorMessage;
		int32 errorLineNumber;

		FFastXml::ParseXmlFile(this, *projectPath, nullptr, nullptr, false, false, errorMessage, errorLineNumber);

		if (cacheDirectory.Len() == 0)
		{
			cacheDirectory = FPaths::Combine(FPaths::GetPath(projectPath), TEXT(".cache"));
		}

		if (defaultLanguage.Len() == 0 || defaultLanguage == TEXT(""))
		{
			defaultLanguage = TEXT("English(US)");
		}
	}
}

bool WwiseProjectInfo::ProcessAttribute(const TCHAR* AttributeName, const TCHAR* AttributeValue)
{
	if (insidePlatformElement)
	{
		if (FCString::Strcmp(AttributeName, TEXT("Name")) == 0)
		{
			currentPlatformInfo.Name = AttributeValue;
		}
		else if (FCString::Strcmp(AttributeName, TEXT("ID")) == 0)
		{
			FGuid::ParseExact(AttributeValue, EGuidFormats::DigitsWithHyphensInBraces, currentPlatformInfo.ID);
		}
	}

	if (insideLanguageElement)
	{
		if (FCString::Strcmp(AttributeName, TEXT("Name")) == 0)
		{
			if (FCString::Strcmp(AttributeValue, TEXT("External")) == 0
				|| FCString::Strcmp(AttributeValue, TEXT("Mixed")) == 0
				|| FCString::Strcmp(AttributeValue, TEXT("SFX")) == 0)
			{
				insideLanguageElement = false;
			}
			else
			{
				if (currentLanguageInfo.Name.IsEmpty())
				{
					currentLanguageInfo.Name = AttributeValue;

					AK::FNVHash32 hash;
					FTCHARToUTF8 utf8(*currentLanguageInfo.Name.ToLower());
					currentLanguageInfo.ShortID = hash.Compute(utf8.Get(), utf8.Length());
				}
			}
		}
		else if (FCString::Strcmp(AttributeName, TEXT("ID")) == 0)
		{
			FGuid::ParseExact(AttributeValue, EGuidFormats::DigitsWithHyphensInBraces, currentLanguageInfo.ID);
		}
	}

	if (insideMiscSettingEntryElement && FCString::Strcmp(AttributeValue, TEXT("Cache")) == 0)
	{
		insideCacheSettings = true;
	}

	if (insidePropertyElement
		&& FCString::Strcmp(AttributeName, TEXT("Name")) == 0
		&& FCString::Strcmp(AttributeValue, TEXT("DefaultLanguage")) == 0
		)
	{
		insideDefaultLanguage = true;
	}

	if (insideDefaultLanguage && FCString::Strcmp(AttributeName, TEXT("Value")) == 0)
	{
		defaultLanguage = AttributeValue;
		insideDefaultLanguage = false;
	}

	return true;
}

bool WwiseProjectInfo::ProcessClose(const TCHAR* Element)
{
	if (insidePlatformElement && FCString::Strcmp(Element, TEXT("Platform")) == 0)
	{
		supportedPlatforms.Add(currentPlatformInfo);
		insidePlatformElement = false;
	}
	if (insideLanguageElement && FCString::Strcmp(Element, TEXT("Language")) == 0)
	{
		supportedLanguages.Add(currentLanguageInfo);
		insideLanguageElement = false;
	}
	if (insidePropertyElement && FCString::Strcmp(Element, TEXT("Property")) == 0)
	{
		insidePropertyElement = false;
	}
	return true;
}

bool WwiseProjectInfo::ProcessComment(const TCHAR* Comment)
{
	return true;
}

bool WwiseProjectInfo::ProcessElement(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber)
{
	if (insideMiscSettingEntryElement && insideCacheSettings && FCString::Strstr(ElementName, TEXT("CDATA")))
	{
		cacheDirectory = ElementName;
		cacheDirectory.RemoveFromStart(TEXT("![CDATA["));
		cacheDirectory.RemoveFromEnd(TEXT("]]"));

		if (auto* settings = GetDefault<UAkSettings>())
		{
			if (FPaths::IsRelative(cacheDirectory))
			{
				cacheDirectory = FPaths::ConvertRelativePathToFull(FPaths::GetPath(GetProjectPath()), cacheDirectory);
			}
		}

		return false;
	}

	if (FCString::Strcmp(ElementName, TEXT("Platform")) == 0)
	{
		insidePlatformElement = true;

		// Clear currentPlatformInfo
		new (&currentPlatformInfo) FWwisePlatformInfo();
	}
	else if (FCString::Strcmp(ElementName, TEXT("Language")) == 0)
	{
		insideLanguageElement = true;

		// Clear currentLanguageInfo
		new (&currentLanguageInfo) FWwiseLanguageInfo();
	}
	else if (FCString::Strcmp(ElementName, TEXT("Property")) == 0)
	{
		insidePropertyElement = true;
	}
	else if (FCString::Strcmp(ElementName, TEXT("MiscSettingEntry")) == 0)
	{
		insideMiscSettingEntryElement = true;
	}

	if (insideDefaultLanguage && FCString::Strcmp(ElementName, TEXT("Value")) == 0)
	{
		defaultLanguage = ElementData;
		insideDefaultLanguage = false;
	}

	return true;
}

bool WwiseProjectInfo::ProcessXmlDeclaration(const TCHAR* ElementData, int32 XmlFileLineNumber)
{
	return true;
}

FString WwiseProjectInfo::GetProjectPath() const
{
	FString projectPath;

	if (auto* settings = GetDefault<UAkSettings>())
	{
		projectPath = settings->WwiseProjectPath.FilePath;

		if (FPaths::IsRelative(projectPath))
		{
			projectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), projectPath);
		}
	}

	return projectPath;
}