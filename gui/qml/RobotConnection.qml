import QtQuick 2.15
import "extras"

RobotConnectionForm {
    buttonSearchRobot{
        onClicked: {
            rosManager.robotDiscovery.startScanning()
        }
    }

    Component.onDestruction: rosManager.robotDiscovery.stopScanning()
}

