import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"

Dialog {
    id: root
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose

    parent: Overlay.overlay

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: "#A0505050"
    }

    width: parent ? Math.min(580, parent.width * 0.90) : 580
    height: parent ? Math.min(610, parent.height * 0.90) : 610

    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0

    signal patientRegistered()

    function resetForm()
    {
        nameField.text = ""
        lastNameField.text = ""
        ageField.text = ""
        weightField.text = ""
        heightField.text = ""
        descriptionField.text = ""
    }

    onOpened: resetForm()

    background: Rectangle {
        color: "#518bb7"
        border.color: "#ffffff"
        border.width: 2
        radius: 12
    }

    contentItem: Item {
        anchors.fill: parent

        Button {
            id: closeButton
            width: 28
            height: 28
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 12
            anchors.rightMargin: 12

            background: Rectangle {
                color: closeButton.down ? "#a9cfe8" : "#ffffff"
                radius: 8
                border.color: "#045671"
                border.width: 2
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/xmark_solid.svg"
                fillMode: Image.PreserveAspectFit
                sourceSize.width: 14
                sourceSize.height: 14
                anchors.centerIn: parent
            }

            onClicked: root.close()
        }

        Column {
            id: formColumn
            width: Math.min(450, parent.width - 64)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 40
            spacing: 15
            padding: 20

            Text {
                text: qsTr("Registrar paciente")
                color: "#ffffff"
                font.pixelSize: 40
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
                padding: 10
            }

            TextField {
                id: nameField
                width: parent.width
                height: 40
                color: "#000000"
                font.pointSize: 15
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 10
                rightPadding: 10
                placeholderText: qsTr("Nombre")

                background: Rectangle {
                    radius: 10
                    color: "#FFFFFF"
                    border.color: "#CCCCCC"
                }
            }

            TextField {
                id: lastNameField
                width: parent.width
                height: 40
                color: "#000000"
                font.pointSize: 15
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 10
                rightPadding: 10
                placeholderText: qsTr("Apellidos")

                background: Rectangle {
                    radius: 10
                    color: "#FFFFFF"
                    border.color: "#CCCCCC"
                }
            }

            TextField {
                id: ageField
                width: parent.width
                height: 40
                color: "#000000"
                font.pointSize: 15
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 10
                rightPadding: 10
                inputMethodHints: Qt.ImhDigitsOnly
                placeholderText: qsTr("Edad (años)")

                background: Rectangle {
                    radius: 10
                    color: "#FFFFFF"
                    border.color: "#CCCCCC"
                }
            }

            TextField {
                id: weightField
                width: parent.width
                height: 40
                color: "#000000"
                font.pointSize: 15
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 10
                rightPadding: 10
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                placeholderText: qsTr("Peso (Kg)")

                background: Rectangle {
                    radius: 10
                    color: "#FFFFFF"
                    border.color: "#CCCCCC"
                }
            }

            TextField {
                id: heightField
                width: parent.width
                height: 40
                color: "#000000"
                font.pointSize: 15
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 10
                rightPadding: 10
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                placeholderText: qsTr("Altura (cm)")

                background: Rectangle {
                    radius: 10
                    color: "#FFFFFF"
                    border.color: "#CCCCCC"
                }
            }

            TextArea {
                id: descriptionField
                width: parent.width
                height: 120
                color: "#000000"
                font.pointSize: 15
                wrapMode: TextEdit.Wrap
                placeholderText: qsTr("Descripción")

                background: Rectangle {
                    radius: 10
                    color: "#FFFFFF"
                    border.color: "#CCCCCC"
                }
            }

            Button {
                id: addButton
                width: 240
                height: 44
                anchors.horizontalCenter: parent.horizontalCenter

                background: Rectangle {
                    radius: 10
                    color: "#aed2ea"
                    border.color: "#aed2ea"
                }

                contentItem: Label {
                    text: qsTr("AÑADIR")
                    color: "#ffffff"
                    font.bold: true
                    font.pixelSize: 20
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (!dbManager) {
                        return
                    }

                    var age = parseInt(ageField.text)
                    var weight = parseFloat(weightField.text)
                    var height = parseFloat(heightField.text)

                    if (!nameField.text || !lastNameField.text) {
                        errorPopup.errorRectangleTextError.text = qsTr("Error: Has dejado campos vacíos")
                        errorPopup.open()
                        return
                    }

                    if (isNaN(age) || isNaN(weight) || isNaN(height)) {
                        errorPopup.errorRectangleTextError.text = qsTr("Error: Edad, peso y altura deben ser números válidos")
                        errorPopup.open()
                        return
                    }

                    var ok = dbManager.registerPatient(
                                nameField.text,
                                lastNameField.text,
                                age,
                                weight,
                                height,
                                descriptionField.text)

                    if (!ok) {
                        errorPopup.errorRectangleTextError.text = qsTr("Error: %1").arg(dbManager.lastError)
                        errorPopup.open()
                        return
                    }

                    root.close()
                    root.patientRegistered()
                }
            }
        }

        ErrorRectangle {
            id: errorPopup
            anchors.centerIn: parent
            errorRectangleTextError.text: ""
        }
    }
}
