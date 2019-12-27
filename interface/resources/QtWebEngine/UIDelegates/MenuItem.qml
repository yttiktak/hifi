import QtQuick 2.5

import controlsUit 1.0
import stylesUit 1.0

Item {
    id: root
    objectName: "MenuItem"

    property alias text: label.text
    property var menu;
    property var shortcut;
    signal triggered();

    HifiConstants { id: hifi  }

    implicitHeight: 2 * label.implicitHeight
    implicitWidth: 2 * hifi.dimensions.menuPadding.x + label.width

    RalewaySemiBold {
        id: label
        size: hifi.fontSizes.rootMenu
        anchors.left: parent.left
        anchors.leftMargin: hifi.dimensions.menuPadding.x
        verticalAlignment: Text.AlignVCenter
        color: enabled ? hifi.colors.baseGrayShadow : hifi.colors.baseGrayShadow50
        enabled: root.enabled
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            menu.visible = false;
            root.triggered();
            menu.done();
        }
    }
}
