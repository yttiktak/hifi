import Hifi 1.0 as Hifi
import QtQuick 2.3
import '.'

Item {
    id: animStats

    anchors.leftMargin: 300
    objectName: "StatsItem"
    property int modality: Qt.NonModal
    implicitHeight: row.height
    implicitWidth: row.width

    Component.onCompleted: {
        animStats.parentChanged.connect(fill);
        fill();
    }
    Component.onDestruction: {
        animStats.parentChanged.disconnect(fill);
    }

    function fill() {
        // This will cause a  warning at shutdown, need to find another way to remove
        // the warning other than filling the anchors to the parent
        anchors.horizontalCenter = parent.horizontalCenter
    }

    Hifi.AnimStats {
        id: root
        objectName: "AnimStats"
        implicitHeight: row.height
        implicitWidth: row.width

        anchors.horizontalCenter: parent.horizontalCenter
        readonly property string bgColor: "#AA111111"

        Row {
            id: row
            spacing: 8

            Rectangle {
                width: firstCol.width + 8;
                height: firstCol.height + 8;
                color: root.bgColor;

                Column {
                    id: firstCol
                    spacing: 4; x: 4; y: 4;

                    StatText {
                        text: root.positionText
                    }
                    StatText {
                        text: root.recenterText
                    }
                    StatText {
                        text: root.overrideJointText
                    }
                    StatText {
                        text: "Anim Vars:--------------------------------------------------------------------------------"
                    }
                    ListView {
                        width: secondCol.width
                        height: root.animVars.length * 15
                        visible: root.animVars.length > 0;
                        model: root.animVars
                        delegate: StatText {
                            text: {
                                var actualText = modelData.split("|")[1];
                                if (actualText) {
                                    return actualText;
                                } else {
                                    return modelData;
                                }
                            }
                            color: {
                                var grayScale = parseFloat(modelData.split("|")[0]);
                                return Qt.rgba(1.0, 1.0, 1.0, grayScale);
                            }
                            styleColor: {
                                var grayScale = parseFloat(modelData.split("|")[0]);
                                return Qt.rgba(0.0, 0.0, 0.0, grayScale);
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: secondCol.width + 8
                height: secondCol.height + 8
                color: root.bgColor;

                Column {
                    id: secondCol
                    spacing: 4; x: 4; y: 4;

                    StatText {
                        text: root.rotationText
                    }
                    StatText {
                        text: root.sittingText
                    }
                    StatText {
                        text: root.flowText
                    }
                    StatText {
                        text: "State Machines:---------------------------------------------------------------------------"
                    }
                    ListView {
                        width: firstCol.width
                        height: root.animStateMachines.length * 15
                        visible: root.animStateMachines.length > 0;
                        model: root.animStateMachines
                        delegate: StatText {
                            text: {
                                return modelData;
                            }
                        }
                    }

                }
            }

            Rectangle {
                width: thirdCol.width + 8
                height: thirdCol.height + 8
                color: root.bgColor;

                Column {
                    id: thirdCol
                    spacing: 4; x: 4; y: 4;

                    StatText {
                        text: root.velocityText
                    }
                    StatText {
                        text: root.walkingText
                    }
                    StatText {
                        text: root.networkGraphText
                    }
                    StatText {
                        text: "Alpha Values:--------------------------------------------------------------------------"
                    }
                    ListView {
                        width: thirdCol.width
                        height: root.animAlphaValues.length * 15
                        visible: root.animAlphaValues.length > 0;
                        model: root.animAlphaValues
                        delegate: StatText {
                            text: {
                                var actualText = modelData.split("|")[1];
                                if (actualText) {
                                    return actualText;
                                } else {
                                    return modelData;
                                }
                            }
                            color: {
                                var grayScale = parseFloat(modelData.split("|")[0]);
                                return Qt.rgba(1.0, 1.0, 1.0, grayScale);
                            }
                            styleColor: {
                                var grayScale = parseFloat(modelData.split("|")[0]);
                                return Qt.rgba(0.0, 0.0, 0.0, grayScale);
                            }
                        }
                    }

                }
            }
        }

        Connections {
            target: root.parent
            onWidthChanged: {
                root.x = root.parent.width - root.width;
            }
        }
    }

}
