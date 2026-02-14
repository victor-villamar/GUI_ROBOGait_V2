import QtQuick 2.15
import QtQuick.Controls 2.15

MapViewForm {
    id: root

    property real maxLinearVelocity: 0.22
    readonly property real zoomStep: 0.2
    readonly property real minZoom: 0.5
    readonly property real maxZoom: 5.0

    // Bind to MapManager properties
    mapAvailable: (userSession.rosManager && userSession.rosManager.mapManager)
                  ? userSession.rosManager.mapManager.mapAvailable
                  : false

    mapWidth: (userSession.rosManager && userSession.rosManager.mapManager)
              ? userSession.rosManager.mapManager.mapWidth
              : 0

    mapHeight: (userSession.rosManager && userSession.rosManager.mapManager)
               ? userSession.rosManager.mapManager.mapHeight
               : 0

    mapResolution: (userSession.rosManager && userSession.rosManager.mapManager)
                   ? userSession.rosManager.mapManager.mapResolution
                   : 0.0

    robotScreenPos: (userSession.rosManager && userSession.rosManager.mapManager)
                    ? userSession.rosManager.mapManager.robotScreenPos
                    : Qt.point(0, 0)

    robotScreenRotation: (userSession.rosManager && userSession.rosManager.mapManager)
                         ? userSession.rosManager.mapManager.robotScreenRotation
                         : 0.0

    robotPoseAvailable: (userSession.rosManager && userSession.rosManager.mapManager)
                        ? userSession.rosManager.mapManager.robotPoseAvailable
                        : false

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

    function fitToView() {
        // Reset zoom and pan
        zoomLevel = 1.0
        panX = 0
        panY = 0
    }

    function zoomIn() {
        var newZoom = zoomLevel + zoomStep
        zoomLevel = Math.min(maxZoom, newZoom)
    }

    function zoomOut() {
        var newZoom = zoomLevel - zoomStep
        zoomLevel = Math.max(minZoom, newZoom)
    }

    Component.onCompleted: {
        
        if (!userSession.rosManager || !userSession.rosManager.mapManager) {
            return
        }

        if (!userSession.rosManager.robotManager || !userSession.rosManager.robotManager.selectedRobotNamespace) {
            return
        }

        // Activate MapManager subscriptions
        userSession.rosManager.mapManager.activateSubscriptions()
        console.log("[MapView] MapManager subscriptions activated")

        // Enable manual control
        if (userSession.rosManager && userSession.rosManager.robotManager) {
            userSession.rosManager.robotManager.enableManualControl()
        }
    }

    Component.onDestruction: {        
        if (userSession.rosManager && userSession.rosManager.mapManager) {
            userSession.rosManager.mapManager.destroySubscriptions()
        }

        if (userSession.rosManager && userSession.rosManager.robotManager) {
            userSession.rosManager.robotManager.disableManualControl()
        }
    }

    // Update map image when it changes
    // TODO: implement map image update
    Connections {
        target: userSession.rosManager && userSession.rosManager.mapManager 
                ? userSession.rosManager.mapManager 
                : null

        function onMapImageChanged() {
            if (userSession.rosManager && userSession.rosManager.mapManager) {
                mapImage.source = ""
                mapImage.source = "image://mapimage/" + Date.now()
            }
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
        zoomIn()
    }

    zoomOutButton.onClicked: {
        zoomOut()
    }

    fitButton.onClicked: {
        fitToView()
    }

}
