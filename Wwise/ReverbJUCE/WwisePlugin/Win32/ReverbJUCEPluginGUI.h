#pragma once

#include "../ReverbJUCEPlugin.h"

class ReverbJUCEPluginGUI final
	: public AK::Wwise::Plugin::PluginMFCWindows<>
	, public AK::Wwise::Plugin::GUIWindows
{
public:
	ReverbJUCEPluginGUI();

};
