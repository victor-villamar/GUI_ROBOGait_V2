import QtQuick 2.15
import "qrc:/Dialogs"

GuestForm {
    id: root

    signal goToLogin()
    signal goToRegister()
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

    function clear() {
        nameField.text = ""
        lastNameField.text = ""
    }

    loginButton.onClicked: {
        clear(); goToLogin()
    }
    registerButton.onClicked: {
        clear(); goToRegister()
    }

    guestButton.onClicked: {
        var name = nameField.text.trim()
        var lastName = lastNameField.text.trim()

        if (name.length === 0 || lastName.length === 0) {
            showError(qsTr("Error: Has dejado campos vacíos"))
            return
        }
        if (!/^[a-zA-ZáéíóúÁÉÍÓÚñÑ\s]+$/.test(name) || !/^[a-zA-ZáéíóúÁÉÍÓÚñÑ\s]+$/.test(lastName)) {
            showError(qsTr("Error: Nombre y apellidos solo letras y espacios"))
            return
        }

        dbManager.loginGuest(name)
        clear()
        authenticated()
    }
}
