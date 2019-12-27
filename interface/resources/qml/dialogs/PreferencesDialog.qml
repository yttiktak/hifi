//
//  PreferencesDialog.qml
//
//  Created by Bradley Austin Davis on 24 Jan 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5

import controlsUit 1.0 as HifiControls
import stylesUit 1.0
import "../windows"
import "preferences"

ScrollingWindow {
    id: root
    title: "Preferences"
    resizable: true
    destroyOnHidden: true
    width: 500
    height: 577
    property var sections: []
    property var showCategories: []
    minSize: Qt.vector2d(400, 500)

    HifiConstants { id: hifi }

    function saveAll() {
        for (var i = 0; i < sections.length; ++i) {
            var section = sections[i];
            section.saveAll();
        }
        destroy();
    }

    function restoreAll() {
        for (var i = 0; i < sections.length; ++i) {
            var section = sections[i];
            section.restoreAll();
        }
        destroy();
    }

    Column {
        width: pane.contentWidth

        Component {
            id: sectionBuilder
            Section { }
        }

        Component.onCompleted: {
            var categories = Preferences.categories;
            var i;

            // build a map of valid categories.
            var categoryMap = {};
            for (i = 0; i < categories.length; i++) {
                categoryMap[categories[i]] = true;
            }

            // create a section for each valid category in showCategories
            // NOTE: the sort order of items in the showCategories array is the same order in the dialog.
            for (i = 0; i < showCategories.length; i++) {
                if (categoryMap[showCategories[i]]) {
                    sections.push(sectionBuilder.createObject(prefControls, {name: showCategories[i]}));
                }
            }

            var useKeyboardPreference = findPreference("User Interface", "Use Virtual Keyboard");
            var keyboardInputPreference = findPreference("User Interface", "Keyboard laser / mallets");
            if (useKeyboardPreference && keyboardInputPreference) {
                keyboardInputPreference.visible = useKeyboardPreference.value;
                useKeyboardPreference.valueChanged.connect(function() {
                    keyboardInputPreference.visible = useKeyboardPreference.value;
                });
            }

            if (sections.length) {
                // Default sections to expanded/collapsed as appropriate for dialog.
                if (sections.length === 1) {
                    sections[0].collapsable = false
                    sections[0].expanded = true
                } else {
                    for (i = 0; i < sections.length; i++) {
                        sections[i].collapsable = true;
                        sections[i].expanded = true;
                    }
                }
                sections[0].isFirst = true;
                sections[sections.length - 1].isLast = true;
            }
        }

        Column {
            id: prefControls
            width: pane.contentWidth
        }
    }

    footer: Row {
        anchors {
            top: parent.top
            right: parent.right
            rightMargin: hifi.dimensions.contentMargin.x
        }
        spacing: hifi.dimensions.contentSpacing.x

        HifiControls.Button {
            text: "Save changes"
            color: 1//hifi.buttons.blue
            onClicked: root.saveAll()
        }

        HifiControls.Button {
            text: "Cancel"
            color: 2//hifi.buttons.white
            onClicked: root.restoreAll()
        }
    }

    function findPreference(category, name) {
        var section = null;
        var preference = null;
        var i;

        // Find category section.
        i = 0;
        while (!section && i < sections.length) {
            if (sections[i].name === category) {
                section = sections[i];
            }
            i++;
        }

        // Find named preference.
        if (section) {
            i = 0;
            while (!preference && i < section.preferences.length) {
                if (section.preferences[i].preference && section.preferences[i].preference.name === name) {
                    preference = section.preferences[i];
                }
                i++;
            }
        }

        return preference;
    }
}
