import QtQuick 2.15

SignInForm {
    id: singin
    
    ErrorRectangle {
        id: errorPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: "Error: Has dejado campos vacíos"
        visible: false
    }

    sip_login_button.onClicked: {
        clear()
        register_page.login_push()
    }

    sip_guest_button.onClicked: {
        clear()
        register_page.guest_push()
    }

    sip_singIn_button.onClicked: {
        if (nameField.text === "" || lastnameField.text === "" || usernameField.text === "" || passwordField.text === "" || repeatpasswordField.text === "") {
            errorPopup.errorRectangleTextError.text = "Error: Has dejado campos vacíos"
            errorPopup.visible = true
        } else if (!/^[a-zA-ZáéíóúÁÉÍÓÚñÑ\s]+$/.test(nameField.text)) {
            errorPopup.errorRectangleTextError.text = "Error: El nombre solo debe contener letras"
            errorPopup.visible = true
        } else if (!/^[a-zA-ZáéíóúÁÉÍÓÚñÑ\s]+$/.test(lastnameField.text)) {
            errorPopup.errorRectangleTextError.text = "Error: El apellido solo debe contener letras"
            errorPopup.visible = true
        } else if (passwordField.text.length < 8) {
            errorPopup.errorRectangleTextError.text = "Error: La contraseña debe tener al menos 8 caracteres"
            errorPopup.visible = true
        } else if (passwordField.text !== repeatpasswordField.text) {
            errorPopup.errorRectangleTextError.text = "Error: Las contraseñas no coinciden"
            errorPopup.visible = true
        } else if (roleSelector.currentText === "Selecciona un rol") {
            console.log(roleSelector.currentText)
            errorPopup.errorRectangleTextError.text = "Error: Selecciona un rol"
            errorPopup.visible = true
        } else {
            errorPopup.visible = false
            console.log("Registrarse presionado")
            ddbb.checkUsername(usernameField.text);
        }
    }
    Connections {
        target: ddbb
        function onPassCheckUsernameChanged() {
            console.log("⚡ passCheckUsername cambió a:", ddbb.passCheckUsername);
            if (ddbb.passCheckUsername === false) {
                errorPopup.errorRectangleTextError.text = "Error: El nombre de usuario ya existe"
                errorPopup.visible = true
            } else {
                errorPopup.visible = false
                console.log("Registrandose")
                // Proceed with registration
                ddbb.signIn(nameField.text, lastnameField.text, usernameField.text,roleSelector.currentText, passwordField.text)
                register_page.login_push()
                clear()
            }
        }
    }
    function clear()
    {
        passwordField.text = ""
        repeatpasswordField.text = ""
        lastnameField.text = ""
        usernameField.text = ""
        nameField.text = ""
        roleSelector.currentIndex = 0
    }
}
