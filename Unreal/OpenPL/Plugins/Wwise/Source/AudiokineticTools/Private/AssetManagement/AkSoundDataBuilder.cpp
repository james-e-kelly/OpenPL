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

#include "AkSoundDataBuilder.h"

#include "AkAssetDatabase.h"
#include "AkAudioBank.h"
#include "AkAudioBankGenerationHelpers.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkAudioStyle.h"
#include "AkAuxBus.h"
#include "AkGroupValue.h"
#include "AkInitBank.h"
#include "AkMediaAsset.h"
#include "AkRtpc.h"
#include "AkSettings.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "AkUnrealHelper.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "AssetTools/Public/AssetToolsModule.h"
#include "Async/Async.h"
#include "Async/ParallelFor.h"
#include "Core/Public/Hash/CityHash.h"
#include "Editor.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "StaticPluginWriter.h"
#include "ToolBehavior/AkToolBehavior.h"
#include "UnrealEd/Public/FileHelpers.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "UnrealEd/Public/ObjectTools.h"

#define LOCTEXT_NAMESPACE "AkAudio"

DECLARE_CYCLE_STAT(TEXT("AkSoundData - Gather Assets"), STAT_GatherAssets, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Gather Media Assets"), STAT_GatherMediaAssets, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Cook Media Asset"), STAT_CookMediaAsset, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Fetch Event Platform Data For Event Group"), STAT_EventPlatformDataEventGroup, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Parse Event Info For Event Group"), STAT_ParseEventInfoEventGroup, STATGROUP_AkSoundDataSource);
DECLARE_CYCLE_STAT(TEXT("AkSoundData - Auto Save assets"), STAT_AutoSaveAssets, STATGROUP_AkSoundDataSource);

namespace AkSoundDataBuilder_Helpers
{
	constexpr auto SoundBankNamePrefix = TEXT("SB_");
}

AutoSetIsBuilding::AutoSetIsBuilding()
{
	if (FAkAudioDevice::Get())
	{
		FAkAudioDevice::Get()->IsBuildingData = true;
	}
}

AutoSetIsBuilding::~AutoSetIsBuilding()
{
	if (FAkAudioDevice::Get())
	{
		FAkAudioDevice::Get()->IsBuildingData = false;
	}
}

AkSoundDataBuilder::AkSoundDataBuilder(const InitParameters& InitParameter)
: initParameters(InitParameter)
{
	platformFile = &FPlatformFileManager::Get().GetPlatformFile();
	TempMediaDependenciesList.Empty();
}

AkSoundDataBuilder::~AkSoundDataBuilder()
{
	if (loadedAssetsHandle.IsValid())
	{
		loadedAssetsHandle->ReleaseHandle();
	}
}

void AkSoundDataBuilder::Init()
{
	assetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	assetToolsModule = &FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	basePackagePath = AkUnrealHelper::GetBaseAssetPackagePath();
	localizedPackagePath = AkUnrealHelper::GetLocalizedAssetPackagePath();

	if (auto akSettings = GetDefault<UAkSettings>())
	{
		splitSwitchContainerMedia = akSettings->SplitSwitchContainerMedia;
		splitMediaPerFolder = akSettings->SplitMediaPerFolder;
	}

	wwiseProjectInfo.Parse();

	cacheDirectory = wwiseProjectInfo.CacheDirectory();
	defaultLanguage = wwiseProjectInfo.DefaultLanguage();

	auto& akAssetDatabase = AkAssetDatabase::Get();
	if (!akAssetDatabase.IsInited())
	{
		akAssetDatabase.Init();
	}

	{
		FScopeLock autoLock(&akAssetDatabase.InitBankLock);
		if (akAssetDatabase.InitBank->DefaultLanguage != defaultLanguage)
		{
			akAssetDatabase.InitBank->DefaultLanguage = defaultLanguage;
			markAssetDirty(akAssetDatabase.InitBank);
		}
	}
}

void AkSoundDataBuilder::createNotificationItem()
{
	if (!IsRunningCommandlet())
	{
		AsyncTask(ENamedThreads::Type::GameThread, [sharedThis = SharedThis(this)]
		{
			FNotificationInfo Info(LOCTEXT("GeneratingSoundData", "Generating Wwise sound data..."));

			Info.Image = FAkAudioStyle::GetBrush(TEXT("AudiokineticTools.AkPickerTabIcon"));
			Info.bFireAndForget = false;
			Info.FadeOutDuration = 0.0f;
			Info.ExpireDuration = 0.0f;
#if UE_4_26_OR_LATER
			Info.Hyperlink = FSimpleDelegate::CreateLambda([]() { FGlobalTabmanager::Get()->TryInvokeTab(FName("OutputLog")); });
#else
			Info.Hyperlink = FSimpleDelegate::CreateLambda([]() { FGlobalTabmanager::Get()->InvokeTab(FName("OutputLog")); });
#endif
			Info.HyperlinkText = LOCTEXT("ShowOutputLogHyperlink", "Show Output Log");
			sharedThis->notificationItem = FSlateNotificationManager::Get().AddNotification(Info);
		});
	}
}

void AkSoundDataBuilder::notifyGenerationFailed()
{
	if (!IsRunningCommandlet())
	{
		AsyncTask(ENamedThreads::Type::GameThread, [sharedThis = SharedThis(this)]
		{
			if (sharedThis->notificationItem)
			{
				sharedThis->notificationItem->SetText(LOCTEXT("SoundDataGenerationFailed", "Generating Wwise sound data failed!"));

				sharedThis->notificationItem->SetCompletionState(SNotificationItem::CS_Fail);
				sharedThis->notificationItem->SetExpireDuration(3.5f);
				sharedThis->notificationItem->SetFadeOutDuration(0.5f);
				sharedThis->notificationItem->ExpireAndFadeout();
			}

			GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
		});
	}
}

void AkSoundDataBuilder::notifyProfilingInProgress()
{
	if (!IsRunningCommandlet())
	{
		AsyncTask(ENamedThreads::Type::GameThread, [sharedThis = SharedThis(this)]
		{
			if (sharedThis->notificationItem)
			{
				sharedThis->notificationItem->SetText(LOCTEXT("SoundDataGenerationProfiling", "Cannot generate sound data while Authoring is profiling."));

				sharedThis->notificationItem->SetCompletionState(SNotificationItem::CS_Fail);
				sharedThis->notificationItem->SetExpireDuration(3.5f);
				sharedThis->notificationItem->SetFadeOutDuration(0.5f);
				sharedThis->notificationItem->ExpireAndFadeout();
			}

			GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
		});
	}
}

void AkSoundDataBuilder::notifyCookingData()
{
	if (!IsRunningCommandlet())
	{
		AsyncTask(ENamedThreads::Type::GameThread, [sharedThis = SharedThis(this)]
		{
			if (sharedThis->notificationItem)
			{
				sharedThis->notificationItem->SetCompletionState(SNotificationItem::CS_Pending);
				sharedThis->notificationItem->SetText(LOCTEXT("CookingSoundData", "Cooking Wwise sound data into the assets..."));
			}
		});
	}
}

void AkSoundDataBuilder::notifyGenerationSucceeded()
{
	if (!IsRunningCommandlet())
	{
		AsyncTask(ENamedThreads::Type::GameThread, [sharedThis = SharedThis(this)]
		{
			if (sharedThis->notificationItem)
			{
				sharedThis->notificationItem->SetText(LOCTEXT("CookingSoundDataCompleted", "Cooking Wwise sound data completed!"));

				sharedThis->notificationItem->SetCompletionState(SNotificationItem::CS_Success);
				sharedThis->notificationItem->SetExpireDuration(3.5f);
				sharedThis->notificationItem->SetFadeOutDuration(0.5f);
				sharedThis->notificationItem->ExpireAndFadeout();
			}

			GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
		});
	}
}

bool AkSoundDataBuilder::processMediaEntry(MediaToCookMap& mediaToCookMap, const FString& platform, TArray<TSoftObjectPtr<UAkMediaAsset>>& medias, const TSharedPtr<FJsonObject>& mediaData, bool isStreamed, bool isInitBank)
{
	FString stringId = mediaData->GetStringField("Id");

	uint32 id = FCString::Atoi64(*stringId);

	FString language = mediaData->GetStringField("Language");

	FString cachePath = mediaData->GetStringField("Path");

	FString mediaName = FPaths::GetBaseFilename(mediaData->GetStringField("ShortName"));

	FSoftObjectPath assetPath;

	bool useDeviceMemory = false;
	FString stringUseDeviceMemory;
	if (mediaData->TryGetStringField(TEXT("UseDeviceMemory"), stringUseDeviceMemory))
	{
		useDeviceMemory = (stringUseDeviceMemory == TEXT("true"));
	}

	bool usingReferenceLanguageAsStandIn = false;
	if (mediaData->TryGetBoolField(TEXT("UsingReferenceLanguageAsStandIn"), usingReferenceLanguageAsStandIn) && usingReferenceLanguageAsStandIn)
	{
		language = defaultLanguage;
	}

	bool changed = false;

	FString subFolder;
	if (splitMediaPerFolder)
	{
		subFolder = FString::Printf(TEXT("%02x/%02x"), (id >> 24) & 0xFF, (id >> 16) & 0xFF);
	}

	if (language != TEXT("SFX"))
	{
		assetPath = ObjectTools::SanitizeObjectPath(FPaths::Combine(localizedPackagePath, language, AkUnrealHelper::MediaFolderName, subFolder, stringId + TEXT(".") + stringId));
	}
	else
	{
		language.Empty();

		assetPath = ObjectTools::SanitizeObjectPath(FPaths::Combine(basePackagePath, AkUnrealHelper::MediaFolderName, subFolder, stringId + TEXT(".") + stringId));
	}

	if (isInitBank)
	{
		FScopeLock autoLock(&mediaLock);
		if (mediaIdToAssetPath.Contains(id))
		{
			return false;
		}
	}

	if (!medias.ContainsByPredicate([&assetPath](const TSoftObjectPtr<UAkMediaAsset>& item) { return item.GetUniqueID() == assetPath; }))
	{
		medias.Emplace(assetPath);

		changed = true;

		{
			FScopeLock autoLock(&mediaLock);

			if (!mediaIdToAssetPath.Contains(id))
			{
				mediaIdToAssetPath.Add(id, assetPath);
			}
		}
	}

	{
		FScopeLock autoLock(&mediaLock);

		auto* mediaAssetToCookIt = mediaToCookMap.Find(id);

		if (mediaAssetToCookIt)
		{
			MediaAssetPerPlatformData& parsedPlatformData = mediaAssetToCookIt->Get()->ParsedPerPlatformData.FindOrAdd(platform);

			FString stringPrefetchSize;
			if (mediaData->TryGetStringField("PrefetchSize", stringPrefetchSize))
			{
				uint32 parsedPrefetchSize = static_cast<uint32>(FCString::Atoi(*stringPrefetchSize));
				
				parsedPlatformData.PrefetchSize = FMath::Max(parsedPlatformData.PrefetchSize, parsedPrefetchSize);
			}

			parsedPlatformData.UseDeviceMemory = useDeviceMemory;
			parsedPlatformData.IsStreamed = isStreamed ? true : parsedPlatformData.IsStreamed;
		}
		else
		{
			auto& newMediaToCookEntry = mediaToCookMap.Emplace(id);
			newMediaToCookEntry = MakeShared<MediaAssetToCook>();
			newMediaToCookEntry->Id = id;
			newMediaToCookEntry->AssetPath = assetPath;
			newMediaToCookEntry->Language = language;
			newMediaToCookEntry->CachePath = cachePath;
			newMediaToCookEntry->MediaName = mediaName;

			MediaAssetPerPlatformData& parsedPlatformData = newMediaToCookEntry->ParsedPerPlatformData.FindOrAdd(platform);
			parsedPlatformData.IsStreamed = isStreamed;
			parsedPlatformData.PrefetchSize = 0;
			parsedPlatformData.UseDeviceMemory = useDeviceMemory;
		}
	}

	return changed;
}

void AkSoundDataBuilder::parseGameSyncs(const TSharedPtr<FJsonObject>& soundBankJson)
{
	if (!AkUnrealHelper::IsUsingEventBased())
	{
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* stateGroups = nullptr;
	if (soundBankJson->TryGetArrayField("StateGroups", stateGroups))
	{
		for (auto& stateGroupValueJson : *stateGroups)
		{
			auto& stateGroupJson = stateGroupValueJson->AsObject();

			FString stringGroupId = stateGroupJson->GetStringField("Id");
			uint32 groupId = static_cast<uint32>(FCString::Atoi64(*stringGroupId));

			auto& statesJson = stateGroupJson->GetArrayField("States");

			for (auto& stateValueJson : statesJson)
			{
				parseGroupValue(stateValueJson->AsObject(), groupId);
			}
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* switchGroups = nullptr;
	if (soundBankJson->TryGetArrayField("SwitchGroups", switchGroups))
	{
		for (auto& switchGroupValueJson : *switchGroups)
		{
			auto& switchGroupJson = switchGroupValueJson->AsObject();

			FString stringGroupId = switchGroupJson->GetStringField("Id");
			uint32 groupId = static_cast<uint32>(FCString::Atoi64(*stringGroupId));

			auto& switchesJson = switchGroupJson->GetArrayField("Switches");

			for (auto& switchValueJson : switchesJson)
			{
				parseGroupValue(switchValueJson->AsObject(), groupId);
			}
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* gameParameters = nullptr;
	if (soundBankJson->TryGetArrayField("GameParameters", gameParameters))
	{
		for (auto& gameParameterJsonValue : *gameParameters)
		{
			parseGameParameter(gameParameterJsonValue->AsObject());
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* triggers = nullptr;
	if (soundBankJson->TryGetArrayField("IncludedTriggers", triggers))
	{
		for (auto& triggerJsonValue : *triggers)
		{
			parseTrigger(triggerJsonValue->AsObject());
		}
	}
}

bool AkSoundDataBuilder::parseBankHash(UAkAssetData* bankData, const TSharedPtr<FJsonObject>& soundBankJson)
{
	bool changed = false;

	FString stringHash;
	if (soundBankJson->TryGetStringField(TEXT("Hash"), stringHash))
	{
		auto newBankHash = static_cast<uint32>(FCString::Strtoui64(*stringHash, nullptr, 10));
		if (bankData->CachedHash != newBankHash)
		{
			bankData->CachedHash = newBankHash;
			changed = true;
		}
	}

	return changed;
}

void AkSoundDataBuilder::parseGroupValue(const TSharedPtr<FJsonObject>& groupValueObject, uint32 groupId)
{
	FString stringGuid = groupValueObject->GetStringField("GUID");

	FGuid guid;
	FGuid::ParseExact(stringGuid, EGuidFormats::DigitsWithHyphensInBraces, guid);

	if (auto groupValue = AkAssetDatabase::Get().GroupValueMap.FindLiveAsset(guid))
	{
		FString stringId = groupValueObject->GetStringField("Id");
		uint32 valueId = static_cast<uint32>(FCString::Atoi64(*stringId));

		bool changed = false;

		if (groupValue->GroupShortID != groupId)
		{
			groupValue->GroupShortID = groupId;
			changed = true;
		}

		if (groupValue->ShortID != valueId)
		{
			groupValue->ShortID = valueId;
			changed = true;
		}

		if (changed)
		{
			markAssetDirty(groupValue);
		}
	}
}

void AkSoundDataBuilder::parseGameParameter(const TSharedPtr<FJsonObject>& gameParameterObject)
{
	FString stringGuid = gameParameterObject->GetStringField("GUID");

	FGuid guid;
	FGuid::ParseExact(stringGuid, EGuidFormats::DigitsWithHyphensInBraces, guid);

	if (auto rtpc = AkAssetDatabase::Get().RtpcMap.FindLiveAsset(guid))
	{
		FString stringId = gameParameterObject->GetStringField("Id");
		uint32 valueId = static_cast<uint32>(FCString::Atoi64(*stringId));

		bool changed = false;

		if (rtpc->ShortID != valueId)
		{
			rtpc->ShortID = valueId;
			changed = true;
		}

		if (changed)
		{
			markAssetDirty(rtpc);
		}
	}
}

void AkSoundDataBuilder::parseTrigger(const TSharedPtr<FJsonObject>& triggerObject)
{
	FString stringGuid = triggerObject->GetStringField("GUID");

	FGuid guid;
	FGuid::ParseExact(stringGuid, EGuidFormats::DigitsWithHyphensInBraces, guid);

	if (auto trigger = AkAssetDatabase::Get().TriggerMap.FindLiveAsset(guid))
	{
		FString stringId = triggerObject->GetStringField("Id");
		uint32 valueId = static_cast<uint32>(FCString::Atoi64(*stringId));

		bool changed = false;

		if (trigger->ShortID != valueId)
		{
			trigger->ShortID = valueId;
			changed = true;
		}

		if (changed)
		{
			markAssetDirty(trigger);
		}
	}
}

bool AkSoundDataBuilder::parseMedia(const TSharedPtr<FJsonObject>& soundBankJson, MediaToCookMap& mediaToCookMap, TArray<TSoftObjectPtr<UAkMediaAsset>>& mediaList, const FString& platform, bool isInitBank)
{
	if (!AkUnrealHelper::IsUsingEventBased())
	{
		return false;
	}

	bool changed = false;

	const TArray<TSharedPtr<FJsonValue>>* streamedFiles = nullptr;
	if (soundBankJson->TryGetArrayField("ReferencedStreamedFiles", streamedFiles))
	{
		for (auto& streamJsonValue : *streamedFiles)
		{
			changed |= processMediaEntry(mediaToCookMap, platform, mediaList, streamJsonValue->AsObject(), true, isInitBank);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* excludedMemoryFiles = nullptr;
	if (soundBankJson->TryGetArrayField("ExcludedMemoryFiles", excludedMemoryFiles))
	{
		for (auto& mediaJsonValue : *excludedMemoryFiles)
		{
			changed |= processMediaEntry(mediaToCookMap, platform, mediaList, mediaJsonValue->AsObject(), false, isInitBank);
		}
	}

	return changed;
}

bool AkSoundDataBuilder::MediaListsAreDifferent(const TArray<TSoftObjectPtr<UAkMediaAsset>>& OldList, const TArray<TSoftObjectPtr<UAkMediaAsset>>& NewList)
{
	if (OldList.Num() != NewList.Num())
	{
		return true;
	}

	for (int j = 0; j < OldList.Num(); j++)
	{
		TSoftObjectPtr<UAkMediaAsset> OldMediaItem = OldList[j];
		const TSoftObjectPtr<UAkMediaAsset>* NewMediaItem = NewList.FindByPredicate([OldMediaItem](const TSoftObjectPtr<UAkMediaAsset> ItemInArray) { return ItemInArray.GetUniqueID() == OldMediaItem.GetUniqueID(); });
		if (!NewMediaItem)
		{
			return true;
		}
	}

	return false;
}

bool AkSoundDataBuilder::DidSwitchContainerDataChange(const TArray<UAkAssetDataSwitchContainerData*>& OldData, const TArray<UAkAssetDataSwitchContainerData*>& NewData)
{
	if (OldData.Num() != NewData.Num())
	{
		return true;
	}

	for (int32 i = 0; i < OldData.Num(); i++)
	{
		// Find the appropriate Switch Container Data in the new array
		UAkAssetDataSwitchContainerData* OldItem = OldData[i];
		if (!OldItem)
		{
			return true;
		}

		UAkAssetDataSwitchContainerData* const* NewItem = NewData.FindByPredicate([OldItem](const UAkAssetDataSwitchContainerData* ItemInArray) { return ItemInArray->GroupValue.GetUniqueID() == OldItem->GroupValue.GetUniqueID(); });
		if (!NewItem || !(*NewItem))
		{
			return true;
		}

		if (!OldItem->DefaultGroupValue && (*NewItem)->DefaultGroupValue)
		{
			return true;
		}

		if (OldItem->DefaultGroupValue && (*NewItem)->DefaultGroupValue && OldItem->DefaultGroupValue->ID != (*NewItem)->DefaultGroupValue->ID)
		{
			return true;
		}

		if (MediaListsAreDifferent(OldItem->TempMediaList, (*NewItem)->TempMediaList))
		{
			return true;
		}

		if (DidSwitchContainerDataChange(OldItem->Children, (*NewItem)->Children))
		{
			return true;
		}
	}

	return false;
}

bool AkSoundDataBuilder::ParseMediaAndSwitchContainers(TSharedPtr<FJsonObject> EventJson, UAkAssetDataSwitchContainer* AssetData, const FString& Platform, MediaToCookMap& MediaToCookMap)
{
	bool changed = false;
	TArray<TSoftObjectPtr<UAkMediaAsset>> NewMediaList;

	parseMedia(EventJson, MediaToCookMap, NewMediaList, Platform, false);

	if (splitSwitchContainerMedia)
	{
		TArray<UAkAssetDataSwitchContainerData*> NewSwitchContainerData;
		const TArray<TSharedPtr<FJsonValue>>* switchContainers = nullptr;
		if (EventJson->TryGetArrayField("SwitchContainers", switchContainers))
		{
			for (auto& switchContainerValueJson : *switchContainers)
			{
				auto& switchContainerJson = switchContainerValueJson->AsObject();

				UAkAssetDataSwitchContainerData* switchContainerEntry = NewObject<UAkAssetDataSwitchContainerData>(AssetData);
				parseSwitchContainer(switchContainerJson, switchContainerEntry, NewMediaList, AssetData, MediaToCookMap);
				NewSwitchContainerData.Add(switchContainerEntry);
			}
		}

		if (DidSwitchContainerDataChange(AssetData->SwitchContainers, NewSwitchContainerData))
		{
			AssetData->SwitchContainers.Empty();
			AssetData->SwitchContainers.Append(NewSwitchContainerData);
			changed = true;
		}
	}

	UAkGroupValue* newDefaultSwitchValue = nullptr;

	FString stringDefaultSwitchValue;
	if (EventJson->TryGetStringField("DefaultSwitchValue", stringDefaultSwitchValue))
	{
		FGuid defaultSwitchValueID;
		FGuid::ParseExact(stringDefaultSwitchValue, EGuidFormats::DigitsWithHyphensInBraces, defaultSwitchValueID);

		if (auto foundDefaultSwitchValue = AkAssetDatabase::Get().GroupValueMap.FindLiveAsset(defaultSwitchValueID))
		{
			newDefaultSwitchValue = foundDefaultSwitchValue;
		}
	}

	if ((!AssetData->DefaultGroupValue && newDefaultSwitchValue)
		|| (AssetData->DefaultGroupValue && newDefaultSwitchValue && AssetData->DefaultGroupValue->ID != newDefaultSwitchValue->ID)
		)
	{
		AssetData->DefaultGroupValue = newDefaultSwitchValue;
		changed = true;
	}

	if (MediaListsAreDifferent(AssetData->TempMediaList, NewMediaList))
	{
		AssetData->TempMediaList.Empty();
		AssetData->TempMediaList.Append(NewMediaList);
		changed = true;
	}

	return changed;
}

template<typename AssetType>
bool AkSoundDataBuilder::ParseEventMetadataSection(TSharedPtr<FJsonObject> eventJson, const FString& SectionName, const AkAssetDatabase::AkTypeMap<AssetType>* AssetMap, TSet<AssetType*>* DestinationSet)
{
	bool changed = false;

	const TArray<TSharedPtr<FJsonValue>>* Actions = nullptr;
	if (eventJson->TryGetArrayField(SectionName, Actions))
	{
		for (auto& ValueJson : *Actions)
		{
			auto& JsonObject = ValueJson->AsObject();
			FGuid AssetGuid;
			FString AssetGuidString = JsonObject->GetStringField("GUID");
			FGuid::ParseExact(AssetGuidString, EGuidFormats::DigitsWithHyphensInBraces, AssetGuid);

			if (auto asset = AssetMap->FindLiveAsset(AssetGuid))
			{
				bool WasAlreadyThere = false;
				DestinationSet->Add(asset, &WasAlreadyThere);
				changed |= !WasAlreadyThere;
			}
		}
	}

	return changed;
}

bool AkSoundDataBuilder::parseAssetInfo(UAkAudioEvent* akEvent, UAkAssetData* platformData, const FString& platform, const FString& language, const TSharedPtr<FJsonObject>& soundBankData, MediaToCookMap& mediaToCookMap)
{
	bool changed = false;

	if (auto* eventPlatformData = Cast<UAkAudioEventData>(platformData))
	{
		const TArray<TSharedPtr<FJsonValue>>* eventsArray = nullptr;
		if (soundBankData->TryGetArrayField("IncludedEvents", eventsArray))
		{
			for(auto& eventJsonValue : *eventsArray)
			{
				auto& eventJson = eventJsonValue->AsObject();
				FString eventStringId = eventJson->GetStringField("GUID");

				FGuid eventId;
				FGuid::ParseExact(eventStringId, EGuidFormats::DigitsWithHyphensInBraces, eventId);

				if (eventId == akEvent->ID)
				{
					changed |= parseEventInfo(akEvent, eventPlatformData, eventJson);
					changed |= ParseMediaAndSwitchContainers(eventJson, eventPlatformData, platform, mediaToCookMap);
					changed |= ParseEventMetadataSection(eventJson, FString(TEXT("ActionPostEvent")),	&AkAssetDatabase::Get().EventMap,		&eventPlatformData->PostedEvents);
					changed |= ParseEventMetadataSection(eventJson, FString(TEXT("AuxBusSends")),		&AkAssetDatabase::Get().AuxBusMap,		&eventPlatformData->UserDefinedSends);
					changed |= ParseEventMetadataSection(eventJson, FString(TEXT("ActionTrigger")),		&AkAssetDatabase::Get().TriggerMap,		&eventPlatformData->PostedTriggers);
					changed |= ParseEventMetadataSection(eventJson, FString(TEXT("ActionSetSwitch")),	&AkAssetDatabase::Get().GroupValueMap,	&eventPlatformData->GroupValues);
					changed |= ParseEventMetadataSection(eventJson, FString(TEXT("ActionSetState")),	&AkAssetDatabase::Get().GroupValueMap,	&eventPlatformData->GroupValues);
					break;
				}
			}
		}
	}

	return changed;
}

bool AkSoundDataBuilder::parseEventInfo(UAkAudioEvent* akEvent, UAkAudioEventData* eventPlatformData, const TSharedPtr<FJsonObject>& eventJson)
{
	bool changed = false;

	FString stringId;
	if (eventJson->TryGetStringField("Id", stringId))
	{
		uint32 id = static_cast<uint32>(FCString::Atoi64(*stringId));
		if (akEvent->ShortID != id)
		{
			akEvent->ShortID = id;
			changed = true;
		}
	}

	FString ValueString;
	if (eventJson->TryGetStringField("MaxAttenuation", ValueString))
	{
		const float EventRadius = FCString::Atof(*ValueString);
		if (eventPlatformData->MaxAttenuationRadius != EventRadius)
		{
			eventPlatformData->MaxAttenuationRadius = EventRadius;
			changed = true;
		}
	}
	else
	{
		if (eventPlatformData->MaxAttenuationRadius != 0)
		{
			// No attenuation info in json file, set to 0.
			eventPlatformData->MaxAttenuationRadius = 0;
			changed = true;
		}
	}

	// if we can't find "DurationType", then we assume infinite
	const bool IsInfinite = !eventJson->TryGetStringField("DurationType", ValueString) || (ValueString == "Infinite") || (ValueString == "Unknown");
	if (eventPlatformData->IsInfinite != IsInfinite)
	{
		eventPlatformData->IsInfinite = IsInfinite;
		changed = true;
	}

	if (!IsInfinite)
	{
		if (eventJson->TryGetStringField("DurationMin", ValueString))
		{
			const float DurationMin = FCString::Atof(*ValueString);
			if (eventPlatformData->MinimumDuration != DurationMin)
			{
				eventPlatformData->MinimumDuration = DurationMin;
				changed = true;
			}
		}

		if (eventJson->TryGetStringField("DurationMax", ValueString))
		{
			const float DurationMax = FCString::Atof(*ValueString);
			if (eventPlatformData->MaximumDuration != DurationMax)
			{
				eventPlatformData->MaximumDuration = DurationMax;
				changed = true;
			}
		}
	}

	return changed;
}

void AkSoundDataBuilder::parseSwitchContainer(const TSharedPtr<FJsonObject>& SwitchContainerJson, UAkAssetDataSwitchContainerData* SwitchContainerEntry, TArray<TSoftObjectPtr<UAkMediaAsset>>& MediaList, UObject* Parent, MediaToCookMap& MediaToCookMap)
{
	FString stringSwitchValue = SwitchContainerJson->GetStringField("SwitchValue");
	FGuid switchValueGuid;
	FGuid::ParseExact(stringSwitchValue, EGuidFormats::DigitsWithHyphensInBraces, switchValueGuid);

	auto groupValue = AkAssetDatabase::Get().GroupValueMap.FindLiveAsset(switchValueGuid);

	if (groupValue)
	{
		SwitchContainerEntry->GroupValue = groupValue;
	}

	FString stringDefaultSwitchValue;
	if (SwitchContainerJson->TryGetStringField("DefaultSwitchValue", stringDefaultSwitchValue))
	{
		FGuid defaultSwitchValue;
		FGuid::ParseExact(stringDefaultSwitchValue, EGuidFormats::DigitsWithHyphensInBraces, defaultSwitchValue);

		if (auto defaultGroupValue = AkAssetDatabase::Get().GroupValueMap.FindLiveAsset(defaultSwitchValue))
		{
			SwitchContainerEntry->DefaultGroupValue = defaultGroupValue;
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* jsonMediaList = nullptr;
	if (SwitchContainerJson->TryGetArrayField("Media", jsonMediaList))
	{
		MediaDependencySet MediaDependenciesInJson;
		for (auto& mediaJsonValue : *jsonMediaList)
		{
			auto& mediaJsonObject = mediaJsonValue->AsObject();

			FString stringId = mediaJsonObject->GetStringField("Id");
			uint32 mediaFileId = static_cast<uint32>(FCString::Atoi64(*stringId));

			FSoftObjectPath* mediaAssetPath = nullptr;

			{
				FScopeLock autoLock(&mediaLock);
				mediaAssetPath = mediaIdToAssetPath.Find(mediaFileId);

				TSharedPtr<MediaAssetToCook>* assetToCook = MediaToCookMap.Find(mediaFileId);
				if (assetToCook)
				{
					assetToCook->Get()->AutoLoad = false;
				}
			}

			if (mediaAssetPath)
			{
				SwitchContainerEntry->TempMediaList.Emplace(*mediaAssetPath);

				MediaList.RemoveAll([mediaAssetPath](const TSoftObjectPtr<UAkMediaAsset>& item) {
					return item.GetUniqueID() == *mediaAssetPath;
				});

				MediaDependenciesInJson.Add(TSoftObjectPtr<UAkMediaAsset>(*mediaAssetPath));
			}
		}

		if (groupValue)
		{
			TempMediaDependenciesList.FindOrAdd(groupValue).Append(MediaDependenciesInJson);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* children = nullptr;
	if (SwitchContainerJson->TryGetArrayField("Children", children))
	{
		for (auto& childJsonValue : *children)
		{
			auto& childJsonObject = childJsonValue->AsObject();

			UAkAssetDataSwitchContainerData* childEntry = NewObject<UAkAssetDataSwitchContainerData>(Parent);
			parseSwitchContainer(childJsonObject, childEntry, MediaList, Parent, MediaToCookMap);
			SwitchContainerEntry->Children.Add(childEntry);
		}
	}
}

bool AkSoundDataBuilder::parseAssetInfo(UAkAuxBus* akAuxBus, UAkAssetData* auxBusPlatformData, const FString& platform, const FString& language, const TSharedPtr<FJsonObject>& soundBankData, MediaToCookMap& mediaToCookMap)
{
	bool changed = false;

	const TArray<TSharedPtr<FJsonValue>>* auxBusses = nullptr;
	if (soundBankData->TryGetArrayField("IncludedAuxBusses", auxBusses))
	{
		for (auto& auxBusJsonValue : *auxBusses)
		{
			auto& auxBusJson = auxBusJsonValue->AsObject();
			FString auxBusStringId = auxBusJson->GetStringField("GUID");

			FGuid auxBusId;
			FGuid::ParseExact(auxBusStringId, EGuidFormats::DigitsWithHyphensInBraces, auxBusId);

			if (auxBusId == akAuxBus->ID)
			{
				FString stringId;
				if (auxBusJson->TryGetStringField("Id", stringId))
				{
					uint32 id = static_cast<uint32>(FCString::Atoi64(*stringId));
					if (akAuxBus->ShortID != id)
					{
						akAuxBus->ShortID = id;
						changed = true;
					}
				}

				break;
			}
		}
	}

	return changed;
}

bool AkSoundDataBuilder::GetAuxBusFromBankInfo(const TSharedPtr<FJsonObject>& soundBankData)
{
	auto& akAssetDatabase = AkAssetDatabase::Get();
	bool bChanged = false;
	const TArray<TSharedPtr<FJsonValue>>* auxBusses = nullptr;
	if (soundBankData->TryGetArrayField("IncludedAuxBusses", auxBusses))
	{
		for (auto& auxBusJsonValue : *auxBusses)
		{
			auto& auxBusJson = auxBusJsonValue->AsObject();
			FString auxBusStringId;
			FGuid auxBusId;

			if (auxBusJson->TryGetStringField("GUID", auxBusStringId) && !auxBusStringId.IsEmpty())
			{
				FGuid::ParseExact(auxBusStringId, EGuidFormats::DigitsWithHyphensInBraces, auxBusId);
			}

			FAssetData auxBusByName;
			const FAssetData* akAuxBusIt = nullptr;

			akAuxBusIt = akAssetDatabase.AuxBusMap.Find(auxBusId);
			if (!akAuxBusIt)
			{
				auto foundAuxBusses = akAssetDatabase.AuxBusMap.FindByName(auxBusJson->GetStringField("Name"));
				if (foundAuxBusses.Num() > 0)
				{
					auxBusByName = foundAuxBusses[0];
					akAuxBusIt = &auxBusByName;
				}
			}

			if (akAuxBusIt)
			{
				if (auto akAuxBus = Cast<UAkAuxBus>(akAuxBusIt->GetAsset()))
				{
					FString stringId;
					if (auxBusJson->TryGetStringField("Id", stringId))
					{
						uint32 id = static_cast<uint32>(FCString::Atoi64(*stringId));
						if (akAuxBus->ShortID != id)
						{
							akAuxBus->ShortID = id;
							markAssetDirty(akAuxBus);
							bChanged = true;
						}
					}
				}
			}
		}
	}

	return bChanged;

}

bool AkSoundDataBuilder::parseAssetInfo(UAkInitBank* akInitBank, UAkAssetData* platformData, const FString& platform, const FString& language, const TSharedPtr<FJsonObject>& soundBankData, MediaToCookMap& mediaToCookMap)
{
	akInitBank->DefaultLanguage = defaultLanguage;
	return GetAuxBusFromBankInfo(soundBankData);
}

bool AkSoundDataBuilder::parseAssetInfo(UAkAudioBank* akAudioBank, UAkAssetData* platformData, const FString& platform, const FString& language, const TSharedPtr<FJsonObject>& soundBankData, MediaToCookMap& mediaToCookMap)
{
	auto& akAssetDatabase = AkAssetDatabase::Get();

	const TArray<TSharedPtr<FJsonValue>>* eventsArray = nullptr;
	if (soundBankData->TryGetArrayField("IncludedEvents", eventsArray))
	{
		bool isUsingNewAssetManagement = AkUnrealHelper::IsUsingEventBased();

		for (auto& eventJsonValue : *eventsArray)
		{
			auto& eventJson = eventJsonValue->AsObject();

			FString eventStringId;
			FGuid eventId;
			if (eventJson->TryGetStringField("GUID", eventStringId) && !eventStringId.IsEmpty())
			{ 
				FGuid::ParseExact(eventStringId, EGuidFormats::DigitsWithHyphensInBraces, eventId);
			}

			FAssetData eventByName;
			FAssetData const* eventIt = nullptr;

			eventIt = akAssetDatabase.EventMap.Find(eventId);
			if (!eventIt)
			{
				auto foundEvents = akAssetDatabase.EventMap.FindByName(eventJson->GetStringField("Name"));
				if (foundEvents.Num() > 0)
				{
					eventByName = foundEvents[0];
					eventIt = &eventByName;
				}
			}

			if (eventIt)
			{
				auto sharedThis = SharedThis(this);

				struct EventRequiredData
				{
					UAkAudioEvent* akAudioEvent = nullptr;
					UAkAssetData* eventPlatformData = nullptr;
				};

				TSharedPtr<EventRequiredData> requiredData = MakeShared<EventRequiredData>();
				requiredData->akAudioEvent = Cast<UAkAudioEvent>(eventIt->GetAsset());

				auto fetchPlatformDataTask = FFunctionGraphTask::CreateAndDispatchWhenReady([requiredData, platform]()
				{
					if (requiredData->akAudioEvent)
					{
						requiredData->eventPlatformData = requiredData->akAudioEvent->FindOrAddAssetData(platform, FString());
					}
				}, GET_STATID(STAT_EventPlatformDataEventGroup), nullptr, ENamedThreads::GameThread);
				{
					FScopeLock autoLock(&parseTasksLock);
					allParseTask.Add(fetchPlatformDataTask);
				}

				auto parseMediaAndSwitchContainersTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, requiredData, eventJson, platform, language, isUsingNewAssetManagement]
				{
					if (!requiredData->akAudioEvent || !requiredData->eventPlatformData)
						return;

					bool changed = false;

					MediaToCookMap& mediaToCookMap = sharedThis->getMediaToCookMap(platform);
					changed |= sharedThis->parseEventInfo(requiredData->akAudioEvent, Cast<UAkAudioEventData>(requiredData->eventPlatformData), eventJson);

					if (isUsingNewAssetManagement)
					{
						if (auto* eventPlatformData = Cast<UAkAudioEventData>(requiredData->eventPlatformData))
						{
							if (language.Len() > 0)
							{
								auto localizedMediaInfo = eventPlatformData->FindOrAddLocalizedData(language);
								changed |= sharedThis->ParseMediaAndSwitchContainers(eventJson, localizedMediaInfo, platform, mediaToCookMap);
							}
							else
							{
								changed |= sharedThis->ParseMediaAndSwitchContainers(eventJson, eventPlatformData, platform, mediaToCookMap);
							}
						}
					}

					if (changed)
					{
						sharedThis->markAssetDirty(requiredData->akAudioEvent);
						sharedThis->markAssetDirty(requiredData->eventPlatformData);
					}
				}, GET_STATID(STAT_ParseEventInfoEventGroup), fetchPlatformDataTask);
				{
					FScopeLock autoLock(&parseTasksLock);
					allParseTask.Add(parseMediaAndSwitchContainersTask);
				}
			}
		}
	}

	GetAuxBusFromBankInfo(soundBankData);

	return false;
}

bool AkSoundDataBuilder::parsePluginInfo(UAkInitBank* InitBank, const FString& Platform, const TSharedPtr<FJsonObject>& PluginInfoJson)
{
	auto& pluginsArrayJson = PluginInfoJson->GetArrayField(TEXT("Plugins"));

	auto platformData = InitBank->FindOrAddAssetDataTyped<UAkInitBankAssetData>(Platform, FString());
	if (!platformData)
	{
		return false;
	}

	TArray<FAkPluginInfo> NewPluginInfo;
	for (auto& pluginJsonValue : pluginsArrayJson)
	{
		auto& pluginJson = pluginJsonValue->AsObject();

		FString pluginDLL;
		bool HasDLL = pluginJson->TryGetStringField(TEXT("DLL"), pluginDLL);
		if (!HasDLL || pluginDLL == TEXT("DefaultSink") || pluginDLL == TEXT("AkSoundEngineDLL"))
		{
			continue;
		}

		auto pluginName = pluginJson->GetStringField(TEXT("Name"));
		auto pluginStringID = pluginJson->GetStringField(TEXT("ID"));

		auto pluginID = static_cast<uint32>(FCString::Atoi64(*pluginStringID));

		NewPluginInfo.Emplace(pluginName, pluginID, pluginDLL);
	}

	bool changed = false;
	if (platformData->PluginInfos.Num() != NewPluginInfo.Num())
	{
		changed = true;
	}
	else
	{
		for (int i = 0; i < platformData->PluginInfos.Num(); i++)
		{
			FAkPluginInfo OldPluginInfo = platformData->PluginInfos[i];
			if (!NewPluginInfo.ContainsByPredicate([OldPluginInfo](const FAkPluginInfo& ItemInArray) { return ItemInArray.PluginID == OldPluginInfo.PluginID; }))
			{
				changed = true;
				break;
			}
		}
	}

	if (changed)
	{
		platformData->PluginInfos.Empty();
		platformData->PluginInfos.Append(NewPluginInfo);
	}

	if (changed && platformData->PluginInfos.Num() > 0)
	{
		StaticPluginWriter::OutputPluginInformation(InitBank, Platform);
	}

	return changed;
}

void AkSoundDataBuilder::cookMediaAsset(const MediaAssetToCook& mediaToCook, const FString& platform)
{
	if (!mediaToCook.Instance || !mediaToCook.CurrentPlatformData)
	{
		return;
	}

	bool changed = false;

	UAkMediaAsset* mediaAsset = mediaToCook.Instance;

	if (mediaAsset->Id != mediaToCook.Id)
	{
		mediaAsset->Id = mediaToCook.Id;
		changed = true;
	}

	if (mediaAsset->MediaName != mediaToCook.MediaName)
	{
		mediaAsset->MediaName = mediaToCook.MediaName;
		changed = true;
	}

	if (mediaAsset->AutoLoad != mediaToCook.AutoLoad)
	{
		mediaAsset->AutoLoad = mediaToCook.AutoLoad;
		changed = true;
	}

	auto mediaFilePath = FPaths::Combine(cacheDirectory, platform, mediaToCook.CachePath);
	auto modificationTime = platformFile->GetTimeStamp(*mediaFilePath).ToUnixTimestamp();
	auto platformData = mediaToCook.CurrentPlatformData;
	const MediaAssetPerPlatformData* parsedPlatformData = mediaToCook.ParsedPerPlatformData.Find(platform);
	if (!parsedPlatformData)
	{
		return;
	}

	bool isStreamedDifferent = platformData->IsStreamed != parsedPlatformData->IsStreamed;
	bool prefetchSizeDifferent = platformData->IsStreamed && platformData->DataChunks.Num() > 0 && platformData->DataChunks[0].IsPrefetch && platformData->DataChunks[0].Data.GetBulkDataSize() != parsedPlatformData->PrefetchSize;

	if (isStreamedDifferent)
	{
		platformData->IsStreamed = parsedPlatformData->IsStreamed;
	}

	if (platformData->UseDeviceMemory != parsedPlatformData->UseDeviceMemory)
	{
		platformData->UseDeviceMemory = parsedPlatformData->UseDeviceMemory;
	}

	bool BulkDataTypeIsOutOfDate = false;
	{
		FScopeLock autoLock(&platformData->DataLock);
		for (auto& DataChunk : platformData->DataChunks)
		{
			if ((DataChunk.Data.GetBulkDataFlags() & BULKDATA_ForceInlinePayload) != 0)
			{
				BulkDataTypeIsOutOfDate = true;
				break;
			}
		}
	}

	TUniquePtr<IFileHandle> fileReader(platformFile->OpenRead(*mediaFilePath));
	if (fileReader)
	{
		auto fileSize = fileReader->Size();

		TArray<uint8> tempData;
		tempData.Reserve(fileSize);

		fileReader->Read(tempData.GetData(), fileSize);

		auto newHash = CityHash64(reinterpret_cast<const char*>(tempData.GetData()), fileSize);

		if (platformData->MediaContentHash != newHash || isStreamedDifferent || prefetchSizeDifferent || BulkDataTypeIsOutOfDate)
		{
			{
				FScopeLock autoLock(&platformData->DataLock);
				platformData->DataChunks.Empty();
			}

			fileReader->Seek(0);

			if (parsedPlatformData->IsStreamed && parsedPlatformData->PrefetchSize > 0)
			{
				auto prefetchChunk = new FAkMediaDataChunk(fileReader.Get(), parsedPlatformData->PrefetchSize, BULKDATA_Force_NOT_InlinePayload, &mediaAsset->BulkDataWriteLock, true);
				{
					FScopeLock autoLock(&platformData->DataLock);
					platformData->DataChunks.Add(prefetchChunk);
				}
			}

			fileReader->Seek(0);
			auto dataChunk = new FAkMediaDataChunk(fileReader.Get(), fileSize, BULKDATA_Force_NOT_InlinePayload, &mediaAsset->BulkDataWriteLock);
			{
				FScopeLock autoLock(&platformData->DataLock);
				platformData->DataChunks.Add(dataChunk);
			}

			platformData->MediaContentHash = newHash;

			changed = true;
		}
	}

	if (changed)
	{
		markAssetDirty(mediaAsset);
	}
}

void AkSoundDataBuilder::dispatchAndWaitMediaCookTasks()
{
	for (auto& entry : mediaToCookPerPlatform)
	{
		dispatchMediaCookTask(entry.Value, entry.Key);
	}

	FTaskGraphInterface::Get().WaitUntilTasksComplete(allMediaTasks);

	FGraphEventArray mediaPathToMediaAssetTasks;

	for (auto& mediaPlatformData : allMediaData)
	{
		auto convertTask = FFunctionGraphTask::CreateAndDispatchWhenReady([mediaPlatformData] {
			mediaPlatformData->FillFinalMediaList();
		}, GET_STATID(STAT_GatherMediaAssets), nullptr, ENamedThreads::GameThread);

		mediaPathToMediaAssetTasks.Add(convertTask);
	}

	for (auto Pair : TempMediaDependenciesList)
	{
		UAkGroupValue* GroupValue = Pair.Key;
		MediaDependencySet& MediaDependenciesInJson = Pair.Value;
		MediaDependencySet MediaDependenciesInAsset(GroupValue->MediaDependencies);
		if (MediaDependenciesInJson.Num() != MediaDependenciesInAsset.Num() || !MediaDependenciesInJson.Includes(MediaDependenciesInAsset))
		{
			GroupValue->MediaDependencies.Reset();
			GroupValue->MediaDependencies.Append(MediaDependenciesInJson.Array());
			markAssetDirty(GroupValue);
		}

	}

	FTaskGraphInterface::Get().WaitUntilTasksComplete(mediaPathToMediaAssetTasks);
}

void AkSoundDataBuilder::dispatchMediaCookTask(const MediaToCookMap& mediaMap, const FString& platform)
{
	for (auto& entry : mediaMap)
	{
		auto& mediaToCook = entry.Value;

		auto sharedThis = SharedThis(this);

		auto fetchMediaTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, mediaToCook, platform] {
			UClass* mediaAssetClass = mediaToCook->Language.Len() > 0 ? UAkLocalizedMediaAsset::StaticClass() : UAkMediaAsset::StaticClass();

			FString assetPackagePath = FPaths::GetPath(mediaToCook->AssetPath.GetAssetPathName().ToString());

			auto assetData = sharedThis->assetRegistryModule->Get().GetAssetByObjectPath(mediaToCook->AssetPath.GetAssetPathName());

			if (assetData.IsValid())
			{
				mediaToCook->Instance = Cast<UAkMediaAsset>(assetData.GetAsset());
			}
			else
			{
				mediaToCook->Instance = Cast<UAkMediaAsset>(sharedThis->assetToolsModule->Get().CreateAsset(mediaToCook->AssetPath.GetAssetName(), assetPackagePath, mediaAssetClass, nullptr));
			}

			if (mediaToCook->Instance)
			{
				mediaToCook->CurrentPlatformData = mediaToCook->Instance->FindOrAddMediaAssetData(platform);
			}
		}, GET_STATID(STAT_GatherMediaAssets), nullptr, ENamedThreads::GameThread);

		auto cookTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis, mediaToCook, platform] {
			sharedThis->cookMediaAsset(*mediaToCook.Get(), platform);
		}, GET_STATID(STAT_CookMediaAsset), fetchMediaTask);

		{
			FScopeLock autoLock(&mediaLock);
			allMediaTasks.Add(cookTask);
		}
	}
}

void AkSoundDataBuilder::markAssetDirty(UObject* obj)
{
	AsyncTask(ENamedThreads::GameThread, [obj, sharedThis = SharedThis(this) ] {
		if (!obj->GetOutermost()->IsDirty())
		{
			obj->GetOutermost()->MarkPackageDirty();
		}
		if (sharedThis->AutoSave)
		{
			sharedThis->PackagesToSave.Add(obj->GetOutermost());
		}
	});
}

FString AkSoundDataBuilder::fillAudioBankInfoMap(AudioBankInfoMap& AudioBankInfoMap, FillAudioBankInfoKind InfoKind, const TSet<FString>& PlatformsToBuild, const TSet<FString>& LanguagesToBuild, ISoundBankInfoCache* SoundBankInfoCache)
{
	auto& akAssetDatabase = AkAssetDatabase::Get();
	TSet<FString> BanksToGenerate;

	{
		FScopeLock autoLock(&akAssetDatabase.EventMap.CriticalSection);

		for (auto& eventEntry : akAssetDatabase.EventMap.TypeMap)
		{
			if (auto eventInstance = Cast<UAkAudioEvent>(eventEntry.Value.GetAsset()))
			{
				if (eventInstance->RequiredBank)
				{
					if (!initParameters.SkipLanguages || (initParameters.SkipLanguages && eventInstance->RequiredBank->LocalizedPlatformAssetDataMap.Num() == 0))
					{
						FString audioBankName;
						if (AkToolBehavior::Get()->AkSoundDataBuilder_GetBankName(this, eventInstance->RequiredBank, initParameters.BanksToGenerate, audioBankName))
						{
							BanksToGenerate.Add(audioBankName);
						}

						if (initParameters.BanksToGenerate.Num() == 0 || initParameters.BanksToGenerate.Contains(audioBankName))
						{
							auto& infoEntry = AudioBankInfoMap.FindOrAdd(audioBankName);
							infoEntry.NeedsRebuild = infoEntry.NeedsRebuild || eventInstance->NeedsRebuild(PlatformsToBuild, LanguagesToBuild, SoundBankInfoCache);

							auto& eventSet = infoEntry.Events;
							switch (InfoKind)
							{
							case FillAudioBankInfoKind::GUID:
								eventSet.Add(eventInstance->ID.ToString(EGuidFormats::DigitsWithHyphensInBraces));
								break;
							case FillAudioBankInfoKind::AssetName:
								eventSet.Add(eventInstance->GetName());
								break;
							default:
								break;
							}
						}
					}
				}
			}
		}
	}

	{
		FScopeLock autoLock(&akAssetDatabase.AuxBusMap.CriticalSection);

		for (auto& auxBusEntry : akAssetDatabase.AuxBusMap.TypeMap)
		{
			if (auto auxBusInstance = Cast<UAkAuxBus>(auxBusEntry.Value.GetAsset()))
			{
				if (auxBusInstance->RequiredBank)
				{
					FString audioBankName;

					if (AkToolBehavior::Get()->AkSoundDataBuilder_GetBankName(this, auxBusInstance->RequiredBank, initParameters.BanksToGenerate, audioBankName))
					{
						BanksToGenerate.Add(audioBankName);
					}

					if (initParameters.BanksToGenerate.Num() == 0 || initParameters.BanksToGenerate.Contains(audioBankName))
					{
						auto& infoEntry = AudioBankInfoMap.FindOrAdd(audioBankName);
						infoEntry.NeedsRebuild = infoEntry.NeedsRebuild || auxBusInstance->NeedsRebuild(PlatformsToBuild, LanguagesToBuild, SoundBankInfoCache);

						auto& auxBusSet = infoEntry.AuxBusses;
						switch (InfoKind)
						{
						case FillAudioBankInfoKind::GUID:
							auxBusSet.Add(auxBusInstance->ID.ToString(EGuidFormats::DigitsWithHyphensInBraces));
							break;
						case FillAudioBankInfoKind::AssetName:
							auxBusSet.Add(auxBusInstance->GetName());
							break;
						default:
							break;
						}
					}
				}
			}
		}
	}

	return FString::Join(BanksToGenerate, TEXT(" "));
}

bool AkSoundDataBuilder::fillBankDataInAsset(UAkAssetData* AssetData, const TArray<uint8>& NewBankData, FCriticalSection* DataLock)
{
	FScopeLock autoLock(DataLock);

	bool changed = false;
	auto newDataSize = NewBankData.Num();
	int64 currentSize = AssetData->Data.GetBulkDataSize();
	void* currentBankData = AssetData->Data.Lock(EBulkDataLockFlags::LOCK_READ_WRITE);
	if (currentSize != newDataSize || FMemory::Memcmp(NewBankData.GetData(), currentBankData, FMath::Min(static_cast<int64>(newDataSize), AssetData->Data.GetBulkDataSize())) != 0)
	{
		void* rawBankData = AssetData->Data.Realloc(newDataSize);
		FMemory::Memcpy(rawBankData, NewBankData.GetData(), newDataSize);
		changed = true;
	}

	AssetData->Data.Unlock();

	return changed;
}

void AkSoundDataBuilder::loadAndWaitForAssetToLoad()
{
	if (IsRunningCommandlet())
	{
		return;
	}

	auto requestAsyncLoadTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis = SharedThis(this)] {
		if (auto audioDevice = FAkAudioDevice::Get())
		{
			TArray<FSoftObjectPath> assetsToLoad;

			for (auto& entry : AkAssetDatabase::Get().AudioTypeMap.TypeMap)
			{
				assetsToLoad.Add(entry.Value.ToSoftObjectPath());
			}

			sharedThis->loadedAssetsHandle = audioDevice->GetStreamableManager().RequestAsyncLoad(assetsToLoad);
		}
	}, GET_STATID(STAT_EventPlatformDataEventGroup), nullptr, ENamedThreads::GameThread);

	auto waitForCompletitionTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis = SharedThis(this)] {

		if (sharedThis->loadedAssetsHandle.IsValid())
		{
			while (!sharedThis->loadedAssetsHandle->HasLoadCompleted())
			{
				FPlatformProcess::Sleep(0.f);
			}
		}
	}, GET_STATID(STAT_EventPlatformDataEventGroup), requestAsyncLoadTask);

	FTaskGraphInterface::Get().WaitUntilTaskCompletes(waitForCompletitionTask);
}

void AkSoundDataBuilder::AutoSaveAssetsBlocking()
{
	if (AutoSave)
	{
		auto saveFilesTask = FFunctionGraphTask::CreateAndDispatchWhenReady([sharedThis = SharedThis(this)]() {
			UEditorLoadingAndSavingUtils::SavePackages(sharedThis->PackagesToSave.Array(), true);
		}, GET_STATID(STAT_AutoSaveAssets), nullptr, ENamedThreads::GameThread);
		FTaskGraphInterface::Get().WaitUntilTaskCompletes(saveFilesTask);
	}
}

#undef LOCTEXT_NAMESPACE
