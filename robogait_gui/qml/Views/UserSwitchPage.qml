import QtQuick 2.15

import "qrc:/Register"

Item {
    id: root

    signal userSwitched()

    state: "login"

    Login {
        id: loginPage
        anchors.fill: parent

        onGoToRegister: root.state = "sign_in"
        onGoToGuest: root.state = "guest"
        onAuthenticated: root.userSwitched()
    }

    SignIn {
        id: signInPage
        anchors.fill: parent
        visible: false
        enabled: false

        onGoToLogin: root.state = "login"
        onGoToGuest: root.state = "guest"
    }

    Guest {
        id: guestPage
        anchors.fill: parent
        visible: false
        enabled: false

        onGoToLogin: root.state = "login"
        onGoToRegister: root.state = "sign_in"
        onAuthenticated: root.userSwitched()
    }

    states: [
        State {
            name: "login"
            PropertyChanges {
                target: loginPage
                visible: true
                enabled: true
            }
            PropertyChanges {
                target: signInPage
                visible: false
                enabled: false
            }
            PropertyChanges {
                target: guestPage
                visible: false
                enabled: false
            }
        },
        State {
            name: "sign_in"
            PropertyChanges {
                target: loginPage
                visible: false
                enabled: false
            }
            PropertyChanges {
                target: signInPage
                visible: true
                enabled: true
            }
            PropertyChanges {
                target: guestPage
                visible: false
                enabled: false
            }
        },
        State {
            name: "guest"
            PropertyChanges {
                target: loginPage
                visible: false
                enabled: false
            }
            PropertyChanges {
                target: signInPage
                visible: false
                enabled: false
            }
            PropertyChanges {
                target: guestPage
                visible: true
                enabled: true
            }
        }
    ]
}
