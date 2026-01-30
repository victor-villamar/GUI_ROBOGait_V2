import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0

import "qrc:/Dialogs"

Item {
    id: root

    signal disconnectRobotRequested()
    signal changeUserRequested()

    property bool dropDownOpen: false
    property bool loggedIn: userSession ? userSession.isAuthenticated : false
    property bool robotConnected: userSession ? userSession.hasRobotAssigned : false
    property string robotDisplayName: userSession ? userSession.robotDisplayName : ""

    property string userLabel: (userSession && userSession.displayName && userSession.displayName.length)
                               ? userSession.displayName
                               : ((userSession && userSession.role === "guest") ? qsTr("Invitado")
                                                                                : (dbManager ? dbManager.userName : ""))

    property int badgeWidth: Math.max(userRow.implicitWidth + 20, 140)
    property int segmentHeight: 44
    property int segmentPadding: 16
    property int dividerSize: 2
    property int segmentWidth: Math.max(
                                 120,
                                 disconnectText.implicitWidth + root.segmentPadding * 2,
                                 changeUserText.implicitWidth + root.segmentPadding * 2,
                                 logoutText.implicitWidth + root.segmentPadding * 2
                               )
    property int menuWidth: Math.max(root.badgeWidth, root.segmentWidth)

    implicitHeight: 56
    implicitWidth: badgeWidth

    visible: loggedIn

    Timer {
        id: autoCloseTimer
        interval: 3000
        repeat: false
        onTriggered: root.dropDownOpen = false
    }

    function restartAutoCloseTimer()
    {
        autoCloseTimer.stop()
        if (root.dropDownOpen) {
            autoCloseTimer.start()
        }
    }

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
            onClicked: {
                root.dropDownOpen = !root.dropDownOpen
                root.restartAutoCloseTimer()
            }
        }
    }

    Rectangle {
        id: dropDown
        z: 3
        width: root.menuWidth
        height: root.dropDownOpen
              ? (root.segmentHeight * (root.robotConnected ? 3 : 2) + root.dividerSize * (root.robotConnected ? 2 : 1))
              : 0
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

        Column {
            id: segmentedColumn
            anchors.fill: parent
            spacing: 0

            Rectangle {
                id: disconnectSegment
                width: parent.width
                height: root.segmentHeight
                color: disconnectArea.pressed ? "#00C8FF" : "transparent"
                visible: root.robotConnected

                Text {
                    id: disconnectText
                    anchors.centerIn: parent
                    text: qsTr("Desconectar")
                    color: root.robotConnected ? "#045671" : "#8a8a8a"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: disconnectArea
                    anchors.fill: parent
                    enabled: root.robotConnected
                    onPressed: root.restartAutoCloseTimer()
                    onClicked: {
                        root.dropDownOpen = false
                        disconnectRobotDialog.openWithMessage(
                            qsTr("¿Cerrar conexión con %1?").arg(root.robotDisplayName)
                        )
                    }
                }
            }

            Rectangle {
                id: dividerAfterDisconnect
                width: parent.width
                height: root.dividerSize
                color: "#045671"
                opacity: 0.35
                visible: root.robotConnected
            }

            Rectangle {
                id: changeUserSegment
                width: parent.width
                height: root.segmentHeight
                color: changeUserArea.pressed ? "#00C8FF" : "transparent"

                Text {
                    id: changeUserText
                    anchors.centerIn: parent
                    text: qsTr("Cambiar usuario")
                    color: "#045671"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: changeUserArea
                    anchors.fill: parent
                    onPressed: root.restartAutoCloseTimer()
                    onClicked: {
                        root.dropDownOpen = false
                        root.changeUserRequested()
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: root.dividerSize
                color: "#045671"
                opacity: 0.35
            }

            Rectangle {
                id: logoutSegment
                width: parent.width
                height: root.segmentHeight
                color: logoutArea.pressed ? "#00C8FF" : "transparent"

                Text {
                    id: logoutText
                    anchors.centerIn: parent
                    text: qsTr("Cerrar sesión")
                    color: "#045671"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: logoutArea
                    anchors.fill: parent
                    onPressed: root.restartAutoCloseTimer()
                    onClicked: {
                        root.dropDownOpen = false
                        var msg = qsTr("¿Cerrar sesión?")
                        if (root.robotConnected && root.robotDisplayName && root.robotDisplayName.length) {
                            msg += "\n\n" + qsTr("Se cerrará la conexión con %1.").arg(root.robotDisplayName)
                        }
                        logoutDialog.openWithMessage(msg)
                    }
                }
            }
        }
    }

    ConfirmationDialog {
        id: disconnectRobotDialog
        acceptText: qsTr("Desconectar")

        onAccepted: root.disconnectRobotRequested()
    }

    ConfirmationDialog {
        id: logoutDialog
        holdToAccept: true
        acceptText: qsTr("Cerrar sesión")

        onAccepted: {
            if (userSession) {
                userSession.logout()
            }
        }
    }
}
