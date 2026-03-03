import QtQuick 2.15
import QtQuick.Controls 2.15

ManualControlForm {
    id: root
    property real maxLinearVelocity: 0.22
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
        if (userSession.rosManager && userSession.rosManager.robotManager) {
            userSession.rosManager.robotManager.enableManualControl()
        }
    }

    Component.onDestruction: {
        if (userSession.rosManager && userSession.rosManager.robotManager) {
            userSession.rosManager.robotManager.disableManualControl()
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

    infoButton.onClicked: {
        infoDialog.openWithMessage(infoDialog.message)
    }

    lockButton.onClicked: {
        manualUnlocked = !manualUnlocked

        if (!manualUnlocked) {
            if (userSession.rosManager && userSession.rosManager.robotManager && userSession.rosManager.robotManager.manualControl) {
                userSession.rosManager.robotManager.manualControl.stopRobot()
            }
        }
    }
}
