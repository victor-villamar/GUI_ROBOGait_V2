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
    readonly property bool isTrajectoryStep: step === 2
    readonly property bool isExperimentStep: step === 3
    readonly property bool isNavigationStep: isTrajectoryStep
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
    signal pathEditToggleRequested()
    signal pathClearRequested()
    signal tracedRoutesRequested()
    signal startTestRequested()
    signal goHomeRequested()
    signal fitRequested()
    signal followRequested()
    signal finishExperimentRequested()
    signal repeatExperimentRequested()
    signal exitExperimentRequested()

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
    property bool tracedRoutesAvailable: false
    property bool personDetectionConfirmed: false
    property bool pathTerminalPoseSet: false
    property bool pathTerminalOrientationOverride: false
    property bool testStarted: false
    property bool experimentHomeVisible: false
    property bool experimentRepeatVisible: false
    property bool experimentExitMode: false
    property bool followRobotChecked: false
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
                                                mapVisualizationManager.splinePathEditor &&
                                                mapVisualizationManager.splinePathEditor.hasPath
    property var pathEditControlPoints: []
    property real pathAnchorHandleSizePx: 14
    property real pathTangentHandleSizePx: 10
    property real pathHandleHitPaddingPx: 12
    property bool pathControlDragActive: false
    property int pathControlDragType: -1
    property int pathControlDragIndex: -1
    property real pathControlDragOverlayX: 0
    property real pathControlDragOverlayY: 0
    readonly property bool pathEditHandlesVisible: mapAvailable &&
                                                   isNavigationStep &&
                                                   pathPlacementEnabled &&
                                                   mapVisualizationManager &&
                                                   mapVisualizationManager.splinePathEditor &&
                                                   mapVisualizationManager.splinePathEditor.isSmoothed &&
                                                   mapVisualizationManager.splinePathEditor.isEditMode

    function refreshPathEditControlPoints(forceRefresh) {
        if (pathControlDragActive && !forceRefresh) {
            return
        }

        if (!pathEditHandlesVisible ||
            !mapVisualizationManager ||
            !mapVisualizationManager.splinePathEditor ||
            !mapVisualizationManager.splinePathEditor.getControlPoints ||
            !mapVisualizationManager.mapToScreen) {
            pathEditControlPoints = []
            return
        }

        var controls = mapVisualizationManager.splinePathEditor.getControlPoints()
        if (!controls || controls.length === 0) {
            pathEditControlPoints = []
            return
        }

        var projected = []
        for (var i = 0; i < controls.length; ++i) {
            var cp = controls[i]
            var screenPoint = mapVisualizationManager.mapToScreen(cp.x, cp.y)
            if (!screenPoint || !screenPoint.available) {
                continue
            }

            projected.push({
                              "type": cp.type,
                              "index": cp.index,
                              "mapX": cp.x,
                              "mapY": cp.y,
                              "screenX": screenPoint.x,
                              "screenY": screenPoint.y
                          })
        }

        pathEditControlPoints = projected
    }

    function beginSplineControlPointDrag(controlType, controlIndex) {
        pathControlDragActive = true
        pathControlDragType = controlType
        pathControlDragIndex = controlIndex
    }

    function dragSplineControlPoint(overlayX, overlayY) {
        if (!pathControlDragActive) {
            return
        }

        pathControlDragOverlayX = overlayX
        pathControlDragOverlayY = overlayY
        handleSplineControlPointDrag(pathControlDragType,
                                     pathControlDragIndex,
                                     overlayX,
                                     overlayY)
    }

    function endSplineControlPointDrag() {
        if (!pathControlDragActive) {
            return
        }

        pathControlDragActive = false
        pathControlDragType = -1
        pathControlDragIndex = -1
        pathControlDragOverlayX = 0
        pathControlDragOverlayY = 0
        refreshPathEditControlPoints(true)
    }

    function handleSplineControlPointDrag(controlType, controlIndex, overlayX, overlayY) {
        if (!mapVisualizationManager ||
            !mapVisualizationManager.splinePathEditor ||
            !mapVisualizationManager.splinePathEditor.moveControlPoint ||
            !mapVisualizationManager.screenToMap) {
            return
        }

        var mapPoint = mapVisualizationManager.screenToMap(overlayX, overlayY)
        if (!mapPoint || !mapPoint.available) {
            return
        }

        mapVisualizationManager.splinePathEditor.moveControlPoint(controlType, controlIndex, mapPoint.x, mapPoint.y)
    }

    onPathEditHandlesVisibleChanged: {
        refreshPathEditControlPoints(true)
    }

    Connections {
        target: root.mapVisualizationManager && root.mapVisualizationManager.splinePathEditor
                ? root.mapVisualizationManager.splinePathEditor
                : null
        ignoreUnknownSignals: true

        function onPathChanged() {
            root.refreshPathEditControlPoints()
        }

        function onEditModeChanged() {
            root.refreshPathEditControlPoints(true)
        }

        function onSmoothedChanged() {
            root.refreshPathEditControlPoints(true)
        }
    }

    Connections {
        target: root.mapVisualizationManager ? root.mapVisualizationManager : null
        ignoreUnknownSignals: true

        function onZoomLevelChanged() {
            root.refreshPathEditControlPoints()
        }

        function onViewTransformChanged() {
            root.refreshPathEditControlPoints()
        }
    }

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
                             : (isTrajectoryStep
                                ? qsTr("Cálculo de trayectoria del robot")
                                : qsTr("Ejecución del experimento")))
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
                splinePathEditor: (root.mapVisualizationManager && root.mapVisualizationManager.splinePathEditor)
                                  ? root.mapVisualizationManager.splinePathEditor
                                  : null

                onCalculateRequested: root.pathAcceptRequested()
                onSmoothRequested: root.pathSegmentRequested()
                onEditToggleRequested: root.pathEditToggleRequested()
                onClearRequested: root.pathClearRequested()
            }

            MapLayerItem {
                id: mapLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable
                z: 1
                isPanningEnabled: !placementEnabled &&
                                  (!isTrajectoryStep || !goalPlacementEnabled) &&
                                  (!isTrajectoryStep || !pathPlacementEnabled || (manualPathAvailable && !pathDragHandler.active)) &&
                                  !root.pathEditHandlesVisible

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
                visible: mapAvailable && (isTrajectoryStep || isExperimentStep)
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
                visible: mapAvailable && (isTrajectoryStep || isExperimentStep)
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
                visible: mapAvailable && (isTrajectoryStep || isExperimentStep)
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

            Item {
                id: splineEditOverlay
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: root.pathEditHandlesVisible
                z: 1.47

                Repeater {
                    model: root.pathEditControlPoints

                    delegate: Rectangle {
                        id: controlPointHandle

                        required property var modelData
                        readonly property int controlType: modelData.type
                        readonly property int controlIndex: modelData.index
                        readonly property bool isAnchor: controlType === 0
                        readonly property bool isDraggedControlPoint: root.pathControlDragActive &&
                                                                     (root.pathControlDragType === controlType) &&
                                                                     (root.pathControlDragIndex === controlIndex)
                        readonly property real handleSize: isAnchor ? root.pathAnchorHandleSizePx : root.pathTangentHandleSizePx
                        readonly property real handleScreenX: isDraggedControlPoint ? root.pathControlDragOverlayX : modelData.screenX
                        readonly property real handleScreenY: isDraggedControlPoint ? root.pathControlDragOverlayY : modelData.screenY

                        width: handleSize
                        height: handleSize
                        radius: width * 0.5
                        color: isAnchor ? AppTheme.map.pathColorSecondary : AppTheme.map.pathColorPrimary
                        border.color: AppTheme.map.white
                        border.width: 1
                        x: handleScreenX - (width * 0.5)
                        y: handleScreenY - (height * 0.5)
                        visible: root.pathEditHandlesVisible

                        MouseArea {
                            id: dragMouseArea
                            anchors.fill: parent
                            anchors.margins: -root.pathHandleHitPaddingPx
                            acceptedButtons: Qt.LeftButton
                            preventStealing: true
                            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                            onPressed: function(mouse) {
                                var overlayPointPressed = dragMouseArea.mapToItem(splineEditOverlay, mouse.x, mouse.y)
                                root.beginSplineControlPointDrag(controlPointHandle.controlType, controlPointHandle.controlIndex)
                                root.dragSplineControlPoint(overlayPointPressed.x, overlayPointPressed.y)
                            }

                            onPositionChanged: function(mouse) {
                                if (!(mouse.buttons & Qt.LeftButton)) {
                                    return
                                }

                                var overlayPoint = dragMouseArea.mapToItem(splineEditOverlay, mouse.x, mouse.y)
                                root.dragSplineControlPoint(overlayPoint.x, overlayPoint.y)
                            }

                            onReleased: {
                                root.endSplineControlPointDrag()
                            }

                            onCanceled: {
                                root.endSplineControlPointDrag()
                            }
                        }
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
            visible: isPositionStep || isOrientationStep

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
            id: trajectoryBar
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(60, root.iconButtonSizePx + 16)
            color: AppTheme.map.panel
            radius: 6
            visible: isTrajectoryStep
            opacity: mapAvailable ? 1.0 : 0.5
            enabled: mapAvailable

            TrajectoryBottomBar {
                anchors.fill: parent
                anchors.margins: 8
                buttonHeightPx: root.buttonHeightPx
                iconButtonSizePx: root.iconButtonSizePx
                iconGlyphSizePx: root.iconGlyphSizePx
                isTrajectoryStep: root.isTrajectoryStep
                mapAvailable: root.mapAvailable
                goalPlacementEnabled: root.goalPlacementEnabled
                pathPlacementEnabled: root.pathPlacementEnabled
                goalAccepted: root.goalAccepted
                goalPathReady: root.goalPathReady
                manualPathReady: root.manualPathReady
                tracedRoutesAvailable: root.tracedRoutesAvailable
                personDetectionConfirmed: root.personDetectionConfirmed

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
                onTracedRoutesRequested: root.tracedRoutesRequested()
            }
        }

        Rectangle {
            id: experimentBar
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(60, root.iconButtonSizePx + 16)
            color: AppTheme.map.panel
            radius: 6
            visible: isExperimentStep
            opacity: mapAvailable ? 1.0 : 0.5
            enabled: mapAvailable

            ExperimentBottomBar {
                anchors.fill: parent
                anchors.margins: 8
                buttonHeightPx: root.buttonHeightPx
                iconButtonSizePx: root.iconButtonSizePx
                iconGlyphSizePx: root.iconGlyphSizePx
                mapAvailable: root.mapAvailable
                followChecked: root.followRobotChecked
                homeVisible: root.experimentHomeVisible
                repeatVisible: root.experimentRepeatVisible
                exitMode: root.experimentExitMode

                onZoomOutRequested: root.zoomOutRequested()
                onZoomInRequested: root.zoomInRequested()
                onFitRequested: root.fitRequested()
                onFollowRequested: root.followRequested()
                onGoHomeRequested: root.goHomeRequested()
                onRepeatRequested: root.repeatExperimentRequested()
                onFinishRequested: root.finishExperimentRequested()
                onExitRequested: root.exitExperimentRequested()
            }
        }

    }

    InformationDialog {
        id: infoDialog
        message: ""
    }
}
