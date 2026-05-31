import QtQuick 2.15
import QtQuick.Controls 2.15
import RobotServiceBridge 1.0

import "qrc:/Components"
import "qrc:/Dialogs"
import "qrc:/Views"

TestMapViewForm {

    id: root
    readonly property real computedIconButtonSizePx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.iconButtonSize, 0) : 50
    readonly property real computedIconGlyphSizePx: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 25
    readonly property real computedHeaderTopInsetPx: uiSizingSettings ? uiSizingSettings.px(3.0, 0) : 12
    readonly property real computedButtonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 44
    readonly property real computedJoystickStickSizePx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.joystickStickSize, 0) : 34
    readonly property real computedWheelSizePx: computedJoystickStickSizePx * 4
    readonly property int busyTimeoutMs: timeoutSettings ? timeoutSettings.testBusyTimeoutMs : 10000

    iconButtonSizePx: computedIconButtonSizePx
    iconGlyphSizePx: computedIconGlyphSizePx
    headerTopInsetPx: computedHeaderTopInsetPx
    buttonHeightPx: computedButtonHeightPx
    wheelSizePx: computedWheelSizePx

    property bool exiting: false
    property bool exitAndQuit: false
    property bool emergencyLatched: false

    signal appExitFinished()

    property bool waitingForNavigationStart: false

    property bool autoLocalizationActive: false
    property bool autoLocalizationWaitingForNav: false
    property bool autoLocalizationWaitingForService: false
    property bool autoLocalizationCompleted: false
    property bool autoLocalizationServiceDone: false
    property bool autoLocalizationSpinDone: false
    property int autoLocalizationSpinMs: timeoutSettings ? timeoutSettings.autoLocalizationSpinTimeoutMs : 30000
    property real autoLocalizationAngularSpeed: 0.75
    property bool initialRobotPoseSaved: false
    property var initialRobotMapPosition: Qt.point(0, 0)
    property real initialRobotTheta: 0
    property bool experimentNavigationSucceeded: false
    property bool experimentNavigationFinished: false
    property bool experimentShowRepeatButton: false
    property bool experimentReturnHomeForRepeatPending: false
    property bool experimentSaveFlowStarted: false
    property bool experimentSavePromptPending: false
    property bool waitingGoalPathResult: false
    property bool waitingHomePathResult: false
    property bool waitingManualPathResult: false
    property var manualPathNavigationPoints: []
    property bool personDetectionInProgress: false

    readonly property var manualControl: (userSession && userSession.rosManager && userSession.rosManager.robotManager)
                                            ? userSession.rosManager.robotManager.manualControl
                                            : null

    readonly property var mapVisualizationManager: (userSession && userSession.rosManager && userSession.rosManager.robotManager)
                                                   ? userSession.rosManager.robotManager.mapVisualizationManager
                                                   : null
    readonly property var splinePathEditor: (mapVisualizationManager && mapVisualizationManager.splinePathEditor)
                                            ? mapVisualizationManager.splinePathEditor
                                            : null

    readonly property int stepPosition: 0
    readonly property int stepOrientation: 1
    readonly property int stepTrajectory: 2
    readonly property int stepExperiment: 3
    readonly property int stepNavigation: stepTrajectory

    step: stepPosition

    readonly property var robotServiceBridge : (userSession && userSession.rosManager && userSession.rosManager.robotManager)
                                                  ? userSession.rosManager.robotManager.robotServiceBridge
                                                  : null
    readonly property var personDetectionMonitor: (userSession && userSession.rosManager && userSession.rosManager.robotManager)
                                                  ? userSession.rosManager.robotManager.personDetectionMonitor
                                                  : null

    mapAvailable: (userSession.rosManager &&
                   userSession.rosManager.robotManager &&
                   userSession.rosManager.robotManager.mapVisualizationManager)
                  ? userSession.rosManager.robotManager.mapVisualizationManager.mapAvailable
                  : false

    particleCloudAvailable: (userSession.rosManager &&
                             userSession.rosManager.robotManager &&
                             userSession.rosManager.robotManager.mapVisualizationManager)
                            ? userSession.rosManager.robotManager.mapVisualizationManager.particleCloudAvailable
                            : false

    showParticleCloud: step === stepPosition

    showRobotPose: mapAvailable && ((placementController && placementController.hasPosition) || autoLocalizationCompleted || autoLocalizationActive)
    followRobotChecked: mapVisualizationManager && mapVisualizationManager.followRobot !== undefined
                        ? mapVisualizationManager.followRobot
                        : false
    experimentHomeVisible: step === stepExperiment &&
                           initialRobotPoseSaved &&
                           experimentNavigationSucceeded &&
                           !experimentReturnHomeForRepeatPending
    experimentRepeatVisible: step === stepExperiment && experimentShowRepeatButton
    experimentExitMode: step === stepExperiment && experimentNavigationFinished
    personDetectionConfirmed: false

    HelpContentProvider {
        id: helpContent
    }

    function buildInfoDialogMessageForCurrentStep() {
        if (step === stepPosition) {
            return helpContent.testMapPositionMessage()
        }
        if (step === stepOrientation) {
            return helpContent.testMapOrientationMessage()
        }
        if (step === stepTrajectory) {
            return helpContent.testMapTrajectoryMessage()
        }
        if (step === stepExperiment) {
            return helpContent.testMapExperimentMessage()
        }
        return helpContent.defaultStepMessage()
    }

    function resetExperimentPhaseState() {
        experimentNavigationSucceeded = false
        experimentNavigationFinished = false
        experimentShowRepeatButton = false
        experimentReturnHomeForRepeatPending = false
        experimentSaveFlowStarted = false
        experimentSavePromptPending = false
        if (experimentSavePromptTimer.running) {
            experimentSavePromptTimer.stop()
        }
        if (experimentFinishBusyDialog.visible) {
            experimentFinishBusyDialog.close()
        }
    }

    function startExperimentSaveFlowWithDelay(messageText) {
        if (experimentSaveFlowStarted) {
            return
        }

        experimentSaveFlowStarted = true
        experimentSavePromptPending = true
        experimentFinishBusyDialog.openWithMessage(messageText)
        experimentSavePromptTimer.restart()
    }

    function resetManualPathFlow() {
        waitingManualPathResult = false
        manualPathReady = false
        manualPathNavigationPoints = []
        pathTerminalPoseSet = false
        pathTerminalOrientationOverride = false
        pathTerminalOrientationDeg = 0
        pathTerminalMapPosition = Qt.point(0, 0)

        if (mapVisualizationManager && mapVisualizationManager.clearGoalRobotPose) {
            mapVisualizationManager.clearGoalRobotPose()
        }
        personDetectionConfirmed = false
        personDetectionInProgress = false
        if (personDetectionMonitor && personDetectionMonitor.stopMonitoring) {
            personDetectionMonitor.stopMonitoring()
        }
    }

    function clearPathsAfterExperimentEnd() {
        waitingGoalPathResult = false
        waitingHomePathResult = false
        waitingManualPathResult = false
        goalPathReady = false
        manualPathReady = false
        manualPathNavigationPoints = []

        if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
            mapVisualizationManager.setPathUpdatesEnabled(false)
        }
        if (mapVisualizationManager && mapVisualizationManager.clearManualPath) {
            mapVisualizationManager.clearManualPath()
        }
        if (splinePathEditor && splinePathEditor.clear) {
            splinePathEditor.clear()
        }
        if (mapVisualizationManager && mapVisualizationManager.clearGoalRobotPose) {
            mapVisualizationManager.clearGoalRobotPose()
        }
        personDetectionInProgress = false
        if (personDetectionMonitor && personDetectionMonitor.stopMonitoring) {
            personDetectionMonitor.stopMonitoring()
        }
    }

    function beginPersonDetection() {
        if (!personDetectionMonitor || !personDetectionMonitor.startMonitoring) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: Monitor de detección no disponible")
            errorPopup.open()
            return
        }

        if (personDetectionInProgress) {
            return
        }

        var okMonitoring = personDetectionMonitor.startMonitoring()
        if (!okMonitoring) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la detección de persona")
            errorPopup.open()
            return
        }

        personDetectionInProgress = true
        busyDialog.openWithMessage(qsTr("Detectando persona..."))
    }

    function stopPersonDetection() {
        personDetectionInProgress = false
        if (personDetectionMonitor && personDetectionMonitor.stopMonitoring) {
            personDetectionMonitor.stopMonitoring()
        }
        if (busyDialog.visible) {
            busyDialog.close()
        }
    }

    function syncPathTerminalGoalRobotPose() {
        if (!pathPlacementEnabled || !pathTerminalPoseSet) {
            return
        }

        if (!mapVisualizationManager || !mapVisualizationManager.setGoalRobotPose) {
            return
        }

        mapVisualizationManager.setGoalRobotPose(pathTerminalMapPosition.x,
                                                 pathTerminalMapPosition.y,
                                                 pathTerminalOrientationDeg * Math.PI / 180)
    }

    function updatePathTerminalPoseFromPoints(points, preserveManualOrientation) {
        if (!points || points.length < 2) {
            pathTerminalPoseSet = false
            if (mapVisualizationManager && mapVisualizationManager.clearGoalRobotPose) {
                mapVisualizationManager.clearGoalRobotPose()
            }
            return false
        }

        var lastPoint = points[points.length - 1]
        pathTerminalMapPosition = Qt.point(lastPoint.x, lastPoint.y)
        pathTerminalPoseSet = true

        if (!(preserveManualOrientation && pathTerminalOrientationOverride)) {
            var prevPoint = points[points.length - 2]
            var dx = lastPoint.x - prevPoint.x
            var dy = lastPoint.y - prevPoint.y
            if (Math.abs(dx) > 1e-9 || Math.abs(dy) > 1e-9) {
                pathTerminalOrientationDeg = Math.atan2(dy, dx) * 180 / Math.PI
            } else {
                pathTerminalOrientationDeg = 0
            }
            pathTerminalOrientationOverride = false
        }

        syncPathTerminalGoalRobotPose()
        return true
    }

    function updatePathTerminalPoseFromEditor(preserveManualOrientation) {
        if (splinePathEditor && splinePathEditor.getPathPoints) {
            var splinePoints = splinePathEditor.getPathPoints()
            return updatePathTerminalPoseFromPoints(splinePoints, preserveManualOrientation)
        }

        return false
    }

    function buildManualPathWaypointsForService(points) {
        var waypoints = []
        if (!points || points.length < 2) {
            return waypoints
        }

        for (var i = 0; i < points.length; ++i) {
            var waypoint = { "x": points[i].x, "y": points[i].y }
            if (i === points.length - 1) {
                waypoint["theta"] = pathTerminalOrientationDeg * Math.PI / 180
            }
            waypoints.push(waypoint)
        }
        return waypoints
    }

    function clearGoalSelection() {
        goalPointSet = false
        goalOrientationEnabled = false
        goalOrientationSet = false
        goalAccepted = false
        goalPathReady = false
        testStarted = false
        initialRobotPoseSaved = false
        initialRobotMapPosition = Qt.point(0, 0)
        initialRobotTheta = 0
        resetExperimentPhaseState()
        waitingGoalPathResult = false
        waitingHomePathResult = false
        goalOrientationDeg = 0
        goalMapPosition = Qt.point(0, 0)
        resetManualPathFlow()
    }

    function saveInitialRobotPose() {
        initialRobotPoseSaved = false

        if (!mapVisualizationManager || !mapVisualizationManager.getRobotPose) {
            return false
        }

        var pose = mapVisualizationManager.getRobotPose()
        if (!pose || !pose.available) {
            return false
        }

        initialRobotMapPosition = Qt.point(pose.x, pose.y)
        initialRobotTheta = pose.theta
        initialRobotPoseSaved = true
        return true
    }

    function handleGoalTap(screenX, screenY) {
        if (!mapVisualizationManager || !mapAvailable || !goalPlacementEnabled) {
            return
        }
        if (goalPointSet) {
            return
        }

        var mapPoint = mapVisualizationManager.screenToMap(screenX, screenY)
        if (!mapPoint || !mapPoint.available) {
            return
        }

        goalMapPosition = Qt.point(mapPoint.x, mapPoint.y)
        goalPointSet = true
        goalAccepted = false
        goalOrientationEnabled = true

        var pose = mapVisualizationManager.getRobotPose()
        if (pose && pose.available) {
            goalOrientationDeg = pose.theta * 180 / Math.PI
        } else {
            goalOrientationDeg = 0
        }

        goalOrientationSet = true

        if (mapVisualizationManager.setGoalRobotPose) {
            mapVisualizationManager.setGoalRobotPose(goalMapPosition.x, goalMapPosition.y, goalOrientationDeg * Math.PI / 180)
        }
    }

    function handleGoalClear() {
        clearGoalSelection()
        goalPathReady = false
        if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
            mapVisualizationManager.setPathUpdatesEnabled(false)
        }
        if (mapVisualizationManager && mapVisualizationManager.clearManualPath) {
            mapVisualizationManager.clearManualPath()
        }

        if (mapVisualizationManager && mapVisualizationManager.clearGoalRobotPose) {
            mapVisualizationManager.clearGoalRobotPose()
        }
    }

    function handleGoalAccept() {
        if (!goalPointSet || !goalOrientationSet) {
            return
        }
        goalPathReady = false
        waitingGoalPathResult = true
        waitingHomePathResult = false
        if (robotServiceBridge && robotServiceBridge.computePathToPose) {
            var okCompute = robotServiceBridge.computePathToPose(goalMapPosition.x, goalMapPosition.y, goalOrientationDeg * Math.PI / 180)
            if (!okCompute) {
                waitingGoalPathResult = false
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo calcular la ruta")
                errorPopup.open()
                return
            }
        } else {
            waitingGoalPathResult = false
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
            errorPopup.open()
            return
        }
        goalAccepted = true
        goalOrientationEnabled = false
    }

    function handlePathClear() {
        resetManualPathFlow()
        if (mapVisualizationManager && mapVisualizationManager.clearManualPath) {
            mapVisualizationManager.clearManualPath()
        }

        if (splinePathEditor && splinePathEditor.clear) {
            splinePathEditor.clear()
        }
    }

    function handlePathSegment() {
        if (!splinePathEditor || !splinePathEditor.smoothPath) {
            return
        }

        var smoothed = splinePathEditor.smoothPath()
        if (smoothed) {
            if (splinePathEditor.setEditModeEnabled) {
                splinePathEditor.setEditModeEnabled(false)
            }
            updatePathTerminalPoseFromEditor(true)
        }
    }

    function handlePathEditToggle() {
        if (!splinePathEditor || !splinePathEditor.setEditModeEnabled) {
            return
        }

        if (!splinePathEditor.isSmoothed) {
            return
        }

        splinePathEditor.setEditModeEnabled(!splinePathEditor.isEditMode)
        updatePathTerminalPoseFromEditor(true)
    }

    function handlePathAccept() {
        if (!splinePathEditor || !splinePathEditor.getPathPointsForCompute || !splinePathEditor.hasEditablePath) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: Suaviza y edita el path antes de aceptar")
            errorPopup.open()
            return
        }

        if (!robotServiceBridge || !robotServiceBridge.computePathThroughPoses) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
            errorPopup.open()
            return
        }

        var points = splinePathEditor.getPathPointsForCompute()
        if (!points || points.length < 2) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: Path suavizado inválido")
            errorPopup.open()
            return
        }

        if (!updatePathTerminalPoseFromPoints(points, true)) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo obtener la orientación final")
            errorPopup.open()
            return
        }

        manualPathReady = false
        manualPathNavigationPoints = buildManualPathWaypointsForService(points)
        if (!manualPathNavigationPoints || manualPathNavigationPoints.length < 2) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: Path suavizado inválido")
            errorPopup.open()
            return
        }
        waitingManualPathResult = true
        waitingGoalPathResult = false
        waitingHomePathResult = false

        if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
            mapVisualizationManager.setPathUpdatesEnabled(false)
        }

        var okCompute = robotServiceBridge.computePathThroughPoses(manualPathNavigationPoints)
        if (!okCompute) {
            resetManualPathFlow()
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo calcular la ruta del path")
            errorPopup.open()
        }
    }

    function handlePathStrokeStart(screenX, screenY) {
        if (!mapAvailable || !pathPlacementEnabled || !splinePathEditor || !splinePathEditor.beginStrokeFromScreen) {
            return
        }

        splinePathEditor.beginStrokeFromScreen(screenX, screenY)
    }

    function handlePathStrokeMove(screenX, screenY) {
        if (!mapAvailable || !pathPlacementEnabled || !splinePathEditor || !splinePathEditor.appendPointFromScreen) {
            return
        }

        splinePathEditor.appendPointFromScreen(screenX, screenY)
    }

    function handlePathStrokeEnd() {
        if (!splinePathEditor || !splinePathEditor.endStroke) {
            return
        }

        splinePathEditor.endStroke()
        updatePathTerminalPoseFromEditor(false)
    }

    onStepChanged: {
        if (step === stepPosition) {
            resetParticleCloudData()
        }
        if (step !== stepPosition) {
            placementEnabled = false
            syncRobotPoseUpdates()
        }

        if (step !== stepOrientation) {
            orientationEnabled = false
            orientationOverride = false
        }

        if (step !== stepTrajectory && step !== stepExperiment) {
            goalPlacementEnabled = false
            pathPlacementEnabled = false
            clearGoalSelection()
            goalPathReady = false
            resetManualPathFlow()
            resetExperimentPhaseState()
            personDetectionConfirmed = false
            stopPersonDetection()
            if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
                mapVisualizationManager.setPathUpdatesEnabled(false)
            }
            if (mapVisualizationManager && mapVisualizationManager.clearManualPath) {
                mapVisualizationManager.clearManualPath()
            }
            if (mapVisualizationManager && mapVisualizationManager.clearGoalRobotPose) {
                mapVisualizationManager.clearGoalRobotPose()
            }
        }

    }

    onPlacementEnabledChanged: {
        if (placementEnabled) {
            autoLocalizationCompleted = false
            autoLocalizationActive = false
            autoLocalizationWaitingForNav = false
            autoLocalizationWaitingForService = false
            autoLocalizationServiceDone = false
            autoLocalizationSpinDone = false
            orientationEnabled = false
            orientationOverride = false

            if (placementController) {
                placementController.clear()
            }
        }

    }

    onGoalPlacementEnabledChanged: {
        if (goalPlacementEnabled) {
            resetManualPathFlow()
            if (pathPlacementEnabled) {
                pathPlacementEnabled = false
            }
            goalPathReady = false
            if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
                mapVisualizationManager.setPathUpdatesEnabled(false)
            }
            if (mapVisualizationManager && mapVisualizationManager.clearManualPath) {
                mapVisualizationManager.clearManualPath()
            }
            if (goalPointSet) {
                goalAccepted = false
                goalOrientationEnabled = true
                if (mapVisualizationManager && mapVisualizationManager.setGoalRobotPose) {
                    mapVisualizationManager.setGoalRobotPose(goalMapPosition.x, goalMapPosition.y, goalOrientationDeg * Math.PI / 180)
                }
            }
            else {
                clearGoalSelection()
                if (mapVisualizationManager && mapVisualizationManager.clearGoalRobotPose) {
                    mapVisualizationManager.clearGoalRobotPose()
                }
            }
            if (mapVisualizationManager && mapVisualizationManager.followRobot !== undefined) {
                mapVisualizationManager.followRobot = false
            }
        }
    }

    onPathPlacementEnabledChanged: {
        if (pathPlacementEnabled) {
            if (goalPlacementEnabled) {
                goalPlacementEnabled = false
            }
            goalPathReady = false
            manualPathReady = false
            if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
                mapVisualizationManager.setPathUpdatesEnabled(false)
            }
            updatePathTerminalPoseFromEditor(false)
            if (mapVisualizationManager && mapVisualizationManager.followRobot !== undefined) {
                mapVisualizationManager.followRobot = false
            }
        }
        else {
            handlePathClear()
        }
    }

    onGoalOrientationDegChanged: {
        if (!goalPointSet || !goalPlacementEnabled || goalAccepted) {
            return
        }
        if (mapVisualizationManager && mapVisualizationManager.setGoalRobotPose) {
            mapVisualizationManager.setGoalRobotPose(goalMapPosition.x, goalMapPosition.y, goalOrientationDeg * Math.PI / 180)
        }
    }

    onPathTerminalOrientationDegChanged: {
        syncPathTerminalGoalRobotPose()
    }

    onPathTerminalMapPositionChanged: {
        syncPathTerminalGoalRobotPose()
    }

    function syncRobotPoseUpdates() {
        var mapVizManager = userSession && userSession.rosManager && userSession.rosManager.robotManager
                           ? userSession.rosManager.robotManager.mapVisualizationManager
                           : null
        if (!mapVizManager) {
            return
        }

        var enableUpdates = (step === stepTrajectory) || (step === stepExperiment) || autoLocalizationActive
        mapVizManager.setRobotPoseUpdatesEnabled(enableUpdates)
    }

    function resetParticleCloudData() {
        var mapVizManager = userSession && userSession.rosManager && userSession.rosManager.robotManager
                           ? userSession.rosManager.robotManager.mapVisualizationManager
                           : null
        if (!mapVizManager) {
            return
        }

        mapVizManager.resetParticleCloud()
    }

    function startAutoLocalizationSpin() {
        if (!manualControl || !userSession || !userSession.rosManager || !userSession.rosManager.robotManager) {
            return
        }

        userSession.rosManager.robotManager.enableManualControl()
        autoLocalizationSpinTimer.start()
        autoLocalizationStopTimer.start()
    }

    function stopAutoLocalizationSpin() {
        if (autoLocalizationSpinTimer.running) {
            autoLocalizationSpinTimer.stop()
        }

        if (autoLocalizationStopTimer.running) {
            autoLocalizationStopTimer.stop()
        }

        if (manualControl) {
            manualControl.stopRobot()
        }

        if (userSession && userSession.rosManager && userSession.rosManager.robotManager) {
            userSession.rosManager.robotManager.disableManualControl()
        }
    }

    function failAutoLocalization(messageText) {
        autoLocalizationActive = false
        autoLocalizationWaitingForNav = false
        autoLocalizationWaitingForService = false
        autoLocalizationCompleted = false
        autoLocalizationServiceDone = false
        autoLocalizationSpinDone = false
        stopAutoLocalizationSpin()
        busyDialog.close()
        syncRobotPoseUpdates()

        if (messageText) {
            errorPopup.errorRectangleTextError.text = messageText
            errorPopup.open()
        }
    }

    function emergencyStop() {
        stopAutoLocalizationSpin()

        waitingForNavigationStart = false
        autoLocalizationActive = false
        autoLocalizationWaitingForNav = false
        autoLocalizationWaitingForService = false
        autoLocalizationCompleted = false
        autoLocalizationServiceDone = false
        autoLocalizationSpinDone = false
        syncRobotPoseUpdates()

        if (autoLocalizationConfirmDialog.visible) {
            autoLocalizationConfirmDialog.close()
        }

        if (busyDialog.visible) {
            busyDialog.close()
        }

        if (manualControl) {
            manualControl.updateVelocity(0.0, 0.0)
            manualControl.stopRobot()
        }
    }

    function publishEmergencyZero() {
        if (manualControl) {
            manualControl.updateVelocity(0.0, 0.0)
        }
    }

    function setEmergencyLatched(active) {
        emergencyLatched = active
        if (emergencyButton.checked !== active) {
            emergencyButton.checked = active
        }
        if (active) {
            emergencyStop()
            publishEmergencyZero()
            if (testStarted && robotServiceBridge && robotServiceBridge.cancelNavigateToPose) {
                robotServiceBridge.cancelNavigateToPose()
            }
        }
        else {
            if (manualControl) {
                manualControl.updateVelocity(0.0, 0.0)
                manualControl.stopPublishing()
            }
        }
    }

    function saveAutoLocalizationPose() {
        if (!placementController) {
            return false
        }

        var mapVizManager = userSession && userSession.rosManager && userSession.rosManager.robotManager
                           ? userSession.rosManager.robotManager.mapVisualizationManager
                           : null
        if (!mapVizManager) {
            return false
        }

        var pose = mapVizManager.getRobotPose()
        if (!pose || !pose.available) {
            return false
        }

        placementController.position = Qt.point(pose.x, pose.y)
        placementController.theta = pose.theta
        return true
    }

    function finalizeAutoLocalization() {
        autoLocalizationActive = false
        autoLocalizationWaitingForService = false
        autoLocalizationWaitingForNav = false

        if (!saveAutoLocalizationPose()) {
            failAutoLocalization(qsTr("Error: No se pudo obtener la posición"))
            return
        }

        autoLocalizationCompleted = true
        placementEnabled = false
        syncRobotPoseUpdates()
    }

    function triggerGlobalLocalization() {
        if (!robotServiceBridge) {
            failAutoLocalization(qsTr("Error: No hay conexión con el robot"))
            return
        }

        autoLocalizationWaitingForNav = false
        autoLocalizationWaitingForService = true

        var okService = robotServiceBridge.reinitializeGlobalLocalization()
        if (!okService) {
            autoLocalizationWaitingForService = false
            failAutoLocalization(qsTr("Error: No se pudo iniciar la autolocalización"))
            return
        }

        startAutoLocalizationSpin()
    }

    function beginAutoLocalization() {
        if (autoLocalizationActive) {
            return
        }

        if (!robotServiceBridge) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
            errorPopup.open()
            return
        }

        var mapName = (userSession.currentMapName || "").trim()
        if (mapName === "") {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay mapa seleccionado")
            errorPopup.open()
            return
        }

        autoLocalizationCompleted = false
        autoLocalizationActive = true
        syncRobotPoseUpdates()
        autoLocalizationWaitingForNav = false
        autoLocalizationWaitingForService = false
        autoLocalizationServiceDone = false
        autoLocalizationSpinDone = false
        busyDialog.openWithMessage(qsTr("Iniciando autolocalización..."))

        if (robotServiceBridge.activeCommandKey === "navigation") {
            if (robotServiceBridge.status === RobotServiceBridge.RUNNING) {
                triggerGlobalLocalization()
                return
            }

            if (robotServiceBridge.status === RobotServiceBridge.STARTING) {
                autoLocalizationWaitingForNav = true
                return
            }
        }

        autoLocalizationWaitingForNav = true
        var okStart = robotServiceBridge.startNavigation(mapName)
        if (!okStart) {
            autoLocalizationWaitingForNav = false
            failAutoLocalization(qsTr("Error: No se pudo iniciar la navegación"))
            return
        }
    }

    function handleBackNavigation() {
        exitConfirmDialog.openWithMessage(qsTr("¿Salir de la prueba? Se perderá la configuración."))
        return true
    }

    function resetToTrajectoryPhase() {
        if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
            mapVisualizationManager.setPathUpdatesEnabled(false)
        }
        if (mapVisualizationManager && mapVisualizationManager.followRobot !== undefined) {
            mapVisualizationManager.followRobot = false
        }

        testStarted = false
        resetExperimentPhaseState()

        goalPlacementEnabled = false
        pathPlacementEnabled = false
        handleGoalClear()
        handlePathClear()
        stopPersonDetection()
        personDetectionConfirmed = false

        step = stepTrajectory
    }

    function beginExit() {
        if (exiting) {
            return
        }

        exiting = true
        busyDialog.openWithMessage(qsTr("Saliendo..."))

        if (robotServiceBridge && robotServiceBridge.activeCommandKey === "navigation") {
            var okStop = robotServiceBridge.stopNavigation()
            if (!okStop) {
                busyDialog.close()
                exiting = false
                exitAndQuit = false
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo detener la navegación")
                errorPopup.open()
                return
            }
        }

        maybeFinalizeExit()
    }

    function maybeFinalizeExit() {
        if (!exiting) {
            return
        }

        if (!robotServiceBridge) {
            finalizeExit()
            return
        }

        if (robotServiceBridge.activeCommandKey !== "navigation") {
            finalizeExit()
            return
        }

        if (robotServiceBridge.status === RobotServiceBridge.STOPPED
            || robotServiceBridge.status === RobotServiceBridge.ERROR
            || robotServiceBridge.status === RobotServiceBridge.IDLE) {
            finalizeExit()
        }
    }

    function finalizeExit() {
        exiting = false
        busyDialog.close()
        if (experimentSavePromptTimer.running) {
            experimentSavePromptTimer.stop()
        }
        experimentSavePromptPending = false
        if (experimentFinishBusyDialog.visible) {
            experimentFinishBusyDialog.close()
        }

        if (manualControl) {
            manualControl.updateVelocity(0.0, 0.0)
            manualControl.stopRobot()
        }

        if (placementController) {
            placementController.clear()
        }

        // Ensure navigation visuals are reset when leaving the test view.
        waitingGoalPathResult = false
        waitingHomePathResult = false
        waitingManualPathResult = false
        goalPathReady = false
        goalAccepted = false
        goalPointSet = false
        goalOrientationEnabled = false
        goalOrientationSet = false
        resetManualPathFlow()

        if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
            mapVisualizationManager.setPathUpdatesEnabled(false)
        }
        if (mapVisualizationManager && mapVisualizationManager.clearManualPath) {
            mapVisualizationManager.clearManualPath()
        }
        if (mapVisualizationManager && mapVisualizationManager.clearGoalRobotPose) {
            mapVisualizationManager.clearGoalRobotPose()
        }
        if (splinePathEditor && splinePathEditor.clear) {
            splinePathEditor.clear()
        }

        if (userSession && userSession.rosManager && userSession.rosManager.robotManager) {
            var mapVizManager = userSession.rosManager.robotManager.mapVisualizationManager
            if (mapVizManager) {
                mapVizManager.destroySubscriptions()
            }
        }

        if (exitAndQuit) {
            exitAndQuit = false
            appExitFinished()
            return
        }

        if (StackView.view) {
            while (StackView.view.depth > 3) {
                StackView.view.pop()
            }
        }
    }

    function requestAppExit() {
        exitAndQuit = true
        beginExit()
        return true
    }

    function handleUserSwitch() {
        stopAutoLocalizationSpin()
        autoLocalizationActive = false
        autoLocalizationWaitingForNav = false
        autoLocalizationWaitingForService = false
        autoLocalizationServiceDone = false
        autoLocalizationSpinDone = false
        waitingForNavigationStart = false

        if (busyDialog.visible) {
            busyDialog.close()
        }

        beginExit()
    }

    Component.onCompleted: {
        busyDialog.openWithMessage(qsTr("Cargando mapa..."))

        var mapVizManager = userSession && userSession.rosManager && userSession.rosManager.robotManager
                           ? userSession.rosManager.robotManager.mapVisualizationManager
                           : null

        if (mapVizManager) {
            mapVizManager.activateSubscriptions()
            syncRobotPoseUpdates()
            if (step === stepPosition) {
                resetParticleCloudData()
            }
        }

        if (!robotServiceBridge) {
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
            errorPopup.open()
            return
        }

        resetExperimentPhaseState()

        var mapName = (userSession.currentMapName || "").trim()

        var okMap = robotServiceBridge.requestMapData(mapName)
        if (!okMap) {
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo cargar el mapa")
            errorPopup.open()
            return
        }

        if (mapAvailable) {
            busyDialog.close()
        }
    }

    onMapAvailableChanged: {
        if (mapAvailable) {
            busyDialog.close()
        }
    }

    onAutoLocalizationRequested: {
        if (step !== stepPosition) {
            return
        }

        autoLocalizationConfirmDialog.openWithMessage(
                    qsTr("Antes de autolocalizarse, asegúrese de que el robot esté en un espacio libre de obstáculos. Durante la autolocalización el robot realizará movimientos."))
    }


    onConfirmPlacementRequested: {
        if (step !== stepPosition) {
            return
        }

        if (autoLocalizationActive) {
            return
        }

        if (autoLocalizationCompleted) {
            placementEnabled = false
        syncRobotPoseUpdates()
            orientationEnabled = false
            orientationOverride = false
            step = stepOrientation
            return
        }

        if (!placementController || !placementController.hasPosition) {
            return
        }

        autoLocalizationCompleted = false

        if (!robotServiceBridge) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
            errorPopup.open()
            return
        }

        var mapName = (userSession.currentMapName || "").trim()
        if (mapName === "") {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay mapa seleccionado")
            errorPopup.open()
            return
        }

        if (robotServiceBridge.activeCommandKey === "navigation") {
            if (robotServiceBridge.status === RobotServiceBridge.RUNNING) {
                placementEnabled = false
        syncRobotPoseUpdates()
                orientationEnabled = false
            orientationOverride = false
                step = stepOrientation
                return
            }

            if (robotServiceBridge.status === RobotServiceBridge.STARTING) {
                waitingForNavigationStart = true
                busyDialog.openWithMessage(qsTr("Guardando posición..."))
                return
            }
        }

        waitingForNavigationStart = true
        busyDialog.openWithMessage(qsTr("Guardando posición..."))

        var okStart = robotServiceBridge.startNavigation(mapName)
        if (!okStart) {
            waitingForNavigationStart = false
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la navegación")
            errorPopup.open()
            return
        }
    }

    onConfirmOrientationRequested: {
        if (step !== stepOrientation) {
            return
        }

        if (!placementController) {
            return
        }

        if (!placementController.hasOrientation) {
            return
        }

        if (orientationOverride) {
            var rm = (userSession && userSession.rosManager) ? userSession.rosManager.robotManager : null
            if (!rm) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
                errorPopup.open()
                return
            }

            var pos = placementController.position
            rm.publishInitialPose(pos.x, pos.y, placementController.theta)
        }

        step = stepTrajectory
    }

    onGoalAcceptRequested: {
        if (step !== stepTrajectory) {
            return
        }
        handleGoalAccept()
    }

    onGoalClearRequested: {
        if (step !== stepTrajectory) {
            return
        }
        handleGoalClear()
    }

    onPathClearRequested: {
        if (step !== stepTrajectory) {
            return
        }
        if (pathPlacementEnabled) {
            handlePathClear()
        }
    }

    onPathAcceptRequested: {
        if (step !== stepTrajectory) {
            return
        }
        handlePathAccept()
    }

    onPathSegmentRequested: {
        if (step !== stepTrajectory) {
            return
        }
        handlePathSegment()
    }

    onPathEditToggleRequested: {
        if (step !== stepTrajectory) {
            return
        }
        handlePathEditToggle()
    }

    onStartTestRequested: {
        if (step !== stepTrajectory) {
            return
        }

        if (!personDetectionConfirmed) {
            beginPersonDetection()
            return
        }

        if (goalPlacementEnabled) {
            if (!goalPathReady) {
                return
            }

            startTestConfirmDialog.openWithMessage(qsTr("¿Iniciar test con este objetivo?"))
            return
        }

        if (pathPlacementEnabled) {
            if (!manualPathReady) {
                return
            }

            startTestConfirmDialog.openWithMessage(qsTr("¿Iniciar navegación con esta ruta?"))
        }

    }

    onGoHomeRequested: {
        if (step !== stepExperiment || !initialRobotPoseSaved) {
            return
        }
        if (!robotServiceBridge || !robotServiceBridge.computePathToPose) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
            errorPopup.open()
            return
        }

        waitingHomePathResult = true
        waitingGoalPathResult = false

        if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
            mapVisualizationManager.setPathUpdatesEnabled(false)
        }

        handlePathClear()

        var okComputeHome = robotServiceBridge.computePathToPose(initialRobotMapPosition.x, initialRobotMapPosition.y, initialRobotTheta)
        if (!okComputeHome) {
            waitingHomePathResult = false
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo calcular la ruta a la posición inicial")
            errorPopup.open()
            return
        }
    }

    onBackOrientationRequested: {
        if (step !== stepOrientation) {
            return
        }

        if (placementController) {
            placementController.clear()
        }

        orientationEnabled = false
        orientationOverride = false
        autoLocalizationCompleted = false
        step = stepPosition
    }

    onBackNavigationRequested: {
        if (step !== stepTrajectory || testStarted) {
            return
        }

        stopPersonDetection()
        personDetectionConfirmed = false
        goalPlacementEnabled = false
        pathPlacementEnabled = false
        handleGoalClear()
        handlePathClear()
        step = stepOrientation
    }

    onFinishExperimentRequested: {
        if (step !== stepExperiment) {
            return
        }

        finishExperimentConfirmDialog.openWithMessage(qsTr("¿Seguro que quieres terminar el experimento actual?"))
    }

    onRepeatExperimentRequested: {
        if (step !== stepExperiment || experimentReturnHomeForRepeatPending) {
            return
        }

        repeatExperimentConfirmDialog.openWithMessage(qsTr("¿Desea realizar el experimento nuevamente? El robot volverá al punto inicial."))
    }

    onExitExperimentRequested: {
        if (step !== stepExperiment || !experimentNavigationFinished) {
            return
        }

        beginExit()
    }

    onZoomInRequested: {
        if (!mapVisualizationManager || !mapVisualizationManager.zoomIn) {
            return
        }

        if (mapVisualizationManager.followRobot !== undefined) {
            mapVisualizationManager.followRobot = false
        }
        mapVisualizationManager.zoomIn()
    }

    onZoomOutRequested: {
        if (!mapVisualizationManager || !mapVisualizationManager.zoomOut) {
            return
        }

        if (mapVisualizationManager.followRobot !== undefined) {
            mapVisualizationManager.followRobot = false
        }
        mapVisualizationManager.zoomOut()
    }

    onFitRequested: {
        if (!mapVisualizationManager || !mapVisualizationManager.fitToView) {
            return
        }

        if (mapVisualizationManager.followRobot !== undefined) {
            mapVisualizationManager.followRobot = false
        }
        mapVisualizationManager.fitToView()
    }

    onFollowRequested: {
        if (!mapVisualizationManager || mapVisualizationManager.followRobot === undefined) {
            return
        }

        mapVisualizationManager.followRobot = !mapVisualizationManager.followRobot
    }

    Component.onDestruction: {
        stopAutoLocalizationSpin()
        stopPersonDetection()

        if (userSession && userSession.rosManager && userSession.rosManager.robotManager) {
            var mapVizManager = userSession.rosManager.robotManager.mapVisualizationManager
            if (mapVizManager) {
                // Ensure pose updates are re-enabled when leaving the test view
                mapVizManager.setRobotPoseUpdatesEnabled(true)
                mapVizManager.destroySubscriptions()
            }
        }

        if (emergencyLatched) {
            setEmergencyLatched(false)
        }
    }

    infoButton.onClicked: {
        infoDialog.openWithMessage(buildInfoDialogMessageForCurrentStep())
    }

    emergencyButton.onClicked: {
        setEmergencyLatched(emergencyButton.checked)
    }

    ConfirmationDialog {
        id: exitConfirmDialog
        holdToAccept: true
        acceptText: qsTr("Salir")

        onAccepted: {
            beginExit()
        }
    }

    ConfirmationDialog {
        id: finishExperimentConfirmDialog
        holdToAccept: true
        acceptText: qsTr("Terminar")

        onAccepted: {
            if (step !== stepExperiment) {
                return
            }

            if (experimentNavigationFinished) {
                return
            }

            if (testStarted && robotServiceBridge && robotServiceBridge.cancelNavigateToPose) {
                robotServiceBridge.cancelNavigateToPose()
            }

            testStarted = false
            experimentNavigationFinished = true
            experimentShowRepeatButton = true
            experimentNavigationSucceeded = false
            clearPathsAfterExperimentEnd()
            startExperimentSaveFlowWithDelay(qsTr("Terminando test..."))
        }
    }

    ConfirmationDialog {
        id: saveExperimentConfirmDialog
        holdToAccept: false
        acceptText: qsTr("Guardar")

        onAccepted: {
            if (!dbManager || !userSession) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: Sesión o base de datos no disponible")
                errorPopup.open()
                return
            }

            if (!userSession.currentPatient) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: No hay paciente seleccionado")
                errorPopup.open()
                return
            }

            var mapName = (userSession.currentMapName || "").trim()
            if (mapName === "") {
                errorPopup.errorRectangleTextError.text = qsTr("Error: No hay mapa seleccionado")
                errorPopup.open()
                return
            }

            var patientName = ""
            if (userSession.currentPatient) {
                var p = userSession.currentPatient
                patientName = (p.lastName + ", " + p.name).trim()
            }

            experimentSaveDialog.doctorName = dbManager.displayName
            experimentSaveDialog.patientName = patientName
            experimentSaveDialog.locationName = (userSession && userSession.currentMapName) ? userSession.currentMapName : ""
            experimentSaveDialog.dateText = Qt.formatDateTime(new Date(), "dd/MM/yyyy HH:mm")
            experimentSaveDialog.annotations = ""
            experimentSaveDialog.open()
        }

        onRejected: {
        }
    }

    ConfirmationDialog {
        id: repeatExperimentConfirmDialog
        holdToAccept: true
        acceptText: qsTr("Repetir")

        onAccepted: {
            if (step !== stepExperiment || experimentReturnHomeForRepeatPending) {
                return
            }

            if (!robotServiceBridge || !robotServiceBridge.navigateToPose) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
                errorPopup.open()
                return
            }

            if (!initialRobotPoseSaved) {
                resetToTrajectoryPhase()
                return
            }

            if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
                mapVisualizationManager.setPathUpdatesEnabled(true)
            }

            experimentReturnHomeForRepeatPending = true
            var okRepeatHome = robotServiceBridge.navigateToPose(initialRobotMapPosition.x, initialRobotMapPosition.y, initialRobotTheta)
            if (!okRepeatHome) {
                experimentReturnHomeForRepeatPending = false
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo volver al punto inicial")
                errorPopup.open()
            }
        }
    }

    ExperimentSaveDialog {
        id: experimentSaveDialog

        onSaveRequested: function(annotations) {
            if (!dbManager || !userSession || !userSession.currentPatient) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: No hay datos suficientes para guardar")
                errorPopup.open()
                return
            }

            var patient = userSession.currentPatient
            var mapName = (userSession.currentMapName || "").trim()
            var okSave = dbManager.saveExperiment(patient.name, patient.lastName, mapName, annotations)
            if (!okSave) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: %1").arg(dbManager.lastError)
                errorPopup.open()
                return
            }

            experimentSaveDialog.close()
        }
    }


    Connections {
        target: robotServiceBridge
        ignoreUnknownSignals: true

        function onStatusChanged() {
            if (autoLocalizationWaitingForNav)
            {
                var navStarting = (robotServiceBridge.activeCommandKey === "navigation")
                if (navStarting && robotServiceBridge.status === RobotServiceBridge.RUNNING)
                {
                    triggerGlobalLocalization()
                }
                else if (robotServiceBridge.status === RobotServiceBridge.ERROR)
                {
                    autoLocalizationWaitingForNav = false
                    failAutoLocalization(qsTr("Error: No se pudo iniciar la navegación"))
                }
            }

            if (waitingForNavigationStart)
            {
                var navPlacementStarting = (robotServiceBridge.activeCommandKey === "navigation")
                if (navPlacementStarting && robotServiceBridge.status === RobotServiceBridge.RUNNING)
                {
                    waitingForNavigationStart = false
                    busyDialog.close()
                    placementEnabled = false
                    syncRobotPoseUpdates()
                    orientationEnabled = false
                    orientationOverride = false
                    step = stepOrientation
                }
                else if (robotServiceBridge.status === RobotServiceBridge.ERROR)
                {
                    waitingForNavigationStart = false
                    busyDialog.close()
                    errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la navegación")
                    errorPopup.open()
                }
            }

            if (exiting) {
                maybeFinalizeExit()
            }
        }

        function onRequestFinished(success) {
            if (autoLocalizationWaitingForService)
            {
                autoLocalizationWaitingForService = false

                if (!success)
                {
                    failAutoLocalization(qsTr("Error: No se pudo iniciar la autolocalización"))
                    return
                }

                autoLocalizationServiceDone = true
                busyDialog.close()
                if (autoLocalizationSpinDone)
                {
                    finalizeAutoLocalization()
                }
                return
            }

            if (autoLocalizationWaitingForNav && !success)
            {
                autoLocalizationWaitingForNav = false
                failAutoLocalization(qsTr("Error: No se pudo iniciar la navegación"))
                return
            }

            if (success)
            {
                return
            }

            if (waitingForNavigationStart)
            {
                waitingForNavigationStart = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la navegación")
                errorPopup.open()
                return
            }

            if (exiting)
            {
                exiting = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo detener la navegación")
                errorPopup.open()
            }
        }

        function onPathComputed(success, points) {
            if (!mapVisualizationManager || !mapVisualizationManager.setManualPathPoints) {
                return
            }
            var fromHomeRequest = waitingHomePathResult
            var fromGoalRequest = waitingGoalPathResult
            var fromManualRequest = waitingManualPathResult

            waitingHomePathResult = false
            waitingGoalPathResult = false
            waitingManualPathResult = false

            if (!success) {
                if (mapVisualizationManager.clearManualPath) {
                    mapVisualizationManager.clearManualPath()
                }
                if (fromGoalRequest) {
                    goalPathReady = false
                }
                if (fromHomeRequest) {
                    errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo calcular la ruta a la posición inicial")
                    errorPopup.open()
                }
                if (fromManualRequest) {
                    manualPathReady = false
                    manualPathNavigationPoints = []
                    errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo calcular la ruta del path")
                    errorPopup.open()
                }
                return
            }

            mapVisualizationManager.setManualPathPoints(points)

            if (fromManualRequest) {
                manualPathReady = points && points.length > 1
                return
            }

            if (fromGoalRequest) {
                goalPathReady = points && points.length > 1
                return
            }

            if (fromHomeRequest) {
                goHomeConfirmDialog.openWithMessage(qsTr("¿Seguro que quieres volver a la posición inicial?"))
                return
            }

            goalPathReady = points && points.length > 1
        }

        function onNavigationFinished(resultCode) {
            if (step !== stepExperiment) {
                return
            }

            if (experimentReturnHomeForRepeatPending) {
                experimentReturnHomeForRepeatPending = false
                if (resultCode !== RobotServiceBridge.NAV_SUCCEEDED) {
                    errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo volver al punto inicial")
                    errorPopup.open()
                    return
                }

                resetToTrajectoryPhase()
                return
            }

            testStarted = false
            experimentNavigationFinished = true
            experimentNavigationSucceeded = (resultCode === RobotServiceBridge.NAV_SUCCEEDED)
            experimentShowRepeatButton = true
            clearPathsAfterExperimentEnd()

            if (!experimentSaveFlowStarted) {
                startExperimentSaveFlowWithDelay(qsTr("Finalizando test..."))
            }
        }

    }

    Connections {
        target: personDetectionMonitor
        ignoreUnknownSignals: true

        function onPersonDetected(detections) {
            if (step !== stepTrajectory || !personDetectionInProgress) {
                return
            }

            stopPersonDetection()
            personDetectedConfirmDialog.openWithMessage(qsTr("Persona detectada (%1). ¿Desea continuar con el test?").arg(detections))
        }
    }


    ConfirmationDialog {
        id: autoLocalizationConfirmDialog
        acceptText: qsTr("Iniciar")
        holdToAccept: true

        onAccepted: {
            beginAutoLocalization()
        }
    }

    ConfirmationDialog {
        id: startTestConfirmDialog
        acceptText: qsTr("Iniciar")
        holdToAccept: true

        onAccepted: {
            if (step !== stepTrajectory) {
                return
            }

            if (!goalPlacementEnabled && !pathPlacementEnabled) {
                return
            }

            if (!saveInitialRobotPose()) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo guardar la posición inicial del robot")
                errorPopup.open()
                return
            }

            if (goalPlacementEnabled) {
                if (!goalPathReady || !robotServiceBridge || !robotServiceBridge.navigateToPose) {
                    errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
                    errorPopup.open()
                    return
                }

                var okStartGoal = robotServiceBridge.navigateToPose(goalMapPosition.x, goalMapPosition.y, goalOrientationDeg * Math.PI / 180)
                if (!okStartGoal) {
                    errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la navegación")
                    errorPopup.open()
                    return
                }
            }
            else if (pathPlacementEnabled) {
                if (!manualPathReady || !manualPathNavigationPoints || manualPathNavigationPoints.length < 2) {
                    errorPopup.errorRectangleTextError.text = qsTr("Error: Ruta manual no preparada")
                    errorPopup.open()
                    return
                }

                if (!robotServiceBridge || !robotServiceBridge.navigateThroughPoses) {
                    errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
                    errorPopup.open()
                    return
                }

                var okStartPath = robotServiceBridge.navigateThroughPoses(manualPathNavigationPoints)
                if (!okStartPath) {
                    errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la navegación por ruta")
                    errorPopup.open()
                    return
                }
            }

            testStarted = true
            resetExperimentPhaseState()
            step = stepExperiment

            if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
                mapVisualizationManager.setPathUpdatesEnabled(true)
            }
        }

        onRejected: {
        }
    }

    ConfirmationDialog {
        id: personDetectedConfirmDialog
        acceptText: qsTr("Aceptar")
        rejectText: qsTr("Volver a detectar")
        holdToAccept: false

        onAccepted: {
            if (step !== stepTrajectory) {
                return
            }

            personDetectionConfirmed = true
        }

        onRejected: {
            if (step !== stepTrajectory) {
                return
            }

            personDetectionConfirmed = false
            beginPersonDetection()
        }
    }

    ConfirmationDialog {
        id: goHomeConfirmDialog
        acceptText: qsTr("Aceptar")
        holdToAccept: true

        onAccepted: {
            if (step !== stepExperiment || !initialRobotPoseSaved) {
                return
            }

            if (mapVisualizationManager && mapVisualizationManager.setPathUpdatesEnabled) {
                mapVisualizationManager.setPathUpdatesEnabled(true)
            }

            if (!robotServiceBridge || !robotServiceBridge.navigateToPose) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
                errorPopup.open()
                return
            }

            var okHome = robotServiceBridge.navigateToPose(initialRobotMapPosition.x, initialRobotMapPosition.y, initialRobotTheta)
            if (!okHome) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la navegación a la posición inicial")
                errorPopup.open()
                return
            }
        }
    }

    Timer {
        id: autoLocalizationSpinTimer
        interval: timeoutSettings ? timeoutSettings.autoLocalizationSpinPublishPeriodMs : 100
        repeat: true
        running: false
        onTriggered: {
            if (manualControl) {
                manualControl.updateVelocity(0.0, autoLocalizationAngularSpeed)
            }
        }
    }

    Timer {
        id: experimentSavePromptTimer
        interval: timeoutSettings ? timeoutSettings.experimentSavePromptDelayMs : 3000
        repeat: false
        running: false
        onTriggered: {
            if (experimentFinishBusyDialog.visible) {
                experimentFinishBusyDialog.close()
            }

            if (experimentSavePromptPending && !saveExperimentConfirmDialog.visible && !experimentSaveDialog.visible) {
                saveExperimentConfirmDialog.openWithMessage(qsTr("¿Desea guardar el experimento?"))
            }

            experimentSavePromptPending = false
        }
    }

    Timer {
        id: autoLocalizationStopTimer
        interval: autoLocalizationSpinMs
        repeat: false
        running: false
        onTriggered: {
            stopAutoLocalizationSpin()
            autoLocalizationSpinDone = true
            if (autoLocalizationServiceDone) {
                finalizeAutoLocalization()
            }
        }
    }

    BusyDialog {
        id: busyDialog
        timeoutMs: root.busyTimeoutMs

        onTimedOut: {
            if (personDetectionInProgress) {
                stopPersonDetection()
                errorPopup.errorRectangleTextError.text = qsTr("Error: Tiempo de espera agotado durante la detección de persona")
                errorPopup.open()
                return
            }

            if (autoLocalizationActive || autoLocalizationWaitingForNav || autoLocalizationWaitingForService) {
                failAutoLocalization(qsTr("Error: Tiempo de espera agotado durante la autolocalización"))
                return
            }

            if (waitingForNavigationStart) {
                waitingForNavigationStart = false
                errorPopup.errorRectangleTextError.text = qsTr("Error: Tiempo de espera agotado al iniciar la navegación")
                errorPopup.open()
                return
            }

            if (exiting) {
                exiting = false
                exitAndQuit = false
                errorPopup.errorRectangleTextError.text = qsTr("Error: Tiempo de espera agotado al salir de la prueba")
                errorPopup.open()
                return
            }

            if (!mapAvailable) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: Tiempo de espera agotado al cargar el mapa")
                errorPopup.open()
            }
        }
    }

    BusyDialog {
        id: experimentFinishBusyDialog
        timeoutMs: 0
    }

    ErrorRectangle {
        id: errorPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: ""
    }
}
