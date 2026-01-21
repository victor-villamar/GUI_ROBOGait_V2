import QtQuick 2.15
import "extras"


ApplicationFlowForm {
    id: applicationFlow
    state: "Home"

    home.buttonStart.onClicked: {
        mystackview.push(robot_connection)
        applicationFlow.state = "robot_connection"
    }
    function backButton()
    {
        if (mystackview.depth > 1) {
            mystackview.pop()
            applicationFlow.state = applicationFlow.previousState
        }
    }

    states: [
        State{
            name: "Home"

            PropertyChanges {
                target: toolbar
                backButton.opacity: 0
                backButton.enabled: false
                logo.opacity: 0
                logo.enabled: false
                title.opacity: 0
                title.enabled: false
            }
            PropertyChanges {
                target: mystackview
                anchors.top: toolbar.bottom
                anchors.bottom: parent.bottom
            }
        },
        State {
            name: "robot_connection"
            PropertyChanges {
                target:applicationFlow
                previousState: "Home"
            }
            PropertyChanges {
                target: toolbar
                backButton.opacity: 1
                backButton.enabled: true
            }
            PropertyChanges {
                target: mystackview
                anchors.top: toolbar.bottom
                anchors.bottom: parent.bottom
            }
        },
        State {
            name: "robot_cmd_vel"
            PropertyChanges { 
                target: applicationFlow
                previousState: "robot_connection"
            }
            PropertyChanges {
                target: toolbar
                backButton.opacity: 0
                backButton.enabled: false
            }
            PropertyChanges { 
                target: mystackview
                anchors.top: toolbar.bottom
                anchors.bottom: parent.bottom
            }
        }
    ]
}
