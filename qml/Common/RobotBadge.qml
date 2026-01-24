import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"

Item {
    id: root

    signal disconnectRequested()

    property bool dropDownOpen: false
    property string robotDisplayName: (rosManager && rosManager.robotManager) ? rosManager.robotManager.selectedRobotDisplayName : ""
    property bool robotConnected: (rosManager && rosManager.robotManager) ? (rosManager.robotManager.selectedRobotNamespace !== "") : false

    implicitHeight: 56
    implicitWidth: robotLabelRow.implicitWidth + 20

    visible: robotConnected

    MouseArea {
        anchors.fill: parent
        enabled: root.dropDownOpen
        onClicked: root.dropDownOpen = false
        z: 1
    }

    Rectangle {
        id: robotLabel
        z: 3
        height: 56
        width: robotLabelRow.implicitWidth + 20
        radius: 18
        color: "#a9cfe8"
        border.color: "#ffffff"
        border.width: 2

        anchors.top: parent.top
        anchors.right: parent.right

        Row {
            id: robotLabelRow
            anchors.fill: parent
            anchors.margins: 10
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

        MouseArea {
            anchors.fill: parent
            onClicked: root.dropDownOpen = !root.dropDownOpen
        }
    }

    Rectangle {
        id: dropDown
        z: 3
        width: robotLabel.width
        height: root.dropDownOpen ? 48 : 0
        opacity: root.dropDownOpen ? 1 : 0
        radius: 14
        color: "#ffffff"
        border.color: "#045671"
        border.width: 2
        clip: true

        anchors.top: robotLabel.bottom
        anchors.right: robotLabel.right
        anchors.topMargin: 6

        Behavior on height {
            NumberAnimation {
                duration: 150
                easing.type: Easing.InOutQuad
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 120 }
        }

        Text {
            anchors.centerIn: parent
            text: qsTr("Desconectar")
            color: "#045671"
            font.pixelSize: 16
            font.bold: true
        }

        MouseArea {
            anchors.fill: parent
            onPressed: dropDown.color = "#00C8FF"
            onReleased: dropDown.color = "#ffffff"
            onCanceled: dropDown.color = "#ffffff"
            onClicked: {
                root.dropDownOpen = false
                disconnectDialog.openWithMessage(
                    qsTr("¿Cerrar conexión con %1?").arg(root.robotDisplayName)
                )
            }
        }
    }

    ConfirmationDialog {
        id: disconnectDialog

        onAccepted: root.disconnectRequested()
    }
}
