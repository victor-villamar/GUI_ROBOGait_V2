import QtQuick 2.15
import "qrc:/Dialogs"

SignInForm {
    id: root

    signal goToLogin()
    signal goToGuest()

    onVisibleChanged: {
        if (visible) {
            authModeDropdown.currentMode = "sign_in"
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

    function clear() {
        nameField.text = ""
        lastNameField.text = ""
        userNameField.text = ""
        passwordField.text = ""
        repeatPasswordField.text = ""
        roleCombo.currentIndex = 0
    }

    authModeDropdown.onModeSelected: function(mode) {
        if (mode === "login") {
            clear(); goToLogin()
        } else if (mode === "guest") {
            clear(); goToGuest()
        } else {
            authModeDropdown.currentMode = "sign_in"
        }
    }

    signInButton.onClicked: {
        var name = nameField.text.trim()
        var lastName = lastNameField.text.trim()
        var userName = userNameField.text.trim()
        var password = passwordField.text
        var repeatPassword = repeatPasswordField.text
        var role = roleCombo.currentText

        if (name.length === 0 || lastName.length === 0 || userName.length === 0 || password.length === 0 || repeatPassword.length === 0) {
            showError(qsTr("Error: Has dejado campos vacíos"))
            return
        }
        if (!/^[a-zA-ZáéíóúÁÉÍÓÚñÑ\s]+$/.test(name) || !/^[a-zA-ZáéíóúÁÉÍÓÚñÑ\s]+$/.test(lastName)) {
            showError(qsTr("Error: Nombre y apellidos solo letras y espacios"))
            return
        }
        // TODO: Restore the password length limit once the develivery is completed
        // if (password.length < 8) {
        //     showError(qsTr("Error: La contraseña debe tener al menos 8 caracteres"))
        //     return
        // }
        if (password !== repeatPassword) {
            showError(qsTr("Error: Las contraseñas no coinciden"))
            return
        }

        var available = dbManager.checkUserNameAvailable(userName)
        if (!available) {
            showError(qsTr("Error: El nombre de usuario ya existe"))
            return
        }

        var ok = dbManager.registerUser(name, lastName, userName, password, role)
        if (!ok) {
            showError(dbManager.lastError && dbManager.lastError.length ? dbManager.lastError : qsTr("Error al registrar"))
            return
        }

        clear()
        goToLogin()
    }
}
