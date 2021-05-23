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

#include "WwiseWorkUnitParser.h"

#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "HAL/FileManager.h"
#include "XmlFile.h"
#include "AkUnrealHelper.h"
#include "WorkUnitXmlVisitor.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "Misc/FileHelper.h"
#include "Internationalization/Regex.h"
#include "WwiseProjectParser.h"
#include "HAL/PlatformFileManager.h"

#define LOCTEXT_NAMESPACE "AkAudio"

DEFINE_LOG_CATEGORY_STATIC(LogWwiseWorkUnitParser, Log, All);

bool WwiseWorkUnitParser::Parse()
{
	if (!visitor)
	{
		return false;
	}

	auto projectFilePath = AkUnrealHelper::GetWwiseProjectPath();

	if (!FPaths::FileExists(projectFilePath))
	{
		return false;
	}

	visitor->OnBeginParse();
	ParsePhysicalFolders(); 
	projectRootFolder = FPaths::GetPath(projectFilePath) + TEXT("/");
	for (int i = EWwiseItemType::Event; i <= EWwiseItemType::LastWwiseDraggable; ++i)
	{
		const auto CurrentType = static_cast<EWwiseItemType::Type>(i);
		visitor->Init(CurrentType);
		parseFolders(EWwiseItemType::FolderNames[i], CurrentType);
	}
	visitor->End();

	return true;
}

bool WwiseWorkUnitParser::ForceParse()
{
	if (!visitor)
	{
		return false;
	}

	wwuLastPopulateTime.Reset();
	visitor->ForceInit();
	return Parse();
}

void WwiseWorkUnitParser::ParsePhysicalFolders()
{
	FScopedSlowTask SlowTask(2.f, LOCTEXT("AK_PopulatingPickerPhysicalFolders", "Parsing Wwise Physical Folders..."));
	SlowTask.MakeDialog();
	visitor->OnBeginPhysicalFolderParse();
	auto projectFilePath = AkUnrealHelper::GetWwiseProjectPath();
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*projectFilePath))
	{
		return;
	}

	//replace CDATA elements in xml so they don't kill the parser
	FString PatternString(TEXT("<\\!\\[CDATA[\\s\\S]*?>"));
	FString fileString;
	bool readSuccess = FFileHelper::LoadFileToString(fileString, *projectFilePath);

	if (!readSuccess)
	{
		UE_LOG(LogWwiseWorkUnitParser, Error, TEXT("Could not read the project file '<%s>' while syncing assets. Some folders may not appear in the content browser or in the Wwise picker."), *projectFilePath);
		return;
	}

	FRegexPattern Pattern(PatternString);
	FRegexMatcher Matcher(Pattern, fileString);
	while (Matcher.FindNext())
	{
		auto t = Matcher.GetCaptureGroup(0);
		fileString = fileString.Replace(*t, TEXT(""));
	}

	WwiseProjectParser parser(visitor);
	FText errorOut;
	int32 errorLineNumber;
	FFastXml::ParseXmlFile(&parser, NULL, fileString.GetCharArray().GetData(), NULL, false, false, errorOut,  errorLineNumber);
	if (!errorOut.IsEmpty())
	{
		UE_LOG(LogWwiseWorkUnitParser, Error, TEXT("Failed to parse the project file '<%s>' while syncing assets. Some folders may not appear in the content browser or in the Wwise picker. \n Line <%i> - Error message: <%s>."), *projectFilePath, errorLineNumber, *errorOut.ToString());
	}
	else 
	{
		visitor->EndPhysicalFolderParse();
	}
}

void WwiseWorkUnitParser::parseFolders(const FString& FolderName, EWwiseItemType::Type ItemType)
{
	TArray<FString> NewWwus;
	TArray<FString> KnownWwus;
	TArray<FString> WwusToProcess;
	TArray<FString> WwusToRemove;
	FString FullPath = FPaths::Combine(projectRootFolder, FolderName);

	IFileManager::Get().FindFilesRecursive(NewWwus, *FullPath, TEXT("*.wwu"), true, false);

	TMap<FString, FDateTime>& LastPopTimeMap = wwuLastPopulateTime.FindOrAdd(ItemType);
	LastPopTimeMap.GetKeys(KnownWwus);

	// Get lists of files to parse, and files that have been deleted
	NewWwus.Sort();
	KnownWwus.Sort();
	int32 iKnown = 0;
	int32 iNew = 0;

	while (iNew < NewWwus.Num() && iKnown < KnownWwus.Num())
	{
		if (KnownWwus[iKnown] == NewWwus[iNew])
		{
			// File was there and is still there.  Check the FileTimes.
			FDateTime LastPopTime = LastPopTimeMap[KnownWwus[iKnown]];
			FDateTime LastModifiedTime = IFileManager::Get().GetTimeStamp(*NewWwus[iNew]);
			if (LastPopTime < LastModifiedTime)
			{
				WwusToProcess.Add(KnownWwus[iKnown]);
			}
			iKnown++;
			iNew++;
		}
		else if (KnownWwus[iKnown] > NewWwus[iNew])
		{
			// New Wwu detected. Add it to the ToProcess list
			WwusToProcess.Add(NewWwus[iNew]);
			iNew++;
		}
		else
		{
			// A file was deleted. Can't process it now, it would change the array indices.
			WwusToRemove.Add(KnownWwus[iKnown]);
			iKnown++;
		}
	}

	//The remainder from the files found on disk are all new files.
	for (; iNew < NewWwus.Num(); iNew++)
	{
		WwusToProcess.Add(NewWwus[iNew]);
	}

	//All the remainder is deleted.
	while (iKnown < KnownWwus.Num())
	{
		visitor->RemoveWorkUnit(KnownWwus[iKnown]);
		LastPopTimeMap.Remove(KnownWwus[iKnown]);
		iKnown++;
	}

	//Delete those tagged.
	for (FString& ToRemove : WwusToRemove)
	{
		visitor->RemoveWorkUnit(ToRemove);
		LastPopTimeMap.Remove(ToRemove);
	}

	FScopedSlowTask SlowTask(WwusToProcess.Num(), LOCTEXT("AK_PopulatingPicker", "Parsing Wwise Work Unit..."));
	SlowTask.MakeDialog();

	preParseWorkUnits(WwusToProcess, ItemType);

	for(auto wwuItem : standaloneWwusToParse) 
	{
		auto info = wwuItem.Value;
		parseWorkUnitFile(info, getRelativePath(info, ItemType), ItemType, &SlowTask);
	}

	TArray<FGuid>keyCopy;
	nestedWwusToParse.GetKeys(keyCopy);

	//Parse nested work units in correct order
	for (int i=0; i < keyCopy.Num();)
	{
		auto currentWwuGuid = keyCopy[i];
		//skip if already parsed through recursion
		if (!nestedWwusToParse.Contains(currentWwuGuid)) {
			i++;
			continue;
		}

		auto wwuInfo = nestedWwusToParse[currentWwuGuid];
		//parse parent first
		if (nestedWwusToParse.Contains(wwuInfo.parentWorkUnitGuid))
		{
			keyCopy.Swap(i, keyCopy.IndexOfByKey(wwuInfo.parentWorkUnitGuid));
			continue;
		}

		parseWorkUnitFile(wwuInfo, getRelativePath(wwuInfo, ItemType), ItemType, &SlowTask);
		i++;
	}
}

void WwiseWorkUnitParser::preParseWorkUnits(const TArray<FString>& WwusToProcess, EWwiseItemType::Type ItemType)
{
	parsedWwus.Empty();
	nestedWwusToParse.Empty();
	standaloneWwusToParse.Empty();
	unparseableWwus.Empty();

	for (int i = 0; i < WwusToProcess.Num(); i++)
	{
		auto workUnitPath = WwusToProcess[i];
		FXmlFile workUnitXml(workUnitPath);
		if (!workUnitXml.IsValid()) 
		{
			unparseableWwus.Add(workUnitPath);
			visitor->RegisterError(workUnitPath, workUnitXml.GetLastError());
		}
		auto workUnitInfo = peekWorkUnit(workUnitPath, ItemType);
		if (workUnitInfo.successfullyParsed)
		{
			if (workUnitInfo.isStandalone) {
				standaloneWwusToParse.Add(workUnitInfo.wwuGuid, workUnitInfo);
			}
			else
			{
				nestedWwusToParse.Add(workUnitInfo.wwuGuid, workUnitInfo);
			}
		}
		else 
		{
			visitor->RegisterError(workUnitInfo.wwuPath, TEXT("XML was valid, but did not have the expected structure."));
		}
	}
}

FString WwiseWorkUnitParser::getRelativePath(const WorkUnitInfo& info, EWwiseItemType::Type ItemType)
{
	FString relativePath;
	if (info.isStandalone) 
	{
		relativePath = info.wwuPath;
		relativePath.RemoveFromStart(projectRootFolder);
		relativePath.RemoveFromEnd(TEXT(".wwu"));
	}
	else
	{
		auto parentPath = visitor->FindRelativePath(info.wwuPath, info.parentWorkUnitGuid, ItemType);
		relativePath = FString::Format(TEXT("{0}/{1}"), { parentPath, info.wwuName });
	}
	return relativePath;
}

void WwiseWorkUnitParser::parseWorkUnitFile(const WorkUnitInfo& wwuInfo, const FString& RelativePath, EWwiseItemType::Type ItemType, FScopedSlowTask* SlowTask)
{
	if (SlowTask) 
	{
		FString Message = TEXT("Parsing WorkUnit: ") + FPaths::GetCleanFilename(wwuInfo.wwuName);
		SlowTask->EnterProgressFrame(1.0f, FText::FromString(Message));
	}

	FDateTime LastModifiedTime = IFileManager::Get().GetTimeStamp(*wwuInfo.wwuPath);
	
	visitor->EnterWorkUnit(wwuInfo, RelativePath, ItemType);	
	if (parseWorkUnitXml(wwuInfo.wwuPath, RelativePath, ItemType)) 
	{
		FDateTime& Time = wwuLastPopulateTime.FindOrAdd(ItemType).FindOrAdd(wwuInfo.wwuPath);
	    Time = LastModifiedTime;
	}
	else
	{
		visitor->RegisterError(wwuInfo.wwuPath, TEXT("XML was valid, but did not have the expected structure."));
	}
	visitor->ExitWorkUnit(wwuInfo.isStandalone);
}

WwiseWorkUnitParser::WorkUnitInfo WwiseWorkUnitParser::peekWorkUnit(const FString& WwuFilePath, EWwiseItemType::Type ItemType)
{
	bool isStandalone = false;
	bool successfullyParsed = false;
	FGuid guid;
	FString name;
	FGuid parentGuid;
	FXmlFile Wwu(WwuFilePath);
	if (Wwu.IsValid())
	{
		const FXmlNode* RootNode = Wwu.GetRootNode();
		if (RootNode)
		{
			const FXmlNode* EventsNode = RootNode->FindChildNode(EWwiseItemType::DisplayNames[ItemType]);
			if (EventsNode)
			{
				successfullyParsed = true;
				const FXmlNode* WorkUnitNode = EventsNode->FindChildNode(TEXT("WorkUnit"));
				if (WorkUnitNode)
				{
					successfullyParsed = true;
					FString WorkUnitPersistMode = WorkUnitNode->GetAttribute(TEXT("PersistMode"));
					if (WorkUnitPersistMode == TEXT("Standalone"))
					{
						isStandalone = true;
					}
					name = WorkUnitNode->GetAttribute(TEXT("Name"));
					FGuid::ParseExact(WorkUnitNode->GetAttribute(TEXT("ID")), EGuidFormats::DigitsWithHyphensInBraces, guid);
					auto parentIDString = WorkUnitNode->GetAttribute(TEXT("OwnerID"));
					if (!parentIDString.IsEmpty())
					{
						FGuid::ParseExact(parentIDString, EGuidFormats::DigitsWithHyphensInBraces, parentGuid);
					}
				}
			}
		}
	}
	return { successfullyParsed, isStandalone,  name, WwuFilePath, guid, parentGuid };
}

bool WwiseWorkUnitParser::parseWorkUnitXml( const FString& WorkUnitPath, const FString& RelativePath, EWwiseItemType::Type ItemType)
{
	FXmlFile WorkUnitXml(WorkUnitPath);
	if (!WorkUnitXml.IsValid())
	{
		return false;
	}

	const FXmlNode* RootNode = WorkUnitXml.GetRootNode();
	if (!RootNode)
	{
		return false;
	}

	const FXmlNode* ItemNode = RootNode->FindChildNode(EWwiseItemType::DisplayNames[ItemType]);
	if (!ItemNode)
	{
		return false;
	}

	const FXmlNode* WorkUnitNode = ItemNode->FindChildNode(TEXT("WorkUnit"));
	if (!WorkUnitNode || (WorkUnitNode->GetAttribute(TEXT("Name")) != FPaths::GetBaseFilename(WorkUnitPath)))
	{
		return false;
	}
	FGuid CurrentId;
	FGuid::ParseExact(WorkUnitNode->GetAttribute(TEXT("ID")), EGuidFormats::DigitsWithHyphensInBraces, CurrentId);

	const FXmlNode* CurrentNode = WorkUnitNode->FindChildNode(TEXT("ChildrenList"));
	if (!CurrentNode)
	{
		return true;
	}

	CurrentNode = CurrentNode->GetFirstChildNode();
	if (!CurrentNode)
	{
		return true;
	}

	parseWorkUnitChildren(CurrentNode, WorkUnitPath, RelativePath, ItemType, CurrentId);
	return true;
}

void WwiseWorkUnitParser::recurse(const FXmlNode* CurrentNode, const FString& WorkUnitPath, const FString& CurrentPath, EWwiseItemType::Type ItemType, const FGuid& ParentId)
{
	if (const FXmlNode* ChildrenNode = CurrentNode->FindChildNode(TEXT("ChildrenList")))
	{
		if (const FXmlNode* FirstChildNode = ChildrenNode->GetFirstChildNode())
		{
			parseWorkUnitChildren(FirstChildNode, WorkUnitPath, CurrentPath, ItemType, ParentId);
		}
	}
}

void WwiseWorkUnitParser::parseWorkUnitChildren(const FXmlNode* NodeToParse, const FString& WorkUnitPath, const FString& RelativePath, EWwiseItemType::Type ItemType, const FGuid& ParentId)
{
	for (const FXmlNode* CurrentNode = NodeToParse; CurrentNode; CurrentNode = CurrentNode->GetNextNode())
	{
		FString CurrentTag = CurrentNode->GetTag();
		FString CurrentName = CurrentNode->GetAttribute(TEXT("Name"));
		FString CurrentPath = FPaths::Combine(RelativePath, CurrentName);
		FString CurrentStringId = CurrentNode->GetAttribute(TEXT("ID"));

		FGuid CurrentId;
		FGuid::ParseExact(CurrentStringId, EGuidFormats::DigitsWithHyphensInBraces, CurrentId);
		
		if (CurrentTag == TEXT("Event"))
		{
			visitor->EnterEvent(CurrentId, CurrentName, CurrentPath);
		}
		else if (CurrentTag == TEXT("AcousticTexture"))
		{
			if (ItemType == EWwiseItemType::Type::AcousticTexture)
			{
				visitor->EnterAcousticTexture(CurrentId, CurrentNode, CurrentName, CurrentPath);
			}
		}
		else if (CurrentTag == TEXT("AuxBus"))
		{
			visitor->EnterAuxBus(CurrentId, CurrentName, CurrentPath);
			recurse(CurrentNode, WorkUnitPath, CurrentPath, ItemType, CurrentId);
			visitor->ExitAuxBus();
		}
		else if (CurrentTag == TEXT("WorkUnit"))
		{
			if (nestedWwusToParse.Contains(CurrentId))
			{
				auto info = nestedWwusToParse[CurrentId];
				info.parentGuid = ParentId;
				parseWorkUnitFile(info, CurrentPath, ItemType);
				nestedWwusToParse.Remove(CurrentId);
			}
			else
			{
				FString currentWwuPhysicalPath = FPaths::Combine(*FPaths::GetPath(WorkUnitPath), *CurrentName) + ".wwu";
				if (!unparseableWwus.Contains(currentWwuPhysicalPath))
				{
					auto info = peekWorkUnit(currentWwuPhysicalPath, ItemType);
					info.parentGuid = ParentId;
					if (info.successfullyParsed)
					{
						parseWorkUnitFile(info, CurrentPath, ItemType);
					}
					else
					{
						visitor->RegisterError(info.wwuPath, TEXT("XML was valid, but did not have the expected structure."));
					}
				}
			}
		}
		else if (CurrentTag == TEXT("SwitchGroup"))
		{
			visitor->EnterSwitchGroup(CurrentId, CurrentName, CurrentPath);
			recurse(CurrentNode, WorkUnitPath, CurrentPath, ItemType, CurrentId);
			visitor->ExitSwitchGroup();
		}
		else if (CurrentTag == TEXT("Switch"))
		{
			visitor->EnterSwitch(CurrentId, CurrentName, CurrentPath);
		}
		else if (CurrentTag == TEXT("StateGroup"))
		{
			visitor->EnterStateGroup(CurrentId, CurrentName, CurrentPath);
			recurse(CurrentNode, WorkUnitPath, CurrentPath, ItemType, CurrentId);
			visitor->ExitStateGroup();
		}
		else if (CurrentTag == TEXT("State"))
		{
			visitor->EnterState(CurrentId, CurrentName, CurrentPath);
		}
		else if (CurrentTag == TEXT("GameParameter"))
		{
			visitor->EnterGameParameter(CurrentId, CurrentName, CurrentPath);
		}
		else if (CurrentTag == TEXT("Trigger"))
		{
			visitor->EnterTrigger(CurrentId, CurrentName, CurrentPath);
		}
		else if (CurrentTag == TEXT("Folder") || CurrentTag == TEXT("Bus"))
		{
			EWwiseItemType::Type currentItemType = EWwiseItemType::Folder;
			if (CurrentTag == TEXT("Bus"))
			{
				currentItemType = EWwiseItemType::Bus;
			}

			visitor->EnterFolderOrBus(CurrentId, CurrentName, CurrentPath, currentItemType);
			recurse(CurrentNode, WorkUnitPath, CurrentPath, ItemType, CurrentId);
			visitor->ExitFolderOrBus();
		}
	}

	visitor->ExitChildrenList();
}

#undef LOCTEXT_NAMESPACE