import QtQuick 2.15

import "qrc:/Views"
import "qrc:/Common"
import "qrc:/Dialogs"


ApplicationFlowForm {
    id: applicationFlow
    state: "Home"

    home.buttonStart.onClicked: {
        mystackview.push(register_page)
        applicationFlow.state = "register_page"
    }

    function backButton()
    {
        if (mystackview.depth > 3) {
            mystackview.pop()
            applicationFlow.state = "main_menu"
            return
        }

        if (mystackview.depth > 1) {
            mystackview.pop()
            applicationFlow.state = applicationFlow.previousState
        }
    }

    Connections {
        target: mystackview.currentItem
        ignoreUnknownSignals: true

        function onAuthenticated() {
            mystackview.replace(robot_connection)
            applicationFlow.state = "robot_connection"
        }
    }

    Connections {
        target: toolbar.userBadge
        ignoreUnknownSignals: true

        function onDisconnectRobotRequested() {
            if (rosManager && rosManager.robotManager) {
                rosManager.robotManager.clearSelection()
            }

            while (mystackview.depth > 2) {
                mystackview.pop()
            }

            applicationFlow.state = "robot_connection"
        }

        function onChangeUserRequested() {
            if (applicationFlow.userSwitchDialog) {
                applicationFlow.userSwitchDialog.open()
            }
        }
    }

    Connections {
        target: rosManager.robotDiscovery

        function onRobotsNamespacesChanged() {
            if (!rosManager || !rosManager.robotManager)
            {
                return
            }

            var nsFull = rosManager.robotManager.selectedRobotNamespace

            if (!nsFull || nsFull.length === 0)
            {
                return
            }

            var robotnamespaces = rosManager.robotDiscovery.robotsNamespaces
            var present = robotnamespaces.indexOf(nsFull) !== -1

            if (!present) {
                var display = rosManager.robotManager.selectedRobotDisplayName
                lostConnectionPopupItem.open()
                lostConnectionPopupItem.errorRectangleTextError.text =
                    qsTr("Se perdió conexión con %1 inesperadamente.").arg(display)

                rosManager.robotManager.clearSelection()

                while(mystackview.depth > 2) {
                    mystackview.pop()
                }

                applicationFlow.state = "robot_connection"
            }
        }
    }

    Connections {
        target: dbManager

        function onPassLoginChanged() {
            if (!dbManager.passLogin) {
                if (rosManager && rosManager.robotManager) {
                    rosManager.robotManager.clearSelection()
                }

                if (patient) {
                    patient.clear()
                }

                while (mystackview.depth > 1) {
                    mystackview.pop()
                }

                mystackview.push(register_page)
                applicationFlow.state = "register_page"
            }
        }

        function onUserNameChanged() {
            if (patient) {
                patient.clear()
            }
        }

        function onUserRoleChanged() {
            if (patient) {
                patient.clear()
            }

            if (dbManager.userRole === "guest") {
                while (mystackview.depth > 3) {
                    mystackview.pop()
                }
            }
        }
    }

    states: [
        State{
            name: "Home"

            PropertyChanges {
                target: toolbar
                logo.opacity: 0
                logo.enabled: false
                title.opacity: 0
                title.enabled: false
                showRobotBadge: false
                showUserBadge: false
            }
            PropertyChanges {
                target: mystackview
                anchors.top: toolbar.bottom
                anchors.bottom: parent.bottom
            }
        },
        State {
            name: "register_page"
            PropertyChanges {
                target: applicationFlow
                previousState: "Home"
            }
            PropertyChanges {
                target: toolbar
                showRobotBadge: false
                showUserBadge: false
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
                showRobotBadge: true
                showUserBadge: true
            }
            PropertyChanges {
                target: mystackview
                anchors.top: toolbar.bottom
                anchors.bottom: parent.bottom
            }
        },
        State {
            name: "main_menu"
            PropertyChanges { 
                target: applicationFlow
                previousState: "robot_connection"
            }
            PropertyChanges {
                target: toolbar
                showRobotBadge: true
                showUserBadge: true
            }
            PropertyChanges { 
                target: mystackview
                anchors.top: toolbar.bottom
                anchors.bottom: parent.bottom
            }
        }
    ]
}
