//
//  TabletGraphicsPreferences.qml
//
//  Created by Vlad Stelmahovsky on 12 Mar 2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.7
import QtQuick.Controls 2.2
import "tabletWindows"
import "../../dialogs"

StackView {
    id: profileRoot
    initialItem: root
    objectName: "stack"
    property string title: "Graphics Settings"

    signal sendToScript(var message);

    function pushSource(path) {
        var item = Qt.createComponent(Qt.resolvedUrl(path));
        profileRoot.push(item);
    }

    function popSource() {
        profileRoot.pop();
    }

    TabletPreferencesDialog {
        id: root
        objectName: "TabletGraphicsPreferences"
        showCategories: ["Graphics Quality"]
    }
}
