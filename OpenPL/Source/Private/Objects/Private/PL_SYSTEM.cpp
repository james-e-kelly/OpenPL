/*
  ==============================================================================

    PL_SYSTEM.cpp
    Created: 3 Feb 2021 2:19:49pm
    Author:  James Kelly

  ==============================================================================
*/

#include "PL_SYSTEM.h"
#include "PL_SCENE.h"

PL_SYSTEM::PL_SYSTEM()
{ }

PL_SYSTEM::~PL_SYSTEM()
{ }

PL_RESULT PL_SYSTEM::AddScene(PL_SCENE* Scene)
{
    Scenes.push_front(std::unique_ptr<PL_SCENE>(Scene));
    return PL_OK;
}

PL_RESULT PL_SYSTEM::RemoveScene(PL_SCENE* Scene)
{
    Scenes.remove_if([Scene](std::unique_ptr<PL_SCENE>& Elem) { return Elem.get() == Scene; });
    return PL_OK;
}

PL_RESULT PL_SYSTEM::SetListenerPosition(const PLVector& NewListenerPosition)
{
    ListenerPosition = NewListenerPosition;
    return PL_OK;
}

PL_RESULT PL_SYSTEM::GetListenerPosition(PLVector& OutListenerPosition) const
{
    OutListenerPosition = ListenerPosition;
    return PL_OK;
}
