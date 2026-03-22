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

    onStepChanged: {
        if (step !== stepPosition) {
            placementEnabled = false
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


    onConfirmPlacementRequested: {
        if (step !== stepPosition) {
            return
        }

        if (!placementController || !placementController.hasPosition) {
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

        if (commandExecutorBridge.activeCommandKey === "navigation") {
            if (commandExecutorBridge.status === CommandExecutorBridge.RUNNING) {
                placementEnabled = false
                step = stepOrientation
                if (placementController && !placementController.hasOrientation) {
                    placementController.setOrientationDegrees(0)
                }
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

        if (!placementController || !placementController.hasOrientation) {
            return
        }

        var rm = (userSession && userSession.rosManager) ? userSession.rosManager.robotManager : null
        if (!rm) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No hay conexión con el robot")
            errorPopup.open()
            return
        }

        var pos = placementController.position
        rm.publishInitialPose(pos.x, pos.y, placementController.theta)

        step = stepNavigation
    }

    onBackOrientationRequested: {
        if (step !== stepOrientation) {
            return
        }

        if (placementController) {
            placementController.clear()
        }

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
            if (waitingForNavigationStart) {
                if (commandExecutorBridge.activeCommandKey === "navigation"
                    && commandExecutorBridge.status === CommandExecutorBridge.RUNNING) {
                    waitingForNavigationStart = false
                    busyDialog.close()
                    placementEnabled = false
                    step = stepOrientation
                    if (placementController && !placementController.hasOrientation) {
                        placementController.setOrientationDegrees(0)
                    }
                } else if (commandExecutorBridge.activeCommandKey === "navigation"
                           && commandExecutorBridge.status === CommandExecutorBridge.ERROR) {
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
            if (success) {
                return
            }

            if (waitingForNavigationStart) {
                waitingForNavigationStart = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la navegación")
                errorPopup.open()
                return
            }

            if (exiting) {
                exiting = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo detener la navegación")
                errorPopup.open()
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
