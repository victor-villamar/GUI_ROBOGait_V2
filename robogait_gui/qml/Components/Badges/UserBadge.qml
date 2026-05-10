import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0
import QtQuick.Window 2.15
import AppTheme 1.0

import "qrc:/Dialogs"

Item {
    id: root

    signal disconnectRobotRequested()
    signal changeUserRequested()
    signal appExitRequested()

    property bool dropDownOpen: false
    property bool loggedIn: userSession ? userSession.isAuthenticated : false
    property bool robotConnected: userSession ? userSession.hasRobotAssigned : false
    property string robotDisplayName: userSession ? userSession.robotDisplayName : ""

    property string userLabel: (userSession && userSession.displayName && userSession.displayName.length)
                               ? userSession.displayName
                               : ((userSession && userSession.role === "guest") ? qsTr("Invitado")
                                                                                : (dbManager ? dbManager.userName : ""))

    readonly property real badgeHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.badgeHeight, 0) : 56
    readonly property real badgeMenuItemHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.badgeMenuItemHeight, 0) : 44
    readonly property real iconGlyphPxRaw: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 34
    readonly property real badgeIconPx: Math.min(iconGlyphPxRaw, badgeHeightPx - 16)

    property int badgeWidth: Math.max(userRow.implicitWidth + 20, 140)
    property int segmentHeight: Math.round(badgeMenuItemHeightPx)
    property int segmentPadding: 16
    property int dividerSize: 2
    property int segmentCount: root.robotConnected ? 4 : 3
    property int dividerCount: segmentCount - 1
    property int segmentWidth: Math.max(
                                 120,
                                 disconnectText.implicitWidth + root.segmentPadding * 2,
                                 changeUserText.implicitWidth + root.segmentPadding * 2,
                                 logoutText.implicitWidth + root.segmentPadding * 2,
                                 appExitText.implicitWidth + root.segmentPadding * 2
                               )
    property int menuWidth: Math.max(root.badgeWidth, root.segmentWidth)

    implicitHeight: Math.round(badgeHeightPx)
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
        height: root.implicitHeight
        width: root.badgeWidth
        radius: Math.round(height * 0.32)
        color: AppTheme.badges.badgeBackground
        border.color: AppTheme.badges.light
        border.width: 2

        anchors.top: parent.top
        anchors.right: parent.right

        RowLayout {
            id: userRow
            anchors.fill: parent
            anchors.margins: 10
            spacing: 12

            Item {
                Layout.preferredWidth: root.badgeIconPx
                Layout.preferredHeight: root.badgeIconPx
                Layout.minimumWidth: root.badgeIconPx
                Layout.minimumHeight: root.badgeIconPx
                Layout.maximumWidth: root.badgeIconPx
                Layout.maximumHeight: root.badgeIconPx
                Layout.leftMargin: 1
                Layout.alignment: Qt.AlignVCenter

                Image {
                    anchors.fill: parent
                    source: "qrc:/qmlresources/icons/white/circle_user_regular.svg"
                    sourceSize.width: root.badgeIconPx
                    sourceSize.height: root.badgeIconPx
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }

            Text {
                text: root.userLabel
                color: AppTheme.badges.primary
                font.pixelSize: 18
                font.bold: true
                elide: Text.ElideRight
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Item {
                Layout.preferredWidth: root.badgeIconPx
                Layout.preferredHeight: root.badgeIconPx
                Layout.minimumWidth: root.badgeIconPx
                Layout.minimumHeight: root.badgeIconPx
                Layout.maximumWidth: root.badgeIconPx
                Layout.maximumHeight: root.badgeIconPx
                Layout.alignment: Qt.AlignVCenter
                visible: false
            }
        }

        HoverHandler {
            id: userHover
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.dropDownOpen = !root.dropDownOpen
                root.restartAutoCloseTimer()
            }
        }

        ToolTip.visible: userHover.hovered
        ToolTip.text: qsTr("Usuario")
        ToolTip.delay: 500
    }

    Rectangle {
        id: dropDown
        z: 3
        width: root.menuWidth
        height: root.dropDownOpen
              ? (root.segmentHeight * root.segmentCount + root.dividerSize * root.dividerCount)
              : 0
        opacity: root.dropDownOpen ? 1 : 0
        radius: 14
        color: AppTheme.badges.light
        border.color: AppTheme.badges.primary
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
                color: disconnectArea.pressed ? AppTheme.badges.pressedBackground : "transparent"
                visible: root.robotConnected

                Text {
                    id: disconnectText
                    anchors.centerIn: parent
                    text: qsTr("Desconectar")
                    color: root.robotConnected ? AppTheme.badges.primary : AppTheme.badges.textDisabled
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
                color: AppTheme.badges.primary
                opacity: 0.35
                visible: root.robotConnected
            }

            Rectangle {
                id: changeUserSegment
                width: parent.width
                height: root.segmentHeight
                color: changeUserArea.pressed ? AppTheme.badges.pressedBackground : "transparent"

                Text {
                    id: changeUserText
                    anchors.centerIn: parent
                    text: qsTr("Cambiar usuario")
                    color: AppTheme.badges.primary
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
                color: AppTheme.badges.primary
                opacity: 0.35
            }

            Rectangle {
                id: logoutSegment
                width: parent.width
                height: root.segmentHeight
                color: logoutArea.pressed ? AppTheme.badges.pressedBackground : "transparent"

                Text {
                    id: logoutText
                    anchors.centerIn: parent
                    text: qsTr("Cerrar sesión")
                    color: AppTheme.badges.primary
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

            Rectangle {
                width: parent.width
                height: root.dividerSize
                color: AppTheme.badges.primary
                opacity: 0.35
            }

            Rectangle {
                id: appExitSegment
                width: parent.width
                height: root.segmentHeight
                color: appExitArea.pressed ? AppTheme.badges.pressedBackground : "transparent"

                Text {
                    id: appExitText
                    anchors.centerIn: parent
                    text: qsTr("Cerrar aplicación")
                    color: AppTheme.badges.primary
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: appExitArea
                    anchors.fill: parent
                    onPressed: root.restartAutoCloseTimer()
                    onClicked: {
                        root.dropDownOpen = false
                        root.appExitRequested()
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
