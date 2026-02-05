import QtQuick 2.15
import "qrc:/Dialogs"

LoginForm {
    id: root

    signal goToRegister()
    signal goToGuest()
    signal authenticated()

    onVisibleChanged: {
        if (visible) {
            authModeDropdown.currentMode = "login"
        }
    }

    ErrorRectangle {
        id: errorPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: ""
    }

    function showError(msg) {
        errorPopup.errorRectangleTextError.text = msg
        errorPopup.open()
    }

    function clearFields() {
        usernameField.text = ""
        passwordField.text = ""
    }

    authModeDropdown.onModeSelected: function(mode) {
        if (mode === "sign_in") {
            clearFields()
            goToRegister()
        } else if (mode === "guest") {
            clearFields()
            goToGuest()
        } else {
            authModeDropdown.currentMode = "login"
        }
    }

    loginButton.onClicked: {
        var userName = usernameField.text.trim()
        var password = passwordField.text

        if (userName.length === 0 || password.length === 0) {
            showError(qsTr("Error: Has dejado campos vacíos"))
            return
        }

        var ok = userSession.loginUser(userName, password)
        if (ok) {
            clearFields()
            authenticated()
        } 
        else {
            var errorMsg = dbManager.lastError && dbManager.lastError.length ? 
                          dbManager.lastError : 
                          qsTr("Nombre de usuario o contraseña incorrectos.")
            showError(errorMsg)
        }
    }
}
