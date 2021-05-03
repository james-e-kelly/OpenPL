// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

namespace OpenPL
{
class PLSystem;
class PLScene;
}

class FOpenPropagationLibraryModule : public IModuleInterface
{
public:
    
    static inline FOpenPropagationLibraryModule &Get() { return FModuleManager::LoadModuleChecked<FOpenPropagationLibraryModule>("OpenPropagationLibrary"); }

    static inline bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("OpenPropagationLibrary"); }
    
    OpenPL::PLSystem* GetSystem() { return SystemInstance; }
    
    OpenPL::PLScene* CreateScene();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
    
protected:
    
    OpenPL::PLSystem* SystemInstance;
};
