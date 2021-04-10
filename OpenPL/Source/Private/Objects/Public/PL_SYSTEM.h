/*
  ==============================================================================

    PL_SYSTEM.h
    Created: 3 Feb 2021 2:19:43pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "OpenPLCommon.h"
#include "OpenPLCommonPrivate.h"
#include <forward_list>
#include <memory>

/**
 * The system class is a container for scene objects. It's primary purpose is to remove having static variables inside this file.
 * Users have to create a system object before going further in the simulation.
 * And because the user has to release the object, it is a nice place to handle memory management.
 */
class PL_SYSTEM
{
public:
    PL_SYSTEM();
    
    ~PL_SYSTEM();
    
    PL_RESULT AddScene(PL_SCENE* Scene);
    
    PL_RESULT RemoveScene(PL_SCENE* Scene);
    
    PL_RESULT SetListenerPosition(const PLVector& NewListenerPosition);
    
    PL_RESULT GetListenerPosition(PLVector& OutListenerPosition) const;
    
private:
    std::forward_list<std::unique_ptr<PL_SCENE>> Scenes;
    
    PLVector ListenerPosition;
};
