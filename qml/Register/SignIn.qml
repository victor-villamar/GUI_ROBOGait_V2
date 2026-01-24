import QtQuick 2.15
import "qrc:/Dialogs"

SignInForm {
    id: root

    signal goToLogin()
    signal goToGuest()

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

    loginButton.onClicked: {
        clear(); goToLogin()
    }
    guestButton.onClicked: {
        clear(); goToGuest()
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
        if (password.length < 8) {
            showError(qsTr("Error: La contraseña debe tener al menos 8 caracteres"))
            return
        }
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
