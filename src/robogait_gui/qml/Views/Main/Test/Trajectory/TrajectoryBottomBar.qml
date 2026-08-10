import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

Item {
    id: root

    property real buttonHeightPx: 44
    property real iconButtonSizePx: 50
    property real iconGlyphSizePx: 25
    readonly property real textButtonPaddingPx: Math.max(24, iconButtonSizePx * 0.45)
    readonly property real textButtonHeightPx: Math.max(buttonHeightPx, iconButtonSizePx)

    property bool isTrajectoryStep: false
    property bool mapAvailable: false
    property bool goalPlacementEnabled: false
    property bool pathPlacementEnabled: false
    property bool goalAccepted: false
    property bool goalPathReady: false
    property bool manualPathReady: false
    property bool tracedRoutesAvailable: false
    property bool personDetectionConfirmed: false

    signal backRequested()
    signal zoomOutRequested()
    signal zoomInRequested()
    signal goalModeRequested()
    signal pathModeRequested()
    signal tracedRoutesRequested()
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
                source: "qrc:/qmlresources/icons/white/minus.png"
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
                source: "qrc:/qmlresources/icons/white/plus.png"
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
                source: "qrc:/qmlresources/icons/white/goal.png"
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
                source: "qrc:/qmlresources/icons/white/path.png"
                width: root.iconGlyphSizePx
                height: root.iconGlyphSizePx
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: root.pathModeRequested()
        }
    }

    Row {
        id: rightControls
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Button {
            id: tracedRoutesButton
            width: Math.max(190, tracedRoutesLabel.implicitWidth + root.textButtonPaddingPx * 2)
            height: root.textButtonHeightPx
            visible: root.tracedRoutesAvailable
            enabled: root.mapAvailable && root.isTrajectoryStep
            opacity: enabled ? 1.0 : 0.4

            background: Rectangle {
                radius: 6
                color: tracedRoutesButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Text {
                id: tracedRoutesLabel
                text: qsTr("VER RUTAS TRAZADAS")
                color: AppTheme.map.white
                font.pixelSize: 13
                minimumPixelSize: 10
                fontSizeMode: Text.Fit
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            onClicked: root.tracedRoutesRequested()
        }

        Button {
            id: startTestButton
            width: Math.max(170, startTestLabel.implicitWidth + root.textButtonPaddingPx * 2)
            height: root.textButtonHeightPx
            visible: !root.personDetectionConfirmed && ((root.goalPlacementEnabled && root.goalAccepted) || (root.pathPlacementEnabled && root.manualPathReady))
            enabled: root.goalPlacementEnabled ? root.goalPathReady : (root.pathPlacementEnabled ? root.manualPathReady : false)
            opacity: enabled ? 1.0 : 0.4

            background: Rectangle {
                radius: 6
                color: startTestButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Text {
                id: startTestLabel
                text: qsTr("DETECTAR PERSONA")
                color: AppTheme.map.white
                font.pixelSize: 14
                minimumPixelSize: 10
                fontSizeMode: Text.Fit
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            onClicked: root.startTestRequested()
        }
    }
}
