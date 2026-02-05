import QtQuick 2.15
import "qrc:/Dialogs"

GuestForm {
    id: root

    signal goToLogin()
    signal goToRegister()
    signal authenticated()

    onVisibleChanged: {
        if (visible) {
            authModeDropdown.currentMode = "guest"
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
    }

    authModeDropdown.onModeSelected: function(mode) {
        if (mode === "login") {
            clear(); goToLogin()
        } else if (mode === "sign_in") {
            clear(); goToRegister()
        } else {
            authModeDropdown.currentMode = "guest"
        }
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
