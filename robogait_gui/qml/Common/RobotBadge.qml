import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"

Item {
    id: root

    property string robotDisplayName: userSession ? userSession.robotDisplayName : ""
    property bool robotConnected: userSession ? userSession.hasRobotAssigned : false

    readonly property real badgeHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.badgeHeight, 0) : 56
    readonly property real iconGlyphPxRaw: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 34
    readonly property real badgeIconPx: Math.min(iconGlyphPxRaw, badgeHeightPx - 16)

    implicitHeight: Math.round(badgeHeightPx)
    implicitWidth: Math.max(robotLabelRow.implicitWidth + 20, 120)

    Rectangle {
        id: robotLabel
        z: 3
        height: root.implicitHeight
        width: root.implicitWidth
        radius: Math.round(height * 0.32)
        color: "#a9cfe8"
        border.color: "#ffffff"
        border.width: 2

        anchors.top: parent.top
        anchors.right: parent.right

        Row {
            id: robotLabelRow
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 10
            spacing: 14

            Rectangle {
                width: root.badgeIconPx
                height: root.badgeIconPx
                radius: Math.round(root.badgeIconPx * 0.28)
                color: "#4f86b4"
                border.color: "#ffffff"
                border.width: 1
                clip: true

                Image {
                    anchors.centerIn: parent
                    width: root.badgeIconPx * 0.68
                    height: root.badgeIconPx * 0.68
                    source: "qrc:/qmlresources/robot/robot.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }

            Text {
                text: root.robotDisplayName
                color: "#045671"
                font.pixelSize: 18
                font.bold: true
                elide: Text.ElideRight
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
