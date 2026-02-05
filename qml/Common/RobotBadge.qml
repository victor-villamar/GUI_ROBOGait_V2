import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"

Item {
    id: root

    property string robotDisplayName: userSession ? userSession.robotDisplayName : ""
    property bool robotConnected: userSession ? userSession.hasRobotAssigned : false

    implicitHeight: 56
    implicitWidth: Math.max(robotLabelRow.implicitWidth + 20, 120)

    Rectangle {
        id: robotLabel
        z: 3
        height: 56
        width: root.implicitWidth
        radius: 18
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
                width: 44
                height: 44
                radius: 12
                color: "#4f86b4"
                border.color: "#ffffff"
                border.width: 1
                clip: true

                Image {
                    anchors.centerIn: parent
                    width: 30
                    height: 30
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
