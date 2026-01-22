import QtQuick 2.15

RobotConnectionForm {
    buttonSearchRobot{
        onClicked: {
            rosManager.robotDiscovery.startScanning()
        }
    }

    Component.onDestruction: rosManager.robotDiscovery.stopScanning()
}

