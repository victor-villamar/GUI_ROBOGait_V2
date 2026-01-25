import QtQuick 2.15

import "qrc:/Dialogs"

MainMenuForm {
    id: root

    ErrorRectangle {
        id: infoPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: ""
    }

    function showNotImplemented(message) {
        infoPopup.errorRectangleTextError.text = message
        infoPopup.open()
    }

    manualControlArea.onClicked: {
        showNotImplemented(qsTr("Control manual: no implementado todavía."))
    }

    selectMapArea.onClicked: {
        showNotImplemented(qsTr("Seleccionar mapa: no implementado todavía."))
    }

    selectPatientArea.onClicked: {
        showNotImplemented(qsTr("Seleccionar paciente: no implementado todavía."))
    }

    testArea.onClicked: {
        showNotImplemented(qsTr("Prueba: no implementado todavía."))
    }
}
