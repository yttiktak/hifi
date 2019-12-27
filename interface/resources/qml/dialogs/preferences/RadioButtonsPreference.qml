//
//  RadioButtonsPreference.qml
//
//  Created by Cain Kilgore on 20th July 2017
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5

import controlsUit 1.0
import stylesUit 1.0

Preference {
    id: root

    height: control.height + hifi.dimensions.controlInterlineHeight

    property int value: 0

    readonly property int visibleBottomPadding: 3
    readonly property int invisibleBottomPadding: 0
    readonly property int indentLeftMargin: 20
    readonly property int nonindentLeftMargin: 0

    Component.onCompleted: {
        value = preference.value;
        repeater.itemAt(preference.value).checked = true;
    }

    function updateValue() {
        for (var i = 0; i < repeater.count; i++) {
            if (repeater.itemAt(i).checked) {
                value = i;
                break;
            }
        }
    }

    function save() {
        var value = 0;
        for (var i = 0; i < repeater.count; i++) {
            if (repeater.itemAt(i).checked) {
                value = i;
                break;
            }
        }
        preference.value = value;
        preference.save();
    }

    RalewaySemiBold {
        id: heading
        size: hifi.fontSizes.inputLabel
        text: preference.heading
        color: hifi.colors.lightGrayText
        visible: text !== ""
        bottomPadding: heading.visible ? visibleBottomPadding : invisibleBottomPadding
    }

    Column {
        id: control
        anchors {
            left: parent.left
            right: parent.right
            top: heading.visible ? heading.bottom : heading.top
            leftMargin: preference.indented ? indentLeftMargin : nonindentLeftMargin
        }
        spacing: 3
        Repeater {
            id: repeater
            model: preference.items.length
            delegate: RadioButton {
                text: preference.items[index]
                letterSpacing: 0
                anchors {
                    left: parent.left
                }
                leftPadding: 0
                colorScheme: hifi.colorSchemes.dark
                onClicked: updateValue();
            }
        }
    }
}
