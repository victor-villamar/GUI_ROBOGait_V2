import QtQuick 2.15

SelectPatientForm {
    addPatient.onClicked: {
        listView.currentIndex = -1
        descriptionPatient.visible = false
        buttonRow.visible = false
        applicationFlow.register_patient_push()
    }
    buttonAccept.onClicked: {
        stringHandler.patientName = ddbb.patient.last_name + ", " + ddbb.patient.first_name
        applicationFlow.popFunction()
    }
    buttonCancel.onClicked: {
        listView.currentIndex = -1
        descriptionPatient.visible = false
        buttonRow.visible = false
    }

}
