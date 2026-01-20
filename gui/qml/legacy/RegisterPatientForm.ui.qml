import QtQuick 2.15
import QtQuick.Controls 2.15

Page {
    id: signinPage
    property alias su_add_user: addInformation
    property alias su_cancel_test: cancelRegisterPatient
    property alias nameField: nameField
    property alias lastnameField: lastnameField
    property alias ageField: ageField
    property alias weightField: weightField
    property alias heightField: heightField
    property alias descriptionField: descriptionField

    // property int keyboardHeight: Qt.inputMethod.visible ? (parent.height - Qt.inputMethod.keyboardRectangle.height + 20) : 0

    Rectangle {
        id: rectangle
        color: "#518bb7"
        anchors.fill: parent

        ScrollView {
            id: formScroll
            width: 560
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: 20
            anchors.bottomMargin: 70
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true
            // ScrollBar.vertical.policy: ScrollBar.AsNeeded
            // bottomPadding: keyboardHeight > 0 ? keyboardHeight : 0

            Behavior on bottomPadding {
                NumberAnimation {
                    duration: 250
                }
            }
            Column {
                id: column
                width: 450
                anchors.top: parent.top
                anchors.topMargin: 0
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 15
                padding: 50

                Text {
                    id: text1
                    color: "#ffffff"
                    text: qsTr("Registrar paciente")
                    font.pixelSize: 40
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.styleName: "Medium"
                    padding: 10
                }

                TextField {
                    id: nameField
                    width: 450
                    height: 40
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 15
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    rightPadding: 10

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: "#CCCCCC"

                        Text {
                            text: qsTr("Nombre")
                            color: "#808080"
                            anchors {
                                right: parent.right
                                rightMargin: 15
                                verticalCenter: parent.verticalCenter
                            }
                            font: nameField.font
                        }
                    }
                    Keys.onTabPressed: lastnameField.focus = true
                    Keys.onReturnPressed: lastnameField.focus = true
                }

                TextField {
                    id: lastnameField
                    width: 450
                    height: 40
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 15
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    rightPadding: 10

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: "#CCCCCC"

                        Text {
                            text: qsTr("Apellidos")
                            color: "#808080"
                            anchors {
                                right: parent.right
                                rightMargin: 15
                                verticalCenter: parent.verticalCenter
                            }
                            font: lastnameField.font
                        }
                    }
                    Keys.onTabPressed: ageField.focus = true
                    Keys.onReturnPressed: ageField.focus = true
                }

                TextField {
                    id: ageField
                    width: 450
                    height: 40
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 15
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    rightPadding: 10

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: "#CCCCCC"

                        Text {
                            text: qsTr("Edad (años)")
                            color: "#808080"
                            anchors {
                                right: parent.right
                                rightMargin: 15
                                verticalCenter: parent.verticalCenter
                            }
                            font: ageField.font
                        }
                    }
                    Keys.onTabPressed: weightField.focus = true
                    Keys.onReturnPressed: weightField.focus = true
                }

                TextField {
                    id: weightField
                    width: 450
                    height: 40
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 15
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    rightPadding: 10

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: "#CCCCCC"

                        Text {
                            text: qsTr("Peso (Kg)")
                            color: "#808080"
                            anchors {
                                right: parent.right
                                rightMargin: 15
                                verticalCenter: parent.verticalCenter
                            }
                            font: lastnameField.font
                        }
                    }
                    Keys.onTabPressed: heightField.focus = true
                    Keys.onReturnPressed: heightField.focus = true
                }

                TextField {
                    id: heightField
                    width: 450
                    height: 40
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 15
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    rightPadding: 10

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: "#CCCCCC"

                        Text {
                            text: qsTr("Altura (cm)")
                            color: "#808080"
                            anchors {
                                right: parent.right
                                rightMargin: 15
                                verticalCenter: parent.verticalCenter
                            }
                            font: lastnameField.font
                        }
                    }
                    Keys.onTabPressed: descriptionField.focus = true
                    Keys.onReturnPressed: descriptionField.focus = true
                }

                TextField {
                    id: descriptionField
                    width: 450
                    height: 150
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 15
                    topPadding: 15
                    leftPadding: 15
                    rightPadding: 15
                    bottomPadding: 15
                    verticalAlignment: TextInput.AlignTop
                    horizontalAlignment: TextInput.AlignLeft
                    wrapMode: TextField.Wrap
                    selectByMouse: true

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: descriptionField.activeFocus ? "#518bb7" : "#CCCCCC"

                        Text {
                            visible: descriptionField.text === ""
                                     && !descriptionField.activeFocus
                            text: qsTr("Descripción")
                            color: "#808080"
                            anchors {
                                left: parent.left
                                leftMargin: 15
                                top: parent.top
                                topMargin: 15
                            }
                            font: lastnameField.font
                        }
                    }
                    Keys.onTabPressed: addInformation.focus = true
                    Keys.onReturnPressed: addInformation.focus = true
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20

                    Button {
                        id: addInformation
                        width: 215
                        height: 40
                        text: qsTr("AÑADIR")
                        font.styleName: "Medium"
                        font.capitalization: Font.AllUppercase
                        font.weight: Font.Light
                        font.bold: true
                        font.pointSize: 19
                        background: Rectangle {
                            radius: 10
                            color: "#aed2ea"
                            border.color: "#aed2ea"
                        }
                    }

                    Button {
                        id: cancelRegisterPatient
                        width: 215
                        height: 40
                        text: qsTr("CANCELAR")
                        font.styleName: "Medium"
                        font.capitalization: Font.AllUppercase
                        font.weight: Font.Normal
                        font.bold: true
                        font.pointSize: 19
                        background: Rectangle {
                            radius: 10
                            color: "#aed2ea"
                            border.color: "#aed2ea"
                        }
                    }
                }
            }
        }
    }
}
