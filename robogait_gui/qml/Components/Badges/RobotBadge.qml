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
    property int detailsCount: (robotManager && robotManager.robotStatusItems) ? robotManager.robotStatusItems.length : 0

    readonly property real badgeHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.badgeHeight, 0) : 56
    readonly property real badgeMenuItemHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.badgeMenuItemHeight, 0) : 44
    readonly property real iconGlyphPxRaw: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 34
    readonly property real badgeIconPx: Math.min(iconGlyphPxRaw, badgeHeightPx - 16)

    property int badgeWidth: Math.max(robotLabelRow.implicitWidth + 20, 140)
    property int segmentHeight: Math.round(badgeMenuItemHeightPx)
    property int segmentPadding: 16
    property int dividerSize: 2
    property int dividerCount: Math.max(0, detailsCount - 1)
    property int menuWidth: badgeWidth

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
              ? (root.segmentHeight * root.detailsCount + root.dividerSize * root.dividerCount)
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

            Repeater {
                model: root.robotManager ? root.robotManager.robotStatusItems : []
                delegate: Item {
                    width: parent.width
                    height: root.segmentHeight

                    Row {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        Text {
                            id: labelText
                            text: modelData.label + ":"
                            color: AppTheme.badges.primary
                            font.pixelSize: 12
                            font.bold: true
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: modelData.value
                            color: AppTheme.badges.primary
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            anchors.verticalCenter: parent.verticalCenter
                            width: Math.max(0, parent.width - labelText.implicitWidth - parent.spacing)
                        }
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: root.dividerSize
                        color: AppTheme.badges.primary
                        opacity: 0.35
                        visible: index < (root.detailsCount - 1)
                    }
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onPressed: root.restartAutoCloseTimer()
        }
    }
}
