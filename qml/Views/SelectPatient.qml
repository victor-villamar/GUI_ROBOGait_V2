import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"

SelectPatientForm {
    id: root

    ListModel { id: patientsModel }

    patientsListView.model: patientsModel
    selectedPatientId: (userSession && userSession.currentPatient && userSession.currentPatient.isActive) ? userSession.currentPatient.id : -1

    property int pendingDeletePatientId: -1
    property string pendingDeletePatientDisplay: ""


    function refreshPatients()
    {
        patientsModel.clear()

        if (!dbManager) {
            return
        }

        if (dbManager.userRole === "guest") {
            return
        }

        var patients = dbManager.listPatients()
        for (var i = 0; i < patients.length; ++i) {
            var p = patients[i]
            patientsModel.append({
                "patient_id": p.id,
                "patient_display": p.display,
                "name": p.name,
                "last_name": p.last_name
            })
        }
    }

    StackView.onActivated: refreshPatients()

    Connections {
        target: dbManager
        ignoreUnknownSignals: true

        function onUserNameChanged() {
            refreshPatients()
        }

        function onUserRoleChanged() {
            if (dbManager.userRole === "guest") {
                patientsModel.clear()
            } else {
                refreshPatients()
            }
        }
    }

    onAddPatientRequested: patientRegisterDialog.open()

    onPatientClicked: function(patientId, display) {
        if (!dbManager) {
            return
        }

        var details = dbManager.getPatientDetails(patientId)
        patientDetailsDialog.openForPatient(patientId, display, details)
    }

    onPatientDeleteRequested: function(patientId, display) {
        pendingDeletePatientId = patientId
        pendingDeletePatientDisplay = display
        deleteConfirmDialog.message = qsTr("¿Borrar el paciente %1?").arg(display)
        deleteConfirmDialog.openWithMessage(deleteConfirmDialog.message)
    }

    PatientDetailsDialog {
        id: patientDetailsDialog

        onAcceptedSelection: function(patientId, display) {
            if (userSession) {
                var basicInfo = dbManager ? dbManager.getPatientBasicInfo(patientId) : null
                userSession.assignPatient(patientId,
                                          basicInfo ? basicInfo.name : "",
                                          basicInfo ? basicInfo.last_name : "",
                                          display)
            }

            if (root.StackView.view) {
                root.StackView.view.pop()
            }
        }
    }

    PatientRegisterDialog {
        id: patientRegisterDialog
        onPatientRegistered: refreshPatients()
    }

    ConfirmationDialog {
        id: deleteConfirmDialog
        holdToAccept: true
        acceptText: qsTr("Borrar")

        onAccepted: {
            if (!dbManager) {
                return
            }

            var ok = dbManager.deletePatient(pendingDeletePatientId)
            if (!ok) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: %1").arg(dbManager.lastError)
                errorPopup.open()
                return
            }

            refreshPatients()
        }
    }

    ErrorRectangle {
        id: errorPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: ""
    }
}
