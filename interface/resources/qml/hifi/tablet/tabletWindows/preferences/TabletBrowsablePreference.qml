//
//  BrowsablePreference.qml
//
//  Created by Dante Ruiz Davis on 13 Feb 2017
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5

import "../../../../dialogs"
import controlsUit 1.0
import "../"

Preference {
    id: root
    property alias text: dataTextField.text
    property alias placeholderText: dataTextField.placeholderText
    height: control.height + hifi.dimensions.controlInterlineHeight

    Component.onCompleted: {
        dataTextField.text = preference.value;
    }

    function save() {
        preference.value = dataTextField.text;
        preference.save();
    }

    Item {
        id: control
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: Math.max(dataTextField.controlHeight, button.height)

        TextField {
            id: dataTextField

            anchors {
                left: parent.left
                right: button.left
                rightMargin: hifi.dimensions.contentSpacing.x
                bottom: parent.bottom
            }

            label: root.label
            placeholderText: root.placeholderText
            colorScheme: hifi.colorSchemes.dark
        }

        Component {
            id: fileBrowserBuilder;
            TabletFileDialog { selectDirectory: true }
        }

        Button {
            id: button
            text: preference.browseLabel
            anchors {
                right: parent.right
                verticalCenter: dataTextField.verticalCenter
            }
            onClicked: {
                var browser = fileBrowserBuilder.createObject({
                    selectDirectory: true,
                    dir: fileDialogHelper.pathToUrl(preference.value)
                });

                browser.selectedFile.connect(function(fileUrl){
                    dataTextField.text = fileDialogHelper.urlToPath(fileUrl);
                });
                
                profileRoot.push(browser);
            }
        }
    }
}
