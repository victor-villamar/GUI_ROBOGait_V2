import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

import "qrc:/Dialogs"

Item {
    id: root

    property string robotDisplayName: userSession ? userSession.robotDisplayName : ""
    property bool robotConnected: userSession ? userSession.hasRobotAssigned : false
    property var robotManager: (userSession && userSession.rosManager) ? userSession.rosManager.robotManager : null

    property bool detailsOpen: false
    readonly property bool canConfigure: userSession
                                               && userSession.isAuthenticated
                                               && (userSession.role === "doctor" || userSession.role === "manager")

    readonly property real badgeHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.badgeHeight, 0) : 56
    readonly property real badgeMenuItemHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.badgeMenuItemHeight, 0) : 44
    readonly property real iconGlyphPxRaw: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 34
    readonly property real badgeIconPx: Math.min(iconGlyphPxRaw, badgeHeightPx - 16)

    property int badgeWidth: Math.max(robotLabelRow.implicitWidth + 20, 140)
    property int segmentHeight: Math.round(badgeMenuItemHeightPx)
    property int segmentPadding: 16
    property int dividerSize: 2
    property int segmentCount: canConfigure ? 2 : 1
    property int dividerCount: segmentCount - 1
    property int menuWidth: Math.max(badgeWidth,
                                     informationText.implicitWidth + segmentPadding * 2,
                                     configurationText.implicitWidth + segmentPadding * 2)

    implicitHeight: Math.round(badgeHeightPx)
    implicitWidth: badgeWidth

    Timer {
        id: autoCloseTimer
        interval: timeoutSettings ? timeoutSettings.robotBadgeAutoCloseMs : 6000
        repeat: false
        onTriggered: root.detailsOpen = false
    }

    function restartAutoCloseTimer()
    {
        autoCloseTimer.stop()
        if (root.detailsOpen) {
            autoCloseTimer.start()
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.detailsOpen
        onClicked: root.detailsOpen = false
        z: 1
    }

    Rectangle {
        id: robotLabel
        z: 3
        height: root.implicitHeight
        width: root.badgeWidth
        radius: Math.round(height * 0.32)
        color: AppTheme.badges.badgeBackground
        border.color: AppTheme.badges.light
        border.width: 2

        anchors.top: parent.top
        anchors.right: parent.right

        Row {
            id: robotLabelRow
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 10
            spacing: 14

            Item {
                width: root.badgeIconPx
                height: root.badgeIconPx

                Image {
                    anchors.centerIn: parent
                    width: root.badgeIconPx
                    height: root.badgeIconPx
                    source: (root.robotManager && root.robotManager.batteryIcon !== "")
                            ? root.robotManager.batteryIcon
                            : "qrc:/qmlresources/robot/robot.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }

                Text {
                    anchors.centerIn: parent
                    text: (root.robotManager && root.robotManager.batteryLevelTrunc >= 0)
                          ? (root.robotManager.batteryLevelTrunc + "%")
                          : ""
                    color: robotNameText.color
                    font.pixelSize: Math.max(10, Math.round(root.badgeIconPx * 0.32))
                    font.bold: true
                }
            }

            Text {
                id: robotNameText
                text: root.robotDisplayName
                color: AppTheme.badges.primary
                font.pixelSize: 18
                font.bold: true
                elide: Text.ElideRight
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.detailsOpen = !root.detailsOpen
                root.restartAutoCloseTimer()
            }
        }
    }

    Rectangle {
        id: detailsPanel
        z: 3
        width: root.menuWidth
        height: root.detailsOpen
              ? (root.segmentHeight * root.segmentCount + root.dividerSize * root.dividerCount)
              : 0
        opacity: root.detailsOpen ? 1 : 0
        radius: 14
        color: AppTheme.badges.light
        border.color: AppTheme.badges.primary
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

        Column {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                width: parent.width
                height: root.segmentHeight
                color: informationArea.pressed ? AppTheme.badges.pressedBackground : "transparent"

                Text {
                    id: informationText
                    anchors.centerIn: parent
                    text: qsTr("Información")
                    color: AppTheme.badges.primary
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: informationArea
                    anchors.fill: parent
                    onPressed: root.restartAutoCloseTimer()
                    onClicked: {
                        root.detailsOpen = false
                        robotInformationDialog.open()
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: root.dividerSize
                color: AppTheme.badges.primary
                opacity: 0.35
                visible: root.canConfigure
            }

            Rectangle {
                width: parent.width
                height: root.segmentHeight
                visible: root.canConfigure
                color: configurationArea.pressed ? AppTheme.badges.pressedBackground : "transparent"

                Text {
                    id: configurationText
                    anchors.centerIn: parent
                    text: qsTr("Configuración")
                    color: AppTheme.badges.primary
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: configurationArea
                    anchors.fill: parent
                    enabled: root.canConfigure
                    onPressed: root.restartAutoCloseTimer()
                    onClicked: {
                        root.detailsOpen = false
                        robotSettingsDialog.openForCurrentSettings()
                    }
                }
            }
        }
    }

    RobotInformationDialog {
        id: robotInformationDialog
        robotManager: root.robotManager
        robotDisplayName: root.robotDisplayName
    }

    RobotSettingsDialog {
        id: robotSettingsDialog
    }
}
