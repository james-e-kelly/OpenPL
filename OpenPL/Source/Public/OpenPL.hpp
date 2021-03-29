/*
  ==============================================================================

    OpenPL_CPP.h
    Created: 20 Aug 2020 4:30:35pm
    Author:  James Kelly

  ==============================================================================
*/

/**
 * Header containing CPP objects for programming in CPP.
 */

#pragma once

#include "OpenPL.h"

namespace OpenPL
{
    class PLSystem;
    class PLScene;

    /**
     * Handles object creation, memory management and baking.
     *
     * If any OpenPL object needs to be created, it should be made through the PLSystem.
     */
    class PLSystem
    {
    private:
        
        PLSystem();
        PLSystem(const PLSystem &);
        
    public:
        
        /**
         * Releases and destroys this object.
         */
        PL_RESULT Release();
        
        /**
         * Creates a new scene object.
         *
         * @param OutScene The newly created scene object.
         */
        PL_RESULT CreateScene(PLScene** OutScene);
    };

    class PLScene
    {
    private:
        
        PLScene();
        PLScene(const PLScene &);
        
    public:

        /**
         * Releases and destroys this scene.
         */
        PL_RESULT Release();
    };
}
