
#include "ReverbJUCEPluginGUI.h"

ReverbJUCEPluginGUI::ReverbJUCEPluginGUI()
{
}

ADD_AUDIOPLUGIN_CLASS_TO_CONTAINER(
    ReverbJUCE,            // Name of the plug-in container for this shared library
    ReverbJUCEPluginGUI,   // Authoring plug-in class to add to the plug-in container
    ReverbJUCEFX           // Corresponding Sound Engine plug-in class
);
