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
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOpenPropagationLibraryModule, OpenPropagationLibrary)
