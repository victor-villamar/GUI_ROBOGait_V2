import QtQuick 2.15
import QtQuick.Controls 2.15

MapViewForm {
    id: root

    property real maxLinearVelocity: 0.22

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

        // Activate MapVisualizationManager subscriptions
        mapVizManager.activateSubscriptions()

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
        mapRenderWidget.zoomIn()
    }

    zoomOutButton.onClicked: {
        mapRenderWidget.zoomOut()
    }

    fitButton.onClicked: {
        mapRenderWidget.fitToView()
    }
}
