//
//  ButtonPreference.qml
//
//  Created by Bradley Austin Davis on 18 Jan 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import TabletScriptingInterface 1.0

import controlsUit 1.0

Preference {
    id: root
    height: button.height + hifi.dimensions.controlInterlineHeight

    Component.onCompleted: button.text = preference.name;

    function save() { }

    Button {
        id: button
        onHoveredChanged: {
            if (hovered) {
                Tablet.playSound(TabletEnums.ButtonHover);
            }
        }

        onClicked: {
            preference.trigger();
            Tablet.playSound(TabletEnums.ButtonClick);
        }
        width: 180
        anchors.bottom: parent.bottom
    }
}
