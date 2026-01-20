import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: descriptionPatientForm
    color: "#e9e9e9"
    radius: 15
    border.color: "#235c87"
    border.width: 2


    property var patientData: {
        "first_name": "Desconocido",
        "last_name": "Desconocido",
        "user_name": "Desconocido",
        "user_lastname": "Desconocido",
        "create_day": "No disponible",
        "age": "No disponible",
        "weight": "No disponible",
        "height": "No disponible",
        "description": "No disponible"
    } // Default values for patient data
    

    Column {
        anchors.fill: parent
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 15
        spacing: 10

        Text {
            id: doctorText
            text: qsTr("Doctor: ") + (patientData.user_name || "") + " " + (patientData.user_lastname || "")
            font.pixelSize: 16
        }
        Text {
            id: creationDateText
            text: qsTr("Fecha de creación: ") + (patientData.create_day || "")
            wrapMode: Text.Wrap
            width: parent.width - 40
            font.pixelSize: 16
        }
        Text {
            id: fullNameText
            text: qsTr("Nombre completo: ") + (patientData.first_name || "") + " " + (patientData.last_name || "")
            font.pixelSize: 16
        }
        Text {
            id: ageText
            text: qsTr("Edad: ") + (patientData.age || "") + qsTr(" años")
            font.pixelSize: 16
        }
        Text {
            id: weightText
            text: qsTr("Peso: ") + (patientData.weight || "") + qsTr(" Kg")
            font.pixelSize: 16
        }
        Text {
            id: heightText
            text: qsTr("Altura: ") + (patientData.height || "") + qsTr(" cm")
            font.pixelSize: 16
        }
        Text {
            id: descriptionText
            text: qsTr("Descripción: ") + (patientData.description || "")
            font.pixelSize: 16
            wrapMode: Text.Wrap
            width: parent.width - 40
        }
    }

    Connections {
        target: ddbb // Replace with the actual object that emits the signal
        function onPatientChanged() {
            descriptionPatientForm.patientData = ddbb.patient;
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:280;width:340}D{i:2}D{i:3}D{i:4}D{i:5}D{i:6}D{i:7}D{i:8}
D{i:1}
}
##^##*/

