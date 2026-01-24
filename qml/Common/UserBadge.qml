import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0

import "qrc:/Dialogs"

Item {
    id: root

    property bool dropDownOpen: false
    property bool loggedIn: dbManager ? dbManager.passLogin : false
    property string userLabel: (dbManager && dbManager.displayName && dbManager.displayName.length)
                         ? dbManager.displayName
                         : ((dbManager && dbManager.userRole === "guest") ? qsTr("Invitado")
                                                                       : (dbManager ? dbManager.userName : ""))

    property int badgeWidth: Math.max(userRow.implicitWidth + 20, logoutText.implicitWidth + 40)

    implicitHeight: 56
    implicitWidth: badgeWidth

    visible: loggedIn

    MouseArea {
        anchors.fill: parent
        enabled: root.dropDownOpen
        onClicked: root.dropDownOpen = false
        z: 1
    }

    Rectangle {
        id: userLabelRect
        z: 3
        height: 56
        width: root.badgeWidth
        radius: 18
        color: "#a9cfe8"
        border.color: "#ffffff"
        border.width: 2

        anchors.top: parent.top
        anchors.right: parent.right

        RowLayout {
            id: userRow
            anchors.fill: parent
            anchors.margins: 10
            spacing: 12

            Item {
                Layout.preferredWidth: 34
                Layout.preferredHeight: 34
                Layout.minimumWidth: 34
                Layout.minimumHeight: 34
                Layout.maximumWidth: 34
                Layout.maximumHeight: 34
                Layout.leftMargin: 1
                Layout.alignment: Qt.AlignVCenter

                Image {
                    anchors.fill: parent
                    source: "qrc:/qmlresources/icons/circle_user_regular.svg"
                    sourceSize.width: 34
                    sourceSize.height: 34
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }

            Text {
                text: root.userLabel
                color: "#045671"
                font.pixelSize: 18
                font.bold: true
                elide: Text.ElideRight
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Item {
                Layout.preferredWidth: 34
                Layout.preferredHeight: 34
                Layout.minimumWidth: 34
                Layout.minimumHeight: 34
                Layout.maximumWidth: 34
                Layout.maximumHeight: 34
                Layout.alignment: Qt.AlignVCenter
                visible: false
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
        width: root.badgeWidth
        height: root.dropDownOpen ? 48 : 0
        opacity: root.dropDownOpen ? 1 : 0
        radius: 14
        color: "#ffffff"
        border.color: "#045671"
        border.width: 2
        clip: true

        anchors.top: userLabelRect.bottom
        anchors.right: userLabelRect.right
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
            id: logoutText
            anchors.centerIn: parent
            text: qsTr("Cerrar sesión")
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
                logoutDialog.openWithMessage(qsTr("¿Cerrar sesión?"))
            }
        }
    }

    ConfirmationDialog {
        id: logoutDialog
        holdToAccept: true
        acceptText: qsTr("Cerrar sesión")

        onAccepted: {
            if (dbManager) {
                dbManager.logout()
            }
        }
    }
}
