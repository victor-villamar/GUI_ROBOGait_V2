import QtQuick 2.15
import "extras"

RobotConnectionForm {
    buttonSearchRobot{
        onClicked:  rosManager.robotDiscovery.startScanning()
    }

    InformationRectangle {
        id: infoPopup
        anchors.centerIn: parent
        infoRectangleText.text: ""
    }
    Component.onDestruction: rosManager.robotDiscovery.stopScanning()
    Connections {
        target: stringHandler
        function onStrFindRobotChanged() {
            infoPopup.open()
            infoPopup.infoRectangleText.text = qsTr(stringHandler.strFindRobot)
            if (!infoPopup.__connectedToClick) {
                infoPopup.__connectedToClick = true
                infoPopup.infoRectangleMouseArea.clickedFromPopup.connect(function() {
                    // applicationFlow.register_page_push()
                })
            }
        }
    }

}

