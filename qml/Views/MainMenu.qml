import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"
import "qrc:/Views"

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
        if (StackView.view) {
            StackView.view.push(manualControlPage)
        }
    }

    selectMapArea.onClicked: {
        showNotImplemented(qsTr("Seleccionar mapa: en desarrollo."))
    }

    selectPatientArea.onClicked: {
        if (StackView.view) {
            StackView.view.push(selectPatientPage)
        }
    }

    testArea.onClicked: {
        showNotImplemented(qsTr("Prueba: en desarrollo."))
    }

    Component {
        id: selectPatientPage
        SelectPatient { }
    }

    Component {
        id: manualControlPage
        ManualControl { }
    }
}
