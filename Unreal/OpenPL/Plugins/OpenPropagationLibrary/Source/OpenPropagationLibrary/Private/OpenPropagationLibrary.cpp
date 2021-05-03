// Copyright Epic Games, Inc. All Rights Reserved.

#include "OpenPropagationLibrary.h"
#include "OpenPL.hpp"

#define LOCTEXT_NAMESPACE "FOpenPropagationLibraryModule"

void FOpenPropagationLibraryModule::StartupModule()
{
    PL_RESULT SystemCreateResult = OpenPL::System_Create(&SystemInstance);
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
