//
//  InputPlugin.cpp
//  input-plugins/src/input-plugins
//
//  Created by Sam Gondelman on 7/13/2015
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "InputPlugin.h"

#include <plugins/PluginManager.h>

#include "KeyboardMouseDevice.h"
#include "TouchscreenDevice.h"
#include "TouchscreenVirtualPadDevice.h"

#if !defined(CUSTOM_INPUT_PLUGINS)
// TODO migrate to a DLL model where plugins are discovered and loaded at runtime by the PluginManager class
InputPluginList getInputPlugins() {
    InputPlugin* PLUGIN_POOL[] = {
        new KeyboardMouseDevice(),
#if defined(Q_OS_ANDROID)
        new TouchscreenVirtualPadDevice(),
#else
        new TouchscreenDevice(), // Touchscreen and Controller Scripts take care on Android
#endif
        nullptr
    };

    InputPluginList result;
    for (int i = 0; PLUGIN_POOL[i]; ++i) {
        InputPlugin* plugin = PLUGIN_POOL[i];
        if (plugin->isSupported()) {
            result.push_back(InputPluginPointer(plugin));
        }
    }
    return result;
}
#endif

void saveInputPluginSettings(const InputPluginList& plugins) {
    foreach (auto inputPlugin, plugins) {
        inputPlugin->saveSettings();
    }
}
