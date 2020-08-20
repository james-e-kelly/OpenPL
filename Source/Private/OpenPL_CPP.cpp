/*
  ==============================================================================

    OpenPL_CPP.cpp
    Created: 20 Aug 2020 4:30:43pm
    Author:  James Kelly

  ==============================================================================
*/

#include "../Public/OpenPL_CPP.h"

namespace OpenPL
{
    PL_RESULT PLSystem::Release()
    {
        return PL_System_Release((PL_SYSTEM*)this);
    }

    PL_RESULT PLSystem::CreateScene(PLScene** OutScene)
    {
        PL_SCENE* CreatedScene = nullptr;
        PL_RESULT Result = PL_System_CreateScene((PL_SYSTEM*)this, &CreatedScene);
        *OutScene = (PLScene*)CreatedScene;
        return Result;
    }

    PL_RESULT PLScene::Release()
    {
        return PL_Scene_Release((PL_SCENE*)this);
    }
}
