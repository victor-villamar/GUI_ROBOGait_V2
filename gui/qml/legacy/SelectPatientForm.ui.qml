import QtQuick 2.15
import QtQuick.Controls 2.15
import "extras"

Rectangle {
    id: rectangle
    color: "#518bb7"
    property alias buttonAccept: buttonAccept
    property alias listView: listView
    property alias buttonRow: buttonRow
    property alias buttonCancel: buttonCancel
    property alias descriptionPatient: descriptionPatient
    property alias addPatient: addPatient

    // Título
    Text {
        id: text1
        text: qsTr("SELECCIONE UN PACIENTE")
        color: "#ffffff"
        font.pixelSize: 30
        horizontalAlignment: Text.AlignHCenter
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 20
    }

    Button {
        id: addPatient
        width: scrollView.width
        height: 60
        anchors.left: parent.left
        anchors.top: text1.bottom
        anchors.leftMargin: 20
        anchors.topMargin: 30
        background: Rectangle {
            color: "#ffffff"
            border.color: "#cccccc"
            radius: 15
        }

        Text {
            id: textMapName
            text: qsTr("Añadir Paciente")
            font.pixelSize: 22
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#000000"
            anchors.verticalCenter: parent.verticalCenter
        }

        Button {
            id: mapDelete
            background: Image {
                source: "../images/icon_app/plus-solid.svg"
                sourceSize.width: 25
                sourceSize.height: 25
                fillMode: Image.PreserveAspectFit
            }

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 20
        }
    }

    // ScrollView que contiene la lista, centrado en la pantalla
    ScrollView {
        id: scrollView
        width: parent.width * 0.5
        anchors.left: parent.left
        anchors.top: addPatient.bottom
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.topMargin: 10
        anchors.leftMargin: 20

        ListView {
            id: listView
            currentIndex: -1
            anchors.fill: parent
            clip: true
            model: ddbb.patients

            // model: ListModel {
            //     ListElement {
            //         display: "Paciente 1"
            //     }
            //     ListElement {
            //         display: "Paciente 2"
            //     }
            //     ListElement {
            //         display: "Paciente 3"
            //     }
            //     ListElement {
            //         display: "Paciente 4"
            //     }
            //     ListElement {
            //         display: "Paciente 5"
            //     }
            //     ListElement {
            //         display: "Paciente 6"
            //     }
            //     ListElement {
            //         display: "Paciente 7"
            //     }
            // }
            delegate: Item {
                width: listView.width
                height: 70

                Button {
                    id: mapButton
                    width: parent.width
                    height: 60
                    background: Rectangle {
                        color: index === listView.currentIndex ? "#aed2ea" : "#ffffff"
                        border.color: "#cccccc"
                        radius: 15
                    }

                    onClicked: {
                        listView.currentIndex = index
                        // console.log("Presionado un item " + index)
                        ddbb.getIdFromName(model.display) // Mostrará el texto asociado al botón
                        descriptionPatient.visible = true
                        buttonRow.visible = true
                        // applicationFlow.menu_push()
                    }
                    Text {
                        id: textAddPatient
                        text: display
                        font.pixelSize: 22
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: "#000000"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    // // Botón de Editar
                    // Button {
                    //     id: mapEdit
                    //     text: "Editar"
                    //     anchors.verticalCenter: parent.verticalCenter
                    //     anchors.right: mapDelete.left
                    //     anchors.rightMargin: 20
                    //     //                                                        onClicked: console.log("Editar:", model.name)
                    // }

                    // Botón de Borrar
                    Button {
                        id: deletePatient
                        background: Image {
                            source: "../images/icon_app/trash-solid.svg"
                            sourceSize.width: 25
                            sourceSize.height: 25
                            fillMode: Image.PreserveAspectFit
                        }

                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 20
                    }
                }
            }
        }
    }
    DescriptionPatientForm {
        id: descriptionPatient
        visible: false
        anchors.left: scrollView.right
        anchors.right: parent.right
        anchors.top: text1.bottom
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 140
        anchors.topMargin: 30
        anchors.rightMargin: 40
        anchors.leftMargin: 40
    }

    Row {
        id: buttonRow
        visible: false
        spacing: parent.width > 500 ? 40 : 20
        anchors.horizontalCenter: descriptionPatient.horizontalCenter
        anchors.top: descriptionPatient.bottom
        anchors.topMargin: 20
        anchors.bottomMargin: 40

        Button {
            id: buttonCancel
            width: 140
            height: 40

            background: Rectangle {
                color: buttonCancel.down ? "#e9e9e9" : "#aed2ea"
                radius: 15
                border.color: "#e9e9e9"
                border.width: 2
            }

            contentItem: Label {
                id: buttonLabelCancel
                text: qsTr("CANCELAR")
                color: "#518bb7"
                font.pixelSize: 20
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            // onClicked: {
            //     listView.currentIndex = -1
            //     descriptionPatient.visible = false
            //     buttonRow.visible = false
            // }
        }

        Button {
            id: buttonAccept
            width: 140
            height: 40

            background: Rectangle {
                color: buttonAccept.down ? "#e9e9e9" : "#aed2ea"
                radius: 15
                border.color: "#e9e9e9"
                border.width: 2
            }

            contentItem: Label {
                id: buttonLabelAccept
                text: qsTr("ACEPTAR")
                color: "#518bb7"
                font.pixelSize: 20
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:1}D{i:4}D{i:5}D{i:2}D{i:8}D{i:7}D{i:15}
D{i:17}D{i:20}D{i:16}
}
##^##*/

