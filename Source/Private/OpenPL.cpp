/*
  ==============================================================================

    OpenPL.cpp
    Created: 15 Aug 2020 8:12:46pm
    Author:  James Kelly

  ==============================================================================
*/

#include "../Public/OpenPL.h"
#include <boost/multi_array.hpp>
#include <forward_list>

typedef boost::multi_array<PLVoxel, 3> VoxelArray;

class PL_SYSTEM
{
public:
    PL_SYSTEM()
    { }
    
    ~PL_SYSTEM()
    { }
    
    PL_RESULT AddScene(PL_SCENE* Scene)
    {
        Scenes.push_front(std::unique_ptr<PL_SCENE>(Scene));
        return PL_OK;
    }
    
    PL_RESULT RemoveScene(PL_SCENE* Scene)
    {
        Scenes.remove_if([Scene](std::unique_ptr<PL_SCENE>& Elem) { return Elem.get() == Scene; });
        return PL_OK;
    }

private:
    std::forward_list<std::unique_ptr<PL_SCENE>> Scenes;
};

class PL_SCENE
{
public:
    
    PL_SCENE(PL_SYSTEM* System)
    :   OwningSystem(System)
    { }
    
    PL_RESULT GetSystem(PL_SYSTEM** OutSystem)
    {
        *OutSystem = OwningSystem;
        return PL_OK;
    }
    
private:
    PL_SYSTEM* OwningSystem;
};

PL_RESULT PL_System_Create (PL_SYSTEM** OutSystem)
{
    if (!OutSystem)
    {
        return PL_ERR_MEMORY;
    }
    
    *OutSystem = new PL_SYSTEM();
    return PL_OK;
}

PL_RESULT PL_System_Release (PL_SYSTEM* System)
{
    delete System;
    return PL_OK;
}

PL_RESULT PL_System_CreateScene(PL_SYSTEM* System, PL_SCENE** OutScene)
{
    if (!System || OutScene)
    {
        return PL_ERR_MEMORY;
    }
    
    PL_SCENE* CreatedScene = new PL_SCENE(System);
    *OutScene = CreatedScene;
    PL_RESULT Result = System->AddScene(CreatedScene);
    return Result;
}

PL_RESULT PL_Scene_Release(PL_SCENE* Scene)
{
    if (!Scene)
    {
        return PL_OK;
    }
    
    PL_SYSTEM* System = nullptr;
    Scene->GetSystem(&System);
    if (System)
    {
        return System->RemoveScene(Scene);
    }
    return PL_ERR_MEMORY;
}

//PL_RESULT PLScene::GetBounds(PLBounds* OutBounds) const
//{
//    if (Impl)
//    {
//        *OutBounds = PLBounds::CreateAABB(Impl->SceneCenter, Impl->SceneSize);
//        return PL_OK;
//    }
//    return PL_ERR_MEMORY;
//}
