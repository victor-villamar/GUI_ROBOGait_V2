import QtQuick 2.15

RegisterPageForm {
    id: registerPage
    state: "register_page_login"

    signal authenticated()

    function restartIdleTimer()
    {
        idleTimer.stop()
        if (registerPage.visible && registerPage.enabled) {
            idleTimer.start()
        }
    }

    function loginPush()  {
         state = "register_page_login"
    }
    function signInPush() {
         state = "register_page_sign_in"
    }
    function guestPush()  {
         state = "register_page_guest"
    }

    Timer {
        id: idleTimer
        interval: 10000
        repeat: false
        onTriggered: {
            if (!applicationFlow || !applicationFlow.mystackview) {
                return
            }

            while (applicationFlow.mystackview.depth > 1) {
                applicationFlow.mystackview.pop()
            }

            applicationFlow.state = "Home"
        }
    }

    onVisibleChanged: restartIdleTimer()
    onEnabledChanged: restartIdleTimer()
    onStateChanged: restartIdleTimer()
    Component.onCompleted: restartIdleTimer()

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        hoverEnabled: true
        propagateComposedEvents: true
        onPressed: function(mouse) {
            registerPage.restartIdleTimer()
            mouse.accepted = false
        }
        onWheel: function(wheel) {
            registerPage.restartIdleTimer()
            wheel.accepted = false
        }
    }

    Connections {
         target: loginPage.usernameField

         function onTextChanged() {
            registerPage.restartIdleTimer()
         }
    }
    Connections {
        target: loginPage.passwordField

         function onTextChanged() {
            registerPage.restartIdleTimer()
         }
    }
    Connections {
        target: signInPage.nameField
        function onTextChanged() {
            registerPage.restartIdleTimer()
        }
    }
    Connections {
        target: signInPage.lastNameField
        function onTextChanged() {
            registerPage.restartIdleTimer()
        }
    }
    Connections {
        target: signInPage.userNameField
        function onTextChanged() {
            registerPage.restartIdleTimer()
        }
    }
    Connections {
        target: signInPage.passwordField
        function onTextChanged() {
            registerPage.restartIdleTimer()
        }
    }
    Connections {
        target: signInPage.repeatPasswordField
        function onTextChanged() {
            registerPage.restartIdleTimer()
        }
    }
    Connections {
        target: signInPage.roleCombo
        function onCurrentIndexChanged() {
            registerPage.restartIdleTimer()
        }
    }
    Connections {
        target: guestPage.nameField
        function onTextChanged() {
            registerPage.restartIdleTimer()
        }
    }
    Connections {
        target: guestPage.lastNameField
        function onTextChanged() {
            registerPage.restartIdleTimer()
        }
    }

    // Connections for loginPage

    Connections {
        target: loginPage

        function onGoToRegister() {
            signInPush()
        }
    }
    Connections {
        target: loginPage
        function onGoToGuest() {
            guestPush()
        }
    }
    Connections {
        target: loginPage
        function onAuthenticated() {
            authenticated()
        }
    }

    // Connections for signInPage

    Connections {
        target: signInPage
        function onGoToLogin() {
            loginPush()
        }
    }
    Connections {
        target: signInPage
        function onGoToGuest() {
            guestPush()
        }
    }

    // Connections for guestPage

    Connections {
        target: guestPage
        function onGoToLogin() {
            loginPush()
        }
    }
    Connections {
        target: guestPage
        function onGoToRegister() {
            signInPush()
        }
    }
    Connections {
        target: guestPage
        function onAuthenticated() {
            authenticated()
        }
    }

    // States

    states: [
        State {
            name: "register_page_login"
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
            name: "register_page_sign_in"
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
            name: "register_page_guest"
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
