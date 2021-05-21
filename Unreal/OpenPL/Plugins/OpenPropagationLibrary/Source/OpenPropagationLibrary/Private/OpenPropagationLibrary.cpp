// Copyright Epic Games, Inc. All Rights Reserved.

#include "OpenPropagationLibrary.h"
#include "OpenPL.hpp"

#define LOCTEXT_NAMESPACE "FOpenPropagationLibraryModule"

//typedef PL_RESULT (*PL_Debug_Callback)     (const char* Message, PL_DEBUG_LEVEL Level);
PL_RESULT DebugCallback (const char* Message, PL_DEBUG_LEVEL Level)
{
    if (Level & PL_DEBUG_LEVEL_ERR)
    {
        FString StringMessage = UTF8_TO_TCHAR(Message);
        UE_LOG(LogTemp, Error, TEXT("[OpenPL] %s"), *StringMessage);
    }
    else if (Level & PL_DEBUG_LEVEL_WARN)
    {
        FString StringMessage = UTF8_TO_TCHAR(Message);
        UE_LOG(LogTemp, Warning, TEXT("[OpenPL] %s"), *StringMessage);
    }
    else if (Level & PL_DEBUG_LEVEL_LOG)
    {
        FString StringMessage = UTF8_TO_TCHAR(Message);
        UE_LOG(LogTemp, Log, TEXT("[OpenPL] %s"), *StringMessage);
    }
    return PL_OK;
}

void FOpenPropagationLibraryModule::StartupModule()
{
    PL_RESULT SystemCreateResult = OpenPL::System_Create(&SystemInstance);
    
    if (SystemCreateResult == PL_OK)
    {
        PL_Debug_Initialize (DebugCallback);
    }
}

void FOpenPropagationLibraryModule::ShutdownModule()
{
    if (SystemInstance)
    {
        SystemInstance->Release();
    }
}

OpenPL::PLScene* FOpenPropagationLibraryModule::CreateScene()
{
    OpenPL::PLScene* Result = nullptr;
    if (SystemInstance)
    {
        SystemInstance->CreateScene(&Result);
    }
    return Result;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOpenPropagationLibraryModule, OpenPropagationLibrary)
