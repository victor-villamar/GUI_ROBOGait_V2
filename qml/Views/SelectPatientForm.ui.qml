import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root

    property alias addPatientButton: addPatientButton
    property alias patientsListView: patientsListView
    property alias patientsScrollView: patientsScrollView
    property int selectedPatientId: -1

    signal addPatientRequested()
    signal patientClicked(int patientId, string display)
    signal patientDeleteRequested(int patientId, string display)

    Rectangle {
        anchors.fill: parent
        color: "#518bb7"

        Text {
            id: titleText
            text: qsTr("SELECCIONE UN PACIENTE")
            color: "#ffffff"
            font.pixelSize: 30
            horizontalAlignment: Text.AlignHCenter
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 20
        }

        Button {
            id: addPatientButton
            width: patientsScrollView.width
            height: 60
            anchors.top: titleText.bottom
            anchors.topMargin: 30
            anchors.horizontalCenter: parent.horizontalCenter

            background: Rectangle {
                color: "#ffffff"
                border.color: "#cccccc"
                radius: 15
            }

            contentItem: Item {
                anchors.fill: parent

                Text {
                    text: qsTr("Añadir Paciente")
                    font.pixelSize: 22
                    color: "#000000"
                    anchors.centerIn: parent
                }

                Image {
                    source: "qrc:/qmlresources/icons/plus_solid.svg"
                    sourceSize.width: 25
                    sourceSize.height: 25
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 20
                }
            }

            onClicked: root.addPatientRequested()
        }

        ScrollView {
            id: patientsScrollView
            width: Math.min(720, parent.width * 0.70)
            anchors.top: addPatientButton.bottom
            anchors.topMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true

            ListView {
                id: patientsListView
                currentIndex: -1
                anchors.fill: parent
                clip: true

                delegate: Item {
                    width: patientsListView.width
                    height: 70

                    Rectangle {
                        id: rowRect
                        width: parent.width
                        height: 60
                        radius: 15
                        color: patient_id === root.selectedPatientId ? "#aed2ea" : "#ffffff"
                        border.color: "#cccccc"

                        Text {
                            text: patient_display
                            font.pixelSize: 22
                            color: "#000000"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                root.patientClicked(patient_id, patient_display)
                            }
                        }

                        Button {
                            id: deletePatientButton
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 20

                            background: Rectangle { color: "transparent" }

                            contentItem: Image {
                                source: "qrc:/qmlresources/icons/trash_solid.svg"
                                sourceSize.width: 25
                                sourceSize.height: 25
                                fillMode: Image.PreserveAspectFit
                            }

                            onClicked: root.patientDeleteRequested(patient_id, patient_display)
                        }
                    }
                }
            }
        }
    }
}
