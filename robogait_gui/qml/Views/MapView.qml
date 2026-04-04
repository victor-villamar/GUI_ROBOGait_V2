import QtQuick 2.15
import QtQuick.Controls 2.15
import CommandExecutorBridge 1.0

import "qrc:/Dialogs"

MapViewForm {
    id: root
    readonly property real computedIconButtonSizePx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.iconButtonSize, 0) : 50
    readonly property real computedIconGlyphSizePx: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 25
    readonly property real computedHeaderTopInsetPx: uiSizingSettings ? uiSizingSettings.px(3.0, 0) : 12
    readonly property real computedButtonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 36
    readonly property real computedJoystickStickSizePx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.joystickStickSize, 0) : 34
    readonly property real computedJoystickAreaSizePx: computedJoystickStickSizePx * 4

    iconButtonSizePx: computedIconButtonSizePx
    iconGlyphSizePx: computedIconGlyphSizePx
    headerTopInsetPx: computedHeaderTopInsetPx
    buttonHeightPx: computedButtonHeightPx
    joystickAreaSizePx: computedJoystickAreaSizePx

    property real maxLinearVelocity: 0.22
    property bool confirmBackNavigation: false
    property string mapName: ""
    property string mapLocation: ""
    property string mapDescription: ""
    property bool waitingForMappingStop: false
    property bool pendingSaveAndExit: false
    property bool pendingSaveToDb: false
    property bool waitingForResetStop: false
    property bool waitingForResetStart: false
    property bool pendingQuit: false

    signal appExitFinished()

    // Bind to MapVisualizationManager properties
    mapAvailable: (userSession.rosManager &&
                   userSession.rosManager.robotManager &&
                   userSession.rosManager.robotManager.mapVisualizationManager)
                  ? userSession.rosManager.robotManager.mapVisualizationManager.mapAvailable
                  : false

    // Map resolution (meters per pixel)
    mapResolution: (userSession.rosManager &&
                   userSession.rosManager.robotManager &&
                   userSession.rosManager.robotManager.mapVisualizationManager)
                  ? userSession.rosManager.robotManager.mapVisualizationManager.mapResolution
                  : 0.0

    // Scale bar values
    scaleMeters: (userSession.rosManager &&
                  userSession.rosManager.robotManager &&
                  userSession.rosManager.robotManager.mapVisualizationManager)
                 ? userSession.rosManager.robotManager.mapVisualizationManager.scaleMeters
                 : 0.0

    scalePixels: (userSession.rosManager &&
                  userSession.rosManager.robotManager &&
                  userSession.rosManager.robotManager.mapVisualizationManager)
                 ? userSession.rosManager.robotManager.mapVisualizationManager.scalePixels
                 : 0

    // Robot pose availability for UI 
    robotPoseAvailable: (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                       ? userSession.rosManager.robotManager.mapVisualizationManager.robotPoseAvailable
                       : false

    // Laser availability for UI
    laserAvailable: (userSession.rosManager &&
                     userSession.rosManager.robotManager &&
                     userSession.rosManager.robotManager.mapVisualizationManager)
                    ? userSession.rosManager.robotManager.mapVisualizationManager.laserAvailable
                    : false

    // Zoom level for UI
    zoomLevel: (userSession.rosManager &&
                userSession.rosManager.robotManager &&
                userSession.rosManager.robotManager.mapVisualizationManager)
               ? userSession.rosManager.robotManager.mapVisualizationManager.zoomLevel
               : 1.0

    // Bind to ManualControl properties
    linearValue: (userSession.rosManager && userSession.rosManager.robotManager && userSession.rosManager.robotManager.manualControl)
                 ? userSession.rosManager.robotManager.manualControl.linearVelocity
                 : 0.0

    angularValue: (userSession.rosManager && userSession.rosManager.robotManager && userSession.rosManager.robotManager.manualControl)
                  ? userSession.rosManager.robotManager.manualControl.angularVelocity
                  : 0.0

    readonly property var commandExecutorBridge : (userSession && userSession.rosManager && userSession.rosManager.robotManager)
                                                  ? userSession.rosManager.robotManager.commandExecutorBridge
                                                  : null

    function scaleLinear(raw) {
        var maxNormalized = 1.0 - (joystick.stick.width / joystick.totalArea.width)
        if (maxNormalized <= 0) {
            return 0.0
        }
        return raw * (maxLinearVelocity / maxNormalized)
    }

    function saveMapToDatabase(shouldPop) {
        if (!dbManager) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: Base de datos no disponible")
            errorPopup.open()
            return false
        }

        var ok = dbManager.registerMap(mapName, mapLocation, mapDescription)
        if (!ok) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: %1").arg(dbManager.lastError)
            errorPopup.open()
            return false
        }

        if (!userSession || !userSession.rosManager || !userSession.rosManager.robotManager
            || !userSession.rosManager.robotManager.mapVisualizationManager) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: Mapa no disponible")
            errorPopup.open()
            return false
        }

        var mapVizManager = userSession.rosManager.robotManager.mapVisualizationManager

        if(!mapVizManager.generateMapPreview(mapName)) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo generar la vista previa del mapa")
            errorPopup.open()
            return false
        }

        if (shouldPop && root.StackView.view) {
            root.StackView.view.pop()
        }

        return true
    }

    function beginSaveAndStop(shouldPop) {
        pendingSaveAndExit = shouldPop
        pendingSaveToDb = true
        waitingForMappingStop = true

        busyDialog.openWithMessage(qsTr("Guardando mapa..."))

        var okStop = commandExecutorBridge.stopMapping(true, mapName)

        if(!okStop) {
            waitingForMappingStop = false
            pendingSaveAndExit = false
            pendingSaveToDb = false
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo guardar el mapa")
            errorPopup.open()
        }
    }

    function beginStopWithoutSave(shouldPop) {
        pendingSaveAndExit = shouldPop
        pendingSaveToDb = false
        waitingForMappingStop = true

        busyDialog.openWithMessage(qsTr("Saliendo sin guardar..."))

        var okStop = commandExecutorBridge.stopMapping(false, "")

        if(!okStop) {
            waitingForMappingStop = false
            pendingSaveAndExit = false
            pendingQuit = false
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo detener el mapeo")
            errorPopup.open()
        }
    }

    function requestAppExit() {
        if (!commandExecutorBridge || commandExecutorBridge.status !== CommandExecutorBridge.RUNNING) {
            appExitFinished()
            return true
        }

        pendingQuit = true
        beginStopWithoutSave(false)
        return true
    }

    function beginResetMapping() {

        if (commandExecutorBridge.status !== CommandExecutorBridge.RUNNING) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo reiniciar el mapeo")
            errorPopup.open()
            return
        }

        waitingForResetStop = true
        waitingForResetStart = false
        pendingSaveAndExit = false
        pendingSaveToDb = false

        busyDialog.openWithMessage(qsTr("Reiniciando mapeo..."))

        var okStop = commandExecutorBridge.stopMapping(false, "")
        if (!okStop) {
            waitingForResetStop = false
            busyDialog.close()
            errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo detener el mapeo")
            errorPopup.open()
        }
    }

    function emergencyStop() {
        if (userSession && userSession.rosManager && userSession.rosManager.robotManager) {
            var manualControl = userSession.rosManager.robotManager.manualControl
            if (manualControl) {
                manualControl.updateVelocity(0.0, 0.0)
                manualControl.stopRobot()
            }
        }

        var hadOperation = waitingForResetStop || waitingForResetStart || waitingForMappingStop
        waitingForResetStop = false
        waitingForResetStart = false
        waitingForMappingStop = false
        pendingSaveAndExit = false
        pendingSaveToDb = false
        pendingQuit = false

        if (busyDialog.visible) {
            busyDialog.close()
        }

        if (hadOperation && commandExecutorBridge && commandExecutorBridge.status === CommandExecutorBridge.RUNNING) {
            commandExecutorBridge.stopMapping(false, "")
        }
    }

    function handleBackNavigation() {
        if (!confirmBackNavigation) {
            return false
        }

        saveChangesDialog.open()
        return true
    }

    Component.onCompleted: {
        
        if (!userSession.rosManager || !userSession.rosManager.robotManager) {
            return
        }

        var mapVizManager = userSession && userSession.rosManager && userSession.rosManager.robotManager
                           ? userSession.rosManager.robotManager.mapVisualizationManager
                           : null

        if (!mapVizManager) {
            return
        }

        if (!userSession.rosManager.robotManager.selectedRobotNamespace) {
            return
        }

        mapVizManager.activateSubscriptions()
        followButton.checked = mapVizManager.followRobot

        // Enable manual control

        if (userSession.rosManager && userSession.rosManager.robotManager) {
            userSession.rosManager.robotManager.enableManualControl()
        }
    }

    Component.onDestruction: {        
        if (userSession.rosManager && userSession.rosManager.robotManager) {
            var mapVizManager = userSession.rosManager.robotManager.mapVisualizationManager
            if (mapVizManager) {
                mapVizManager.destroySubscriptions()
            }

            userSession.rosManager.robotManager.disableManualControl()
        }
    }

    // Info button
    infoButton.onClicked: {
        infoDialog.open()
    }

    emergencyButton.onClicked: {
        emergencyStop()
    }

    lockButton.onClicked: {
        manualUnlocked = !manualUnlocked

        if (!manualUnlocked) {
            if (userSession.rosManager && userSession.rosManager.robotManager && userSession.rosManager.robotManager.manualControl) {
                userSession.rosManager.robotManager.manualControl.stopRobot()
            }
        }
    }

    joystick.onDirChanged: function(linear, angular) {
        if (!manualUnlocked) {
            return
        }

        var scaledLinear = scaleLinear(linear)

        if (userSession.rosManager && userSession.rosManager.robotManager && userSession.rosManager.robotManager.manualControl) {
            userSession.rosManager.robotManager.manualControl.updateVelocity(scaledLinear, angular)
        }
    }

    joystick.onReleased: {
        if (!manualUnlocked) {
            return
        }

        if (userSession.rosManager && userSession.rosManager.robotManager && userSession.rosManager.robotManager.manualControl) {
            userSession.rosManager.robotManager.manualControl.stopRobot()
        }
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

    saveMapButton.onClicked: {
        saveConfirmDialog.openWithMessage(qsTr("¿Está seguro de guardar el mapa creado?"))
    }

    resetMapButton.onClicked: {
        resetConfirmDialog.openWithMessage(qsTr("¿Está seguro de resetear el mapa creado?"))
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

    Dialog {
        id: saveChangesDialog
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose

        parent: Overlay.overlay

        Overlay.modal: Rectangle {
            anchors.fill: parent
            color: "#A0505050"
        }

        width: parent ? Math.min(520, parent.width * 0.85) : 520
        x: parent ? Math.round((parent.width - width) / 2) : 0
        y: parent ? Math.round((parent.height - height) / 2) : 0

        background: Rectangle {
            color: "#a9cfe8"
            border.color: "#ffffff"
            border.width: 2
            radius: 12
        }

        contentItem: Item {
            implicitWidth: saveChangesDialog.width
            implicitHeight: messageText.implicitHeight + buttonRow.implicitHeight + 40

            Text {
                id: messageText
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: 18
                anchors.leftMargin: 18
                anchors.rightMargin: 18
                text: qsTr("¿Guardar los cambios en este archivo?")
                color: "#045671"
                font.pixelSize: 18
                font.bold: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Row {
                id: buttonRow
                spacing: 12
                anchors.top: messageText.bottom
                anchors.topMargin: 20
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    id: saveButton
                    width: 120
                    height: root.buttonHeightPx
                    text: qsTr("Guardar")

                    background: Rectangle {
                        color: saveButton.down ? "#518bb7" : "#ffffff"
                        radius: 8
                        border.color: "#045671"
                        border.width: 2
                    }

                    contentItem: Label {
                        text: saveButton.text
                        color: "#045671"
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        beginSaveAndStop(true)
                        saveChangesDialog.close()
                    }
                }

                Button {
                    id: noSaveButton
                    width: 120
                    height: root.buttonHeightPx
                    text: qsTr("No Guardar")

                    background: Rectangle {
                        color: noSaveButton.down ? "#518bb7" : "#ffffff"
                        radius: 8
                        border.color: "#045671"
                        border.width: 2
                    }

                    contentItem: Label {
                        text: noSaveButton.text
                        color: "#045671"
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        saveChangesDialog.close()
                        beginStopWithoutSave(true)
                    }
                }

                Button {
                    id: cancelButton
                    width: 120
                    height: root.buttonHeightPx
                    text: qsTr("Cancelar")

                    background: Rectangle {
                        color: cancelButton.down ? "#518bb7" : "#ffffff"
                        radius: 8
                        border.color: "#045671"
                        border.width: 2
                    }

                    contentItem: Label {
                        text: cancelButton.text
                        color: "#045671"
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: saveChangesDialog.close()
                }
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

    ConfirmationDialog {
        id: saveConfirmDialog
        holdToAccept: false
        acceptText: qsTr("Guardar")

        onAccepted: {
            beginSaveAndStop(true)
        }
    }

    ConfirmationDialog {
        id: resetConfirmDialog
        holdToAccept: false
        acceptText: qsTr("Resetear")

        onAccepted: {
            beginResetMapping()
        }
    }

    Connections {
        target: (userSession && userSession.rosManager && userSession.rosManager.robotManager &&
                 userSession.rosManager.robotManager.mapVisualizationManager)
                  ? userSession.rosManager.robotManager.mapVisualizationManager
                  : null

        function onMapAvailableChanged() {
            if(target.mapAvailable) {
                target.fitToView()
            }
        }
    }

    Connections {
        target: commandExecutorBridge

        function onStatusChanged() {

            // Handle reset stop

            if (waitingForResetStop && commandExecutorBridge.status === CommandExecutorBridge.STOPPED) {
                waitingForResetStop = false
                waitingForResetStart = true

                var mapViz = userSession && userSession.rosManager && userSession.rosManager.robotManager
                             ? userSession.rosManager.robotManager.mapVisualizationManager
                             : null
                if (mapViz)
                {
                    mapViz.clearMap()
                }

                var okStart = commandExecutorBridge.startMapping()
                if (!okStart) {
                    waitingForResetStart = false
                    busyDialog.close()
                    errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar el mapeo")
                    errorPopup.open()
                }
                return
            }

            if (waitingForResetStart && commandExecutorBridge.status === CommandExecutorBridge.RUNNING) {
                waitingForResetStart = false
                busyDialog.close()
                return
            }

            if((waitingForResetStop || waitingForResetStart) && commandExecutorBridge.status === CommandExecutorBridge.ERROR) {
                waitingForResetStop = false
                waitingForResetStart = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo reiniciar el mapeo")
                errorPopup.open()
                return
            }

            // Handle mapping stop

            if (!waitingForMappingStop) {
                return
            }

            if (commandExecutorBridge.status === CommandExecutorBridge.STOPPED) {
                waitingForMappingStop = false
                busyDialog.close()

                if (pendingQuit) {
                    pendingQuit = false
                    appExitFinished()
                    return
                }

                if (pendingSaveAndExit) {
                    pendingSaveAndExit = false
                    if (pendingSaveToDb) {
                        pendingSaveToDb = false
                        saveMapToDatabase(true)
                    } else if (root.StackView.view) {
                        root.StackView.view.pop()
                    }
                }
            }
            else if (commandExecutorBridge.status === CommandExecutorBridge.ERROR) {
                var wasSaving = pendingSaveToDb
                waitingForMappingStop = false
                pendingSaveAndExit = false
                pendingSaveToDb = false
                pendingQuit = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = wasSaving
                    ? qsTr("Error: No se pudo guardar el mapa.")
                    : qsTr("Error: No se pudo detener el mapeo.")
                errorPopup.open()
            }
        }

        function onRequestFinished(success) {
            if (success) {
                return
            }

            if (waitingForResetStop || waitingForResetStart) {
                waitingForResetStop = false
                waitingForResetStart = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo reiniciar el mapeo")
                errorPopup.open()
                return
            }

            if (waitingForMappingStop) {
                var wasSaving = pendingSaveToDb
                waitingForMappingStop = false
                pendingSaveAndExit = false
                pendingSaveToDb = false
                pendingQuit = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = wasSaving
                    ? qsTr("Error: No se pudo guardar el mapa.")
                    : qsTr("Error: No se pudo detener el mapeo.")
                errorPopup.open()
            }
        }

    }
}
