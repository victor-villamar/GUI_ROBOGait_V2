import QtQuick 2.15
import QtQuick.Controls 2.15

RobotConnectionForm {

    function goToMainMenu() {
        if(StackView.view)
        {
            StackView.view.push(mainMenuPage)
            applicationFlow.state = "main_menu"
        }
    }

    buttonSearchRobot{
        onClicked: {
            userSession.rosManager.robotDiscovery.startScanning()
        }
    }

    Component.onDestruction: userSession.rosManager.robotDiscovery.stopScanning()

    Component {
        id: mainMenuPage
        MainMenu {}
    }
}

