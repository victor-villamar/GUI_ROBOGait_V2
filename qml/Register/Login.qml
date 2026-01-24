import QtQuick 2.15
import "qrc:/Dialogs"

LoginForm {
    id: root

    signal goToRegister()
    signal goToGuest()
    signal authenticated()

    ErrorRectangle {
        id: errorPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: ""
    }

    function showError(msg) {
        errorPopup.errorRectangleTextError.text = msg
        errorPopup.open()
    }

    registerButton.onClicked: {
        usernameField.text = ""
        passwordField.text = ""
        goToRegister()
    }

    guestButton.onClicked: {
        usernameField.text = ""
        passwordField.text = ""
        goToGuest()
    }

    loginButton.onClicked: {
        var userName = usernameField.text.trim()
        var password = passwordField.text

        if (userName.length === 0 || password.length === 0) {
            showError(qsTr("Error: Has dejado campos vacíos"))
            return
        }

        var ok = dbManager.login(userName, password)
        if (ok) {
            usernameField.text = ""
            passwordField.text = ""
            authenticated()
        } 
        else {
            showError(dbManager.lastError && dbManager.lastError.length ? dbManager.lastError
                                                                     : qsTr("Nombre de usuario o contraseña incorrectos."))
        }
    }
}
