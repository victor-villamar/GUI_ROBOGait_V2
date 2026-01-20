import QtQuick 2.15
import "extras"


Item {
    id: register_page
    state: "rp_login"

    Login {
            anchors.fill: parent
        id: login
    }
    SignIn {
            anchors.fill: parent
        id: sign_in
    }
    Guest {
            anchors.fill: parent
        id: guest
    }

    states: [
        State {
            name: "rp_login"
            StateChangeScript {
                script: console.log("estoy en login")
            }
            PropertyChanges {
                target: login
                visible: true
                enabled: true
            }
            PropertyChanges {
                target: sign_in
                visible: false
                enabled: false
            }
            PropertyChanges {
                target: guest
                visible: false
                enabled: false
            }
        },
        State {
            name: "rp_sign_in"
            StateChangeScript {
                script: console.log("estoy en sign_in")
            }
            PropertyChanges {
                target: sign_in
                visible: true
                enabled: true
            }
            PropertyChanges {
                target: login
                visible: false
                enabled: false
            }
            PropertyChanges {
                target: guest
                visible: false
                enabled: false
            }
        },
        State {
            name: "rp_guest"
            StateChangeScript {
                script: console.log("estoy en guest")
            }
            PropertyChanges {
                target: guest
                visible: true
                enabled: true
            }
            PropertyChanges {
                target: sign_in  // Aquí estaba el error
                visible: false
                enabled: false
            }
            PropertyChanges {
                target: login
                visible: false
                enabled: false
            }
        }
    ]

    function guest_push() {
        register_page.state = "rp_guest"
    }

    function login_push() {
        register_page.state = "rp_login"
    }

    function sign_in_push() {
        register_page.state = "rp_sign_in"
    }
}
