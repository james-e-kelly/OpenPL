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

#include "Containers/Array.h"
#include "Containers/Set.h"
#include "Containers/UnrealString.h"
#include "FastXml.h"

struct FWwiseLanguageInfo
{
	FGuid ID;
	FString Name;
	uint32 ShortID;
};

struct FWwisePlatformInfo
{
	FGuid ID;
	FString Name;
};

class WwiseProjectInfo : public IFastXmlCallback
{
public:
	virtual ~WwiseProjectInfo() {}

	void Parse();

	FString const& CacheDirectory() const { return cacheDirectory; }

	TArray<FWwisePlatformInfo> const& SupportedPlatforms() const { return supportedPlatforms; }
	TArray<FWwiseLanguageInfo> const& SupportedLanguages() const { return supportedLanguages; }
	FString DefaultLanguage() const { return defaultLanguage; }

	bool ProcessAttribute(const TCHAR* AttributeName, const TCHAR* AttributeValue) override;
	bool ProcessClose(const TCHAR* Element) override;
	bool ProcessComment(const TCHAR* Comment) override;
	bool ProcessElement(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber) override;
	bool ProcessXmlDeclaration(const TCHAR* ElementData, int32 XmlFileLineNumber) override;

private:
	FString GetProjectPath() const;

private:
	TArray<FWwiseLanguageInfo> supportedLanguages;
	TArray<FWwisePlatformInfo> supportedPlatforms;
	FString defaultLanguage;
	FString cacheDirectory;

	bool insidePlatformElement = false;
	bool insideLanguageElement = false;
	bool insideMiscSettingEntryElement = false;
	bool insideCacheSettings = false;
	bool insidePropertyElement = false;
	bool insideDefaultLanguage = false;

	FWwiseLanguageInfo currentLanguageInfo;
	FWwisePlatformInfo currentPlatformInfo;
};