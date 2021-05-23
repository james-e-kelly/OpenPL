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


#include "AkBankManager.h"

#include "AkAudioBank.h"
#include "AkAudioDevice.h"
#include "AkInclude.h"
#include "Async/Async.h"
#include "Misc/ScopeLock.h"

FAkBankManager* FAkBankManager::Instance = nullptr;

FAkBankManager* FAkBankManager::GetInstance()
{
	return Instance;
}

void FAkBankFunctionPtrCallbackInfo::HandleAction(AkUInt32 BankID, const void * InMemoryBankPtr, AKRESULT ActionResult)
{
	if (CallbackFunc)
	{
		// Call the user's callback function
		CallbackFunc(BankID, InMemoryBankPtr, ActionResult, UserCookie);
	}
}

void FAkBankLatentActionCallbackInfo::HandleAction(AkUInt32 BankID, const void * InMemoryBankPtr, AKRESULT ActionResult)
{
	if (BankLatentAction)
	{
		BankLatentAction->ActionDone = true;
	}
}

void FAkBankBlueprintDelegateCallbackInfo::HandleAction(AkUInt32 BankID, const void * InMemoryBankPtr, AKRESULT ActionResult)
{
	if (BankBlueprintCallback.IsBound())
	{
		auto CachedBlueprintCallback = BankBlueprintCallback;
		AsyncTask(ENamedThreads::GameThread, [ActionResult, CachedBlueprintCallback]()
		{
			CachedBlueprintCallback.ExecuteIfBound((EAkResult)ActionResult);
		});
	}
}

void FAkBankManager::BankLoadCallback(
	AkUInt32		in_bankID,
	const void *	in_pInMemoryBankPtr,
	AKRESULT		in_eLoadResult,
	void *			in_pCookie
)
{
	if (in_pCookie)
	{
		IAkBankCallbackInfo* BankCbInfo = (IAkBankCallbackInfo*)in_pCookie;
		if (in_eLoadResult == AK_Success)
		{
			if (BankCbInfo->Bank)
			{
				// Load worked; put the bank in the list.
				GetInstance()->AddLoadedBank(BankCbInfo->Bank);
			}
		}

		BankCbInfo->HandleAction(in_bankID, in_pInMemoryBankPtr, in_eLoadResult);

		delete BankCbInfo;
	}
}

void FAkBankManager::BankUnloadCallback(
	AkUInt32		in_bankID,
	const void *	in_pInMemoryBankPtr,
	AKRESULT		in_eUnloadResult,
	void *			in_pCookie
)
{
	if (in_pCookie)
	{
		IAkBankCallbackInfo* BankCbInfo = (IAkBankCallbackInfo*)in_pCookie;
		if (in_eUnloadResult == AK_Success)
		{
			if (BankCbInfo->Bank)
			{
				// Load worked; put the bank in the list.
				GetInstance()->RemoveLoadedBank(BankCbInfo->Bank);
			}
		}

		BankCbInfo->HandleAction(in_bankID, in_pInMemoryBankPtr, in_eUnloadResult);

		delete BankCbInfo;
	}
}

FAkBankManager::FAkBankManager()
{
	if (Instance)
	{
		UE_LOG(LogInit, Error, TEXT("FAkBankManager has already been instantiated."));
	}

	Instance = this;
}

FAkBankManager::~FAkBankManager()
{
	if (Instance == this)
	{
		auto GcCopy = TArray<TStrongObjectPtr<UAkAudioBank>>(gcStorage);
		TSet<UAkAudioBank*> LoadedBanksCopy(m_LoadedBanks);
		for (auto* AudioBank : LoadedBanksCopy)
		{
			if (AudioBank != nullptr && AudioBank->IsValidLowLevel())
			{
				AudioBank->Unload();
			}
		}

		Instance = nullptr;
	}
}

void FAkBankManager::AddLoadedBank(UAkAudioBank* Bank)
{
	if (!Bank)
	{
		return;
	}

	FScopeLock autoLock(&m_bankLock);
	auto LoadCnt = Bank->LegacyLoadedRefCnt.Increment();
	if (LoadCnt == 1)
	{
		bool bIsAlreadyInSet = false;
		m_LoadedBanks.Add(Bank, &bIsAlreadyInSet);
		addToGCStorage(Bank);
		ensureMsgf(bIsAlreadyInSet == false, TEXT("Soundbank already loaded; data returned by AkBankManager::GetLoadedBankList may become incorrect."));
	}
}

void FAkBankManager::RemoveLoadedBank(UAkAudioBank* Bank)
{
	if (!Bank)
	{
		return;
	}

	FScopeLock autoLock(&m_bankLock);
	auto LoadCnt = Bank->LegacyLoadedRefCnt.Decrement();
	if (LoadCnt == 0)
	{
		m_LoadedBanks.Remove(Bank);
		removeFromGCStorage(Bank);
	}
}

void FAkBankManager::ClearLoadedBanks()
{
	FScopeLock autoLock(&m_bankLock);
	m_LoadedBanks.Empty();
	gcStorage.Empty();
}


void FAkBankManager::addToGCStorage(UAkAudioBank* Bank)
{
	if (IsInGameThread())
	{
		addToGCStorageInternal(Bank);
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [this, Bank] {
			addToGCStorageInternal(Bank);
		});
	}
}

void FAkBankManager::addToGCStorageInternal(UAkAudioBank* Bank)
{
	for (auto& entry : gcStorage)
	{
		if (entry.Get() == Bank)
		{
			return;
		}
	}

	gcStorage.Emplace(Bank);
}

void FAkBankManager::removeFromGCStorage(UAkAudioBank* Bank)
{
	if (IsInGameThread())
	{
		removeFromGCStorageInternal(Bank);
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [this, Bank] {
			removeFromGCStorageInternal(Bank);
		});
	}
}

void FAkBankManager::removeFromGCStorageInternal(UAkAudioBank* Bank)
{
	auto removeIndex = gcStorage.IndexOfByPredicate([Bank](auto& item) { return item.Get() == Bank; });
	if (removeIndex != -1)
	{
		gcStorage.RemoveAt(removeIndex);
	}
}
