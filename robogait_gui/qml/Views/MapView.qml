import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"

MapViewForm {
    id: root

    property real maxLinearVelocity: 0.22
    property bool confirmBackNavigation: false
    property string mapName: ""
    property string mapLocation: ""
    property string mapDescription: ""

    // Bind to MapVisualizationManager properties
    mapAvailable: (userSession.rosManager &&
                   userSession.rosManager.robotManager &&
                   userSession.rosManager.robotManager.mapVisualizationManager)
                  ? userSession.rosManager.robotManager.mapVisualizationManager.mapAvailable
                  : false

    // Robot pose availability for UI 
    robotPoseAvailable: (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                       ? userSession.rosManager.robotManager.mapVisualizationManager.robotPoseAvailable
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

        var ok = dbManager.registerMap(mapName, mapLocation, mapDescription)
        if (!ok) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: %1").arg(dbManager.lastError)
            errorPopup.open()
            return false
        }

        if (shouldPop && root.StackView.view) {
            root.StackView.view.pop()
        }

        return true
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

        var mapVizManager = userSession.rosManager.robotManager.mapVisualizationManager
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
                    height: 32
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
                        if (saveMapToDatabase(true)) {
                            saveChangesDialog.close()
                        }
                    }
                }

                Button {
                    id: noSaveButton
                    width: 120
                    height: 32
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
                        if (root.StackView.view) {
                            root.StackView.view.pop()
                        }
                    }
                }

                Button {
                    id: cancelButton
                    width: 120
                    height: 32
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
            saveMapToDatabase(true)
        }
    }

    ConfirmationDialog {
        id: resetConfirmDialog
        holdToAccept: false
        acceptText: qsTr("Resetear")

        onAccepted: {
            // TODO: Reset map logic
        }
    }

}
