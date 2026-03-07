import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"
import "qrc:/Common"

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
    height: keyboardHelper.computedHeight

    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? keyboardHelper.computedY : 0

    KeyboardAwareHelper {
        id: keyboardHelper
        target: root
        maxDialogHeight: parent ? Math.min(610, parent.height * 0.90) : 610
    }

    signal createMapRequested(string name, string location, string description)

    function resetForm() {
        nameField.text = ""
        locationField.text = ""
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
            z: 2
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

        ScrollView {
            id: scroll
            z: 1
            anchors.fill: parent
            anchors.margins: 16
            clip: true

            Column {
                id: formColumn
                width: Math.min(450, scroll.width - 32)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: Math.max(20, Math.round((scroll.height - formColumn.implicitHeight) / 2))
                spacing: 15
                padding: 20

                Text {
                    text: qsTr("DATOS DEL MAPA")
                    color: "#ffffff"
                    font.pixelSize: 40
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
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
                    placeholderText: qsTr("Nombre del mapa")

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: "#CCCCCC"
                    }
                }

                TextField {
                    id: locationField
                    width: parent.width
                    height: 40
                    color: "#000000"
                    font.pointSize: 15
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    rightPadding: 10
                    placeholderText: qsTr("Localización del mapa")

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

                Item {
                    width: parent.width
                    height: buttonsRow.implicitHeight

                    Row {
                        id: buttonsRow
                        spacing: 20
                        anchors.horizontalCenter: parent.horizontalCenter

                        Button {
                            id: createButton
                            width: 215
                            height: 40

                            background: Rectangle {
                                radius: 10
                                color: "#aed2ea"
                                border.color: "#aed2ea"
                            }

                            contentItem: Label {
                                text: qsTr("HACER MAPA")
                                color: "#ffffff"
                                font.bold: true
                                font.pixelSize: 19
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                if (nameField.text.trim() === "" ||
                                    locationField.text.trim() === "" ||
                                    descriptionField.text.trim() === "") {
                                    errorPopup.errorRectangleTextError.text = qsTr("Error: Has dejado campos vacíos")
                                    errorPopup.open()
                                    return
                                }

                                root.close()
                                root.createMapRequested(nameField.text, locationField.text, descriptionField.text)
                            }
                        }

                        Button {
                            id: cancelButton
                            width: 215
                            height: 40

                            background: Rectangle {
                                radius: 10
                                color: "#aed2ea"
                                border.color: "#aed2ea"
                            }

                            contentItem: Label {
                                text: qsTr("CANCELAR")
                                color: "#ffffff"
                                font.bold: true
                                font.pixelSize: 19
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: root.close()
                        }
                    }
                }
            }
        }

        ErrorRectangle {
            id: errorPopup
            z: 3
            anchors.centerIn: parent
            errorRectangleTextError.text: ""
        }
    }
}
