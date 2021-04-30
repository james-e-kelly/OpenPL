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

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
    
protected:
    
    OpenPL::PLSystem* SystemInstance;
};
