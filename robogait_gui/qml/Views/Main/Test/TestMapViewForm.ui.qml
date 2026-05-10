import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0
import QtQuick.Layouts 1.15
import MapRendering 1.0


import "qrc:/Components"
import "qrc:/Dialogs"
import "qrc:/Views"

Rectangle {
    id: root
    color: AppTheme.map.appBackground

    property alias infoButton: infoButton
    property alias emergencyButton: emergencyButton
    property alias infoDialog: infoDialog
    property alias mapLayerItem: mapLayerItem
    property alias robotLayerItem: robotLayerItem
    property alias pathLayerItem: pathLayerItem
    property alias livePathLayerItem: livePathLayerItem

    property int step: 0
    readonly property bool isPositionStep: step === 0
    readonly property bool isOrientationStep: step === 1
    readonly property bool isNavigationStep: step === 2
    property real iconButtonSizePx: 0
    property real iconGlyphSizePx: 0
    property real headerTopInsetPx: 0
    property real buttonHeightPx: 0
    property real wheelSizePx: 0
    property real emergencyButtonSizePx: iconButtonSizePx * 2
    property real emergencyIconSizePx: emergencyButtonSizePx * 0.6
    property real headerButtonSpacing: 8

    signal confirmPlacementRequested()
    signal confirmOrientationRequested()
    signal backOrientationRequested()
    signal backNavigationRequested()
    signal zoomOutRequested()
    signal zoomInRequested()
    signal autoLocalizationRequested()
    signal goalAcceptRequested()
    signal goalClearRequested()
    signal pathAcceptRequested()
    signal pathSegmentRequested()
    signal pathClearRequested()
    signal startTestRequested()
    signal goHomeRequested()

    property bool mapAvailable: false
    property bool particleCloudAvailable: false
    property int mapContentMargin: 10
    property bool placementEnabled: false
    property bool orientationEnabled: false
    property bool orientationOverride: false
    property bool goalPlacementEnabled: false
    property bool pathPlacementEnabled: false
    property bool goalPointSet: false
    property bool goalOrientationEnabled: false
    property bool goalOrientationSet: false
    property bool goalAccepted: false
    property bool goalPathReady: false
    property bool manualPathReady: false
    property bool pathTerminalPoseSet: false
    property bool pathTerminalOrientationOverride: false
    property bool testStarted: false
    property real goalOrientationDeg: 0
    property var goalMapPosition: Qt.point(0, 0)
    property real pathTerminalOrientationDeg: 0
    property var pathTerminalMapPosition: Qt.point(0, 0)
    property var placementController: (userSession.rosManager && userSession.rosManager.robotManager)
                                      ? userSession.rosManager.robotManager.robotPlacementController
                                      : null
    property bool showRobotPose: false
    property bool showParticleCloud: false
    readonly property bool manualPathAvailable: mapVisualizationManager &&
                                                mapVisualizationManager.manualPathEditor &&
                                                mapVisualizationManager.manualPathEditor.hasPath


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        anchors.topMargin: 20 + headerTopInsetPx
        spacing: 15

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(60, root.iconButtonSizePx + 20)
            color: AppTheme.map.panel
            radius: 8

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                Item {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: root.iconButtonSizePx
                }

                HelpButton {
                    id: infoButton
                    Layout.preferredWidth: root.iconButtonSizePx
                    Layout.preferredHeight: root.iconButtonSizePx
                    Layout.alignment: Qt.AlignVCenter
                    glyphSize: root.iconGlyphSizePx
                }

                Text {
                    Layout.fillWidth: true
                    text: isPositionStep
                          ? qsTr("Posicionamiento del robot en el mapa: Posición")
                          : (isOrientationStep
                             ? qsTr("Posicionamiento del robot en el mapa: Orientación")
                             : qsTr("Cálculo de trayectoria del robot"))
                    font.pixelSize: 24
                    font.bold: true
                    color: AppTheme.map.white
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Item {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: root.iconButtonSizePx
                }
            }
        }

        Rectangle {
            id: mapDisplayArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: AppTheme.map.panelHeader
            radius: 8
            border.color: AppTheme.map.panel
            border.width: 2
            clip: true

            RobotLayerItem {
                id: robotLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: showRobotPose
                z: 2
                bodyColor: AppTheme.map.robotBody
                wheelColor: AppTheme.map.robotWheel
                headColor: AppTheme.map.robotHead

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerRobotLayerItem(robotLayerItem)
                    }
                }
            }




            Button {
                id: emergencyButton
                width: root.emergencyButtonSizePx > 0 ? root.emergencyButtonSizePx : root.iconButtonSizePx * 2
                height: width
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 21
                anchors.topMargin: 21
                padding: 0
                checkable: true
                z: 80

                background: Rectangle {
                    radius: width / 2
                    color: emergencyButton.checked ? AppTheme.map.emergencyStopLatchedBg : "transparent"
                    border.color: emergencyButton.checked ? AppTheme.map.emergencyStopLatchedBorder : "transparent"
                    border.width: emergencyButton.checked ? 2 : 0
                }

                contentItem: Image {
                    source: "qrc:/qmlresources/icons/color/emergency_stop.svg"
                    width: root.emergencyIconSizePx > 0 ? root.emergencyIconSizePx : root.iconGlyphSizePx * 2
                    height: width
                    anchors.centerIn: parent
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: emergencyButton.checked ? 0.6 : 1.0
                }
            }

            TrajectoryGoalActionPanel {
                buttonHeightPx: root.buttonHeightPx
                mapAvailable: root.mapAvailable
                isNavigationStep: root.isNavigationStep
                goalPlacementEnabled: root.goalPlacementEnabled
                testStarted: root.testStarted
                goalPointSet: root.goalPointSet
                goalOrientationSet: root.goalOrientationSet

                onCalculateRequested: root.goalAcceptRequested()
                onClearRequested: root.goalClearRequested()
            }

            TrajectoryPathActionPanel {
                buttonHeightPx: root.buttonHeightPx
                mapAvailable: root.mapAvailable
                isNavigationStep: root.isNavigationStep
                pathPlacementEnabled: root.pathPlacementEnabled
                testStarted: root.testStarted
                manualPathEditor: (root.mapVisualizationManager && root.mapVisualizationManager.manualPathEditor)
                                  ? root.mapVisualizationManager.manualPathEditor
                                  : null

                onCalculateRequested: root.pathAcceptRequested()
                onSmoothRequested: root.pathSegmentRequested()
                onClearRequested: root.pathClearRequested()
            }

            MapLayerItem {
                id: mapLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable
                z: 1
                isPanningEnabled: !placementEnabled &&
                                  !goalPlacementEnabled &&
                                  !testStarted &&
                                  (!pathPlacementEnabled || (manualPathAvailable && !pathDragHandler.active))

                PlacementTapHandler {
                    mapAvailable: root.mapAvailable
                    isPositionStep: root.isPositionStep
                    placementEnabled: root.placementEnabled
                    testStarted: root.testStarted
                    placementController: root.placementController
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    gesturePolicy: TapHandler.DragThreshold
                    onTapped: {
                        if (testStarted) {
                            return
                        }
                        if (mapAvailable && isNavigationStep && pathPlacementEnabled) {
                            return
                        }
                        if (mapAvailable && isNavigationStep && goalPlacementEnabled) {
                            if (root.handleGoalTap) {
                                root.handleGoalTap(point.position.x, point.position.y)
                            }
                            return
                        }
                    }
                }

                DragHandler {
                    id: pathDragHandler
                    target: null
                    acceptedButtons: Qt.LeftButton
                    dragThreshold: 0
                    grabPermissions: PointerHandler.TakeOverForbidden
                    enabled: mapAvailable &&
                             isNavigationStep &&
                             pathPlacementEnabled &&
                             !testStarted &&
                             (!manualPathAvailable || active)

                    onActiveChanged: {
                        if (active) {
                            if (root.handlePathStrokeStart) {
                                root.handlePathStrokeStart(centroid.position.x, centroid.position.y)
                            }
                        }
                        else {
                            if (root.handlePathStrokeEnd) {
                                root.handlePathStrokeEnd()
                            }
                        }
                    }

                    onCentroidChanged: {
                        if (!active) {
                            return
                        }
                        if (root.handlePathStrokeMove) {
                            root.handlePathStrokeMove(centroid.position.x, centroid.position.y)
                        }
                    }

                    onCanceled: function() {
                        if (root.handlePathStrokeEnd) {
                            root.handlePathStrokeEnd()
                        }
                    }
                }

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerMapLayerItem(mapLayerItem)
                    }
                }
            }

            RobotLayerItem {
                id: goalRobotLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable && isNavigationStep && !testStarted &&
                         ((goalPlacementEnabled && goalPointSet) ||
                          (pathPlacementEnabled && pathTerminalPoseSet))
                z: 2
                headColor: AppTheme.map.pathHeadBlue
                bodyColor: AppTheme.map.robotBody
                wheelColor: AppTheme.map.robotWheel

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerGoalRobotLayerItem(goalRobotLayerItem)
                    }
                }
            }

            PathLayerItem {
                id: pathLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable && isNavigationStep
                z: 1.45
                pathColor: AppTheme.map.pathColorPrimary

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerPathLayerItem(pathLayerItem)
                    }
                }
            }

            PathLayerItem {
                id: manualDrawPathLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable && isNavigationStep
                z: 1.455
                pathColor: AppTheme.map.pathColorSecondary

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerManualDrawPathLayerItem(manualDrawPathLayerItem)
                    }
                }
            }

            PathLayerItem {
                id: livePathLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable && isNavigationStep
                z: 1.46
                pathColor: AppTheme.map.pathColorTertiary

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerLivePathLayerItem(livePathLayerItem)
                    }
                }
            }

            ParticleCloudLayerItem {
                id: particleCloudLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable && particleCloudAvailable && showParticleCloud
                z: 1.4
                particleColor: AppTheme.map.particle

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerParticleCloudLayerItem(particleCloudLayerItem)
                    }
                }
            }

            PlacementOrientationPanel {
                mapAvailable: root.mapAvailable
                isOrientationStep: root.isOrientationStep
                orientationEnabled: root.orientationEnabled
                wheelSizePx: root.wheelSizePx
                placementController: root.placementController
            }

            TrajectoryGoalOrientationPanel {
                mapAvailable: root.mapAvailable
                isNavigationStep: root.isNavigationStep
                goalPlacementEnabled: root.goalPlacementEnabled
                goalPointSet: root.goalPointSet
                goalAccepted: root.goalAccepted
                testStarted: root.testStarted
                goalOrientationEnabled: root.goalOrientationEnabled
                wheelSizePx: root.wheelSizePx
                inputAngleDeg: root.goalOrientationDeg

                onOrientationChanged: function(deg) {
                    root.goalOrientationDeg = deg
                    root.goalOrientationSet = true
                }
            }

            TrajectoryPathOrientationPanel {
                mapAvailable: root.mapAvailable
                isNavigationStep: root.isNavigationStep
                pathPlacementEnabled: root.pathPlacementEnabled
                pathTerminalPoseSet: root.pathTerminalPoseSet
                testStarted: root.testStarted
                wheelSizePx: root.wheelSizePx
                inputAngleDeg: root.pathTerminalOrientationDeg

                onOrientationChanged: function(deg) {
                    root.pathTerminalOrientationDeg = deg
                    root.pathTerminalOrientationOverride = true
                }
            }

            Rectangle {
                id: emptyMapPlaceholder
                anchors.fill: parent
                anchors.margins: mapContentMargin
                color: AppTheme.map.panelHeader
                border.color: AppTheme.map.panel
                visible: !mapAvailable
                z: 0
            }

            Text {
                anchors.centerIn: parent
                text: qsTr("Esperando mapa...")
                color: AppTheme.map.white
                opacity: 0.7
                font.pixelSize: 16
                visible: !mapAvailable
            }
        }

        Rectangle {
            id: bottomBar
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(60, root.buttonHeightPx + 16)
            color: AppTheme.map.panel
            radius: 6
            visible: !isNavigationStep

            Item {
                id: bottomBarContent
                anchors.fill: parent
                anchors.margins: 8

                PlacementBottomBar {
                    anchors.fill: parent
                    buttonHeightPx: root.buttonHeightPx
                    isPositionStep: root.isPositionStep
                    isOrientationStep: root.isOrientationStep
                    placementEnabled: root.placementEnabled
                    orientationEnabled: root.orientationEnabled
                    placementController: root.placementController

                    onTogglePlacementRequested: root.placementEnabled = !root.placementEnabled

                    onClearPlacementRequested: {
                        if (root.placementController) {
                            root.placementController.clear()
                        }
                    }

                    onAutoLocalizationRequested: root.autoLocalizationRequested()

                    onToggleOrientationRequested: {
                        if (root.orientationEnabled) {
                            root.orientationEnabled = false
                            return
                        }

                        root.orientationEnabled = true
                        root.orientationOverride = true
                        if (root.placementController) {
                            root.placementController.setOrientationDegrees(0)
                        }
                    }

                    onBackOrientationRequested: root.backOrientationRequested()

                    onConfirmPositionRequested: root.confirmPlacementRequested()
                    onConfirmOrientationRequested: root.confirmOrientationRequested()
                }
            }
        }

        Rectangle {
            id: navigationBar
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(60, root.iconButtonSizePx + 16)
            color: AppTheme.map.panel
            radius: 6
            visible: isNavigationStep
            opacity: mapAvailable ? 1.0 : 0.5
            enabled: mapAvailable

            TrajectoryBottomBar {
                anchors.fill: parent
                anchors.margins: 8
                buttonHeightPx: root.buttonHeightPx
                iconButtonSizePx: root.iconButtonSizePx
                iconGlyphSizePx: root.iconGlyphSizePx
                isNavigationStep: root.isNavigationStep
                mapAvailable: root.mapAvailable
                testStarted: root.testStarted
                goalPlacementEnabled: root.goalPlacementEnabled
                pathPlacementEnabled: root.pathPlacementEnabled
                goalAccepted: root.goalAccepted
                goalPathReady: root.goalPathReady
                manualPathReady: root.manualPathReady

                onBackRequested: root.backNavigationRequested()
                onZoomOutRequested: root.zoomOutRequested()
                onZoomInRequested: root.zoomInRequested()

                onGoalModeRequested: {
                    if (root.goalPlacementEnabled) {
                        root.goalPlacementEnabled = false
                    }
                    else {
                        root.goalPlacementEnabled = true
                        root.pathPlacementEnabled = false
                    }
                }

                onPathModeRequested: {
                    if (root.pathPlacementEnabled) {
                        root.pathPlacementEnabled = false
                    }
                    else {
                        root.pathPlacementEnabled = true
                        root.goalPlacementEnabled = false
                    }
                }

                onStartTestRequested: root.startTestRequested()
                onGoHomeRequested: root.goHomeRequested()
            }
        }

    }

    InformationDialog {
        id: infoDialog
        message: ""
    }
}
