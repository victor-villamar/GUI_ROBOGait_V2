import QtQuick 2.15

import "qrc:/Views"
import "qrc:/Common"
import "qrc:/Dialogs"


ApplicationFlowForm {
    id: applicationFlow
    state: "Home"
    property bool keyboardVisible: false
    property real keyboardHeight: 0
    property real keyboardSafeArea: currentKeyboardSafeArea
    property real currentKeyboardSafeArea: {
        var item = mystackview.currentItem
        return item && item.keyboardSafeArea !== undefined ? item.keyboardSafeArea : 0
    }
    keyboardInset: keyboardVisible ? Math.max(0, keyboardHeight - keyboardSafeArea) : 0

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
        target: applicationFlow.state === "register_page" ? mystackview.currentItem : null
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
            if (userSession) {
                userSession.clearRobot()
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
        target: userSession

        function onRobotDisconnectedWithName(robotDisplayName) {
            if (applicationFlow.state !== "register_page" && applicationFlow.state !== "robot_connection") {
                lostConnectionPopupItem.open()
                lostConnectionPopupItem.errorRectangleTextError.text =
                    qsTr("Se perdió conexión con %1. Tiempo de espera agotado.").arg(robotDisplayName)

                while(mystackview.depth > 2) {
                    mystackview.pop()
                }

                applicationFlow.state = "robot_connection"
            }
        }
    }

    Connections {
        target: dbManager

        function onUserLoggedOut() {
            while (mystackview.depth > 1) {
                mystackview.pop()
            }

            mystackview.push(register_page)
            applicationFlow.state = "register_page"
        }

        function onUserNameChanged() {
            if (userSession) {
                userSession.clearPatient()
            }
        }

        function onUserRoleChanged() {
            if (userSession) {
                userSession.clearPatient()
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
