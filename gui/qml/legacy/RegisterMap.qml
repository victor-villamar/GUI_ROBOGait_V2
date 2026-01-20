import QtQuick 2.15
import QtQuick.Controls 2.15
import "extras"

RegisterMapForm {
    id: registerPatientForm

    ErrorRectangle {
        id: errorPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: qsTr("Error: Has dejado campos vacíos")
        visible: false
    }

    rm_add_map.onClicked: {
        if (mapNameField.text === "" || locationField.text === "") {
            errorPopup.errorRectangleTextError.text = qsTr("Error: Has dejado campos vacíos")
            errorPopup.visible = true
        } else {
            errorPopup.visible = false
            console.log("Añadir información del mapa presionado")
            ddbb.setMapInformation(ddbb.username, mapNameField.text, locationField.text, descriptionField.text)
            applicationFlow.type_save = -1
            stringHandler.setSaveMap(0)
            applicationFlow.create_map_push()
        }
    }

    rm_cancel_test.onClicked: {
        applicationFlow.popFunction()
    }
}
