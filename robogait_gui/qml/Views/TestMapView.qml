import QtQuick 2.15
import QtQuick.Controls 2.15
import CommandExecutorBridge 1.0

import "qrc:/Dialogs"
import "qrc:/Views"

TestMapViewForm {

    id: root

    property int experimentId: -1
    property bool experimentRegistered: false

    property bool exiting: false

    property bool waitingForNavigationStart: false

    property bool autoLocalizationActive: false
    property bool autoLocalizationWaitingForNav: false
    property bool autoLocalizationWaitingForService: false
    property bool autoLocalizationCompleted: false
    property bool autoLocalizationServiceDone: false
    property bool autoLocalizationSpinDone: false
    property int autoLocalizationSpinMs: 30000
    property real autoLocalizationAngularSpeed: 0.75

    readonly property var manualControl: (userSession && userSession.rosManager && userSession.rosManager.robotManager)
                                            ? userSession.rosManager.robotManager.manualControl
                                            : null

    readonly property int stepPosition: 0
    readonly property int stepOrientation: 1
    readonly property int stepNavigation: 2

    step: stepPosition

    readonly property var commandExecutorBridge : (userSession && userSession.rosManager && userSession.rosManager.robotManager)
                                                  ? userSession.rosManager.robotManager.commandExecutorBridge
                                                  : null

    mapAvailable: (userSession.rosManager &&
                   userSession.rosManager.robotManager &&
                   userSession.rosManager.robotManager.mapVisualizationManager)
                  ? userSession.rosManager.robotManager.mapVisualizationManager.mapAvailable
                  : false

    showRobotPose: mapAvailable && ((placementController && placementController.hasPosition) || autoLocalizationCompleted || autoLocalizationActive)

    onStepChanged: {
        if (step !== stepPosition) {
            placementEnabled = false
        syncRobotPoseUpdates()
        }

        if (step !== stepOrientation) {
            orientationEnabled = false
            orientationOverride = false
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

    function syncRobotPoseUpdates() {
        var mapVizManager = userSession && userSession.rosManager && userSession.rosManager.robotManager
                           ? userSession.rosManager.robotManager.mapVisualizationManager
                           : null
        if (!mapVizManager) {
            return
        }

        var enableUpdates = (step === stepNavigation) || autoLocalizationActive
        mapVizManager.setRobotPoseUpdatesEnabled(enableUpdates)
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
        if (!commandExecutorBridge) {
            failAutoLocalization(qsTr("Error: No hay conexión con el robot"))
            return
        }

        autoLocalizationWaitingForNav = false
        autoLocalizationWaitingForService = true

        var okService = commandExecutorBridge.reinitializeGlobalLocalization()
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

        if (!commandExecutorBridge) {
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

        if (commandExecutorBridge.activeCommandKey === "navigation") {
            if (commandExecutorBridge.status === CommandExecutorBridge.RUNNING) {
                triggerGlobalLocalization()
                return
            }

            if (commandExecutorBridge.status === CommandExecutorBridge.STARTING) {
                autoLocalizationWaitingForNav = true
                return
            }
        }

        autoLocalizationWaitingForNav = true
        var okStart = commandExecutorBridge.startNavigation(mapName)
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

    function beginExit() {
        if (exiting) {
            return
        }

        exiting = true
        busyDialog.openWithMessage(qsTr("Saliendo..."))

        if (commandExecutorBridge && commandExecutorBridge.activeCommandKey === "navigation") {
            var okStop = commandExecutorBridge.stopNavigation()
            if (!okStop) {
                busyDialog.close()
                exiting = false
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

        if (!commandExecutorBridge) {
            finalizeExit()
            return
        }

        if (commandExecutorBridge.activeCommandKey !== "navigation") {
            finalizeExit()
            return
        }

        if (commandExecutorBridge.status === CommandExecutorBridge.STOPPED
            || commandExecutorBridge.status === CommandExecutorBridge.ERROR
            || commandExecutorBridge.status === CommandExecutorBridge.IDLE) {
            finalizeExit()
        }
    }

    function finalizeExit() {
        exiting = false
        busyDialog.close()

        if (placementController) {
            placementController.clear()
        }

        if (userSession && userSession.rosManager && userSession.rosManager.robotManager) {
            var mapVizManager = userSession.rosManager.robotManager.mapVisualizationManager
            if (mapVizManager) {
                mapVizManager.destroySubscriptions()
            }
        }

        if (dbManager && experimentRegistered && experimentId > 0) {
            dbManager.deleteExperiment(experimentId)
            experimentRegistered = false
            experimentId = -1
        }

        if (StackView.view) {
            while (StackView.view.depth > 3) {
                StackView.view.pop()
            }
        }
    }

    Component.onCompleted: {
        busyDialog.openWithMessage(qsTr("Cargando mapa..."))

        var mapVizManager = userSession && userSession.rosManager && userSession.rosManager.robotManager
                           ? userSession.rosManager.robotManager.mapVisualizationManager
                           : null

        if (mapVizManager) {
            mapVizManager.activateSubscriptions()
            syncRobotPoseUpdates()
        }

        if (!dbManager || !userSession) {
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: Sesión o base de datos no disponible")
            errorPopup.open()
            return
        }

        var patient = userSession.currentPatient
        if (!patient) {
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay paciente seleccionado")
            errorPopup.open()
            return
        }

        var mapName = (userSession.currentMapName || "").trim()
        if (mapName === "") {
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay mapa seleccionado")
            errorPopup.open()
            return
        }

        if (!commandExecutorBridge) {
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
            errorPopup.open()
            return
        }

        var ok = dbManager.registerExperiment(patient.name, patient.lastName, mapName)
        if (!ok) {
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: %1").arg(dbManager.lastError)
            errorPopup.open()
            return
        }

        experimentRegistered = true
        experimentId = dbManager.lastExperimentId

        var okMap = commandExecutorBridge.requestMapData(mapName)
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

        if (!commandExecutorBridge) {
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

        if (commandExecutorBridge.activeCommandKey === "navigation") {
            if (commandExecutorBridge.status === CommandExecutorBridge.RUNNING) {
                placementEnabled = false
        syncRobotPoseUpdates()
                orientationEnabled = false
            orientationOverride = false
                step = stepOrientation
                return
            }

            if (commandExecutorBridge.status === CommandExecutorBridge.STARTING) {
                waitingForNavigationStart = true
                busyDialog.openWithMessage(qsTr("Guardando posición..."))
                return
            }
        }

        waitingForNavigationStart = true
        busyDialog.openWithMessage(qsTr("Guardando posición..."))

        var okStart = commandExecutorBridge.startNavigation(mapName)
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

        step = stepNavigation
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

    zoomInButton.onClicked: {
        if (userSession.rosManager &&
            userSession.rosManager.robotManager &&
            userSession.rosManager.robotManager.mapVisualizationManager) {
            userSession.rosManager.robotManager.mapVisualizationManager.followRobot = false
            userSession.rosManager.robotManager.mapVisualizationManager.zoomIn()
        }
    }

    zoomOutButton.onClicked: {
        if (userSession.rosManager &&
            userSession.rosManager.robotManager &&
            userSession.rosManager.robotManager.mapVisualizationManager) {
            userSession.rosManager.robotManager.mapVisualizationManager.followRobot = false
            userSession.rosManager.robotManager.mapVisualizationManager.zoomOut()
        }
    }

    fitButton.onClicked: {
        if (userSession.rosManager &&
            userSession.rosManager.robotManager &&
            userSession.rosManager.robotManager.mapVisualizationManager) {
            userSession.rosManager.robotManager.mapVisualizationManager.followRobot = false
            userSession.rosManager.robotManager.mapVisualizationManager.fitToView()
        }
    }

    followButton.onClicked: {
        if (userSession.rosManager &&
            userSession.rosManager.robotManager &&
            userSession.rosManager.robotManager.mapVisualizationManager) {
            var mapVizManager = userSession.rosManager.robotManager.mapVisualizationManager
            mapVizManager.followRobot = !mapVizManager.followRobot
        }
    }

    Connections {
        target: (userSession.rosManager &&
                 userSession.rosManager.robotManager &&
                 userSession.rosManager.robotManager.mapVisualizationManager)
                ? userSession.rosManager.robotManager.mapVisualizationManager
                : null

        function onFollowRobotChanged() {
            followButton.checked = target.followRobot
        }
    }

    Component.onDestruction: {
        stopAutoLocalizationSpin()

        if (userSession && userSession.rosManager && userSession.rosManager.robotManager) {
            var mapVizManager = userSession.rosManager.robotManager.mapVisualizationManager
            if (mapVizManager) {
                mapVizManager.destroySubscriptions()
            }
        }
    }

    infoButton.onClicked: {
        infoDialog.open()
    }

    ConfirmationDialog {
        id: exitConfirmDialog
        holdToAccept: true
        acceptText: qsTr("Salir")

        onAccepted: {
            beginExit()
        }
    }


    Connections {
        target: commandExecutorBridge
        ignoreUnknownSignals: true

        function onStatusChanged() {
            if (autoLocalizationWaitingForNav)
            {
                if (commandExecutorBridge.activeCommandKey === "navigation"
                    && commandExecutorBridge.status === CommandExecutorBridge.RUNNING)
                {
                    triggerGlobalLocalization()
                }
                else if (commandExecutorBridge.activeCommandKey === "navigation"
                           && commandExecutorBridge.status === CommandExecutorBridge.ERROR)
                {
                    autoLocalizationWaitingForNav = false
                    failAutoLocalization(qsTr("Error: No se pudo iniciar la navegación"))
                }
            }

            if (waitingForNavigationStart)
            {
                if (commandExecutorBridge.activeCommandKey === "navigation"
                    && commandExecutorBridge.status === CommandExecutorBridge.RUNNING)
                {
                    waitingForNavigationStart = false
                    busyDialog.close()
                    placementEnabled = false
                    syncRobotPoseUpdates()
                    orientationEnabled = false
                    orientationOverride = false
                    step = stepOrientation
                }
                else if (commandExecutorBridge.activeCommandKey === "navigation"
                           && commandExecutorBridge.status === CommandExecutorBridge.ERROR)
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

    }


    ConfirmationDialog {
        id: autoLocalizationConfirmDialog
        acceptText: qsTr("Iniciar")
        holdToAccept: true

        onAccepted: {
            beginAutoLocalization()
        }
    }

    Timer {
        id: autoLocalizationSpinTimer
        interval: 100
        repeat: true
        running: false
        onTriggered: {
            if (manualControl) {
                manualControl.updateVelocity(0.0, autoLocalizationAngularSpeed)
            }
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
    }

    ErrorRectangle {
        id: errorPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: ""
    }
}
