//
//  SpinnerSliderPreference.qml
//
//  Created by Cain Kilgore on 11th July 2017
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5

import "../../dialogs"
import controlsUit 1.0

Preference {
    id: root
    property alias slider: slider
    property alias spinner: spinner
    height: control.height + hifi.dimensions.controlInterlineHeight

    Component.onCompleted: {
        slider.value = preference.value;
        spinner.realValue = preference.value;
    }

    function save() {
        preference.value = slider.value;
        preference.save();
    }

    Item {
        id: control
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: Math.max(labelText.height, slider.height, spinner.height, button.height)

        Label {
            id: labelText
            text: root.label + ":"
            colorScheme: hifi.colorSchemes.dark
            anchors {
                left: parent.left
                right: slider.left
                rightMargin: hifi.dimensions.labelPadding
                verticalCenter: parent.verticalCenter
            }
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.Wrap
        }

        Slider {
            id: slider
            value: preference.value
            width: 100
            minimumValue: preference.min
            maximumValue: preference.max
            stepSize: preference.step
            onValueChanged: {
                spinner.realValue = value
            }
            anchors {
                right: spinner.left
                rightMargin: 10
                verticalCenter: parent.verticalCenter
            }
            colorScheme: hifi.colorSchemes.dark
        }
        
        SpinBox {
            id: spinner
            decimals: preference.decimals
            realValue: preference.value
            minimumValue: preference.min
            maximumValue: preference.max
            width: 100
            onValueChanged: {
                slider.value = realValue;
            }
            anchors {
                right: button.left
                rightMargin: 10
                verticalCenter: parent.verticalCenter
            }
            colorScheme: hifi.colorSchemes.dark
        }

        GlyphButton {
            id: button
            onClicked: {
                if (spinner.maximumValue >= 1) {
                    spinner.realValue = 1
                    slider.value = 1
                } else {
                    spinner.realValue = spinner.maximumValue
                    slider.value = spinner.maximumValue
                }
            }
            width: 30
            glyph: hifi.glyphs.reload
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
            colorScheme: hifi.colorSchemes.dark
        }
    }
}
