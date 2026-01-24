import QtQuick 2.15

RegisterPageForm {
    id: registerPage
    state: "register_page_login"

    signal authenticated()

    function updateBackButtonVisibility()
    {
        if (!applicationFlow || !applicationFlow.toolbar || !applicationFlow.toolbar.backButton)
        {
            return
        }

        applicationFlow.toolbar.backButton.visible = (registerPage.state === "register_page_login")
    }

    function loginPush()  { state = "register_page_login" }
    function signInPush() { state = "register_page_sign_in" }
    function guestPush()  { state = "register_page_guest" }

    onStateChanged: updateBackButtonVisibility()
    Component.onCompleted: updateBackButtonVisibility()

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
