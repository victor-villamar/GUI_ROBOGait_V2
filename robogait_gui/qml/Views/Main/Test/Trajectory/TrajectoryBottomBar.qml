import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

Item {
    id: root

    property real buttonHeightPx: 44
    property real iconButtonSizePx: 50
    property real iconGlyphSizePx: 25

    property bool isTrajectoryStep: false
    property bool mapAvailable: false
    property bool goalPlacementEnabled: false
    property bool pathPlacementEnabled: false
    property bool goalAccepted: false
    property bool goalPathReady: false
    property bool manualPathReady: false
    property bool personDetectionConfirmed: false

    signal backRequested()
    signal zoomOutRequested()
    signal zoomInRequested()
    signal goalModeRequested()
    signal pathModeRequested()
    signal startTestRequested()

    Row {
        id: leftControls
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Button {
            id: backButton
            width: 140
            height: root.iconButtonSizePx
            enabled: root.isTrajectoryStep
            opacity: enabled ? 1.0 : 0.4

            background: Rectangle {
                radius: 6
                color: backButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Text {
                text: qsTr("ATRÁS")
                color: AppTheme.map.white
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.backRequested()
        }

        Button {
            id: zoomOutButton
            width: root.iconButtonSizePx
            height: root.iconButtonSizePx
            enabled: root.mapAvailable && root.isTrajectoryStep
            opacity: enabled ? 1.0 : 0.5

            background: Rectangle {
                radius: 6
                color: zoomOutButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/white/minus.svg"
                width: root.iconGlyphSizePx
                height: root.iconGlyphSizePx
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: root.zoomOutRequested()
        }

        Button {
            id: zoomInButton
            width: root.iconButtonSizePx
            height: root.iconButtonSizePx
            enabled: root.mapAvailable && root.isTrajectoryStep
            opacity: enabled ? 1.0 : 0.5

            background: Rectangle {
                radius: 6
                color: zoomInButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/white/plus.svg"
                width: root.iconGlyphSizePx
                height: root.iconGlyphSizePx
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: root.zoomInRequested()
        }
    }

    Row {
        id: trajectoryModeButtons
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12
        z: 10

        Button {
            id: goalModeButton
            width: root.iconButtonSizePx
            height: root.iconButtonSizePx
            enabled: root.mapAvailable && root.isTrajectoryStep
            opacity: enabled ? 1.0 : 0.5

            background: Rectangle {
                radius: 6
                color: root.goalPlacementEnabled ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/white/goal.svg"
                width: root.iconGlyphSizePx
                height: root.iconGlyphSizePx
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: root.goalModeRequested()
        }

        Button {
            id: pathModeButton
            width: root.iconButtonSizePx
            height: root.iconButtonSizePx
            enabled: root.mapAvailable && root.isTrajectoryStep
            opacity: enabled ? 1.0 : 0.5

            background: Rectangle {
                radius: 6
                color: root.pathPlacementEnabled ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/white/path.svg"
                width: root.iconGlyphSizePx
                height: root.iconGlyphSizePx
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: root.pathModeRequested()
        }
    }

    Button {
        id: startTestButton
        width: 150
        height: root.iconButtonSizePx
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        visible: ((root.goalPlacementEnabled && root.goalAccepted) || (root.pathPlacementEnabled && root.manualPathReady))
        enabled: root.goalPlacementEnabled ? root.goalPathReady : (root.pathPlacementEnabled ? root.manualPathReady : false)
        opacity: enabled ? 1.0 : 0.4

        background: Rectangle {
            radius: 6
            color: startTestButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
            border.color: AppTheme.map.white
            border.width: 1
        }

        contentItem: Text {
            text: root.personDetectionConfirmed ? qsTr("INICIAR TEST") : qsTr("DETECTAR PERSONA")
            color: AppTheme.map.white
            font.pixelSize: 14
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        onClicked: root.startTestRequested()
    }
}
