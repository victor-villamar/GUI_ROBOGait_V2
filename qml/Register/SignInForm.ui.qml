import QtQuick 2.15
import QtQuick.Controls 2.15

Page {
    id: root

    property alias nameField: nameField
    property alias lastNameField: lastNameField
    property alias userNameField: userNameField
    property alias roleCombo: roleCombo
    property alias passwordField: passwordField
    property alias repeatPasswordField: repeatPasswordField

    property alias signInButton: signInButton
    property alias loginButton: loginButton
    property alias guestButton: guestButton

    Rectangle {
        anchors.fill: parent
        color: "#518bb7"

        ScrollView {
            id: scroll
            anchors.fill: parent
            anchors.topMargin: 20
            anchors.bottomMargin: 30
            clip: true

            Item {
                id: content
                width: scroll.width
                property int topPadding: 40
                height: Math.max(formColumn.implicitHeight + (topPadding * 2), scroll.height)

                Column {
                    id: formColumn
                    width: Math.min(400, Math.max(0, content.width - 40))
                    spacing: 16
                    anchors.top: parent.top
                    anchors.topMargin: content.topPadding
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        color: "#ffffff"
                        font.pixelSize: 26
                        font.bold: true
                        text: qsTr("REGISTRO")
                        horizontalAlignment: Text.AlignHCenter
                        width: formColumn.width
                    }

                    TextField {
                        id: nameField
                        width: formColumn.width
                        height: 50
                        font.pointSize: 20
                        color: "#000000"
                        placeholderTextColor: "#808080"
                        placeholderText: qsTr("Nombre")
                        background: Rectangle {
                            radius: 10
                            color: "#ffffff"
                            border.color: "#CCCCCC"
                        }
                    }

                    TextField {
                        id: lastNameField
                        width: formColumn.width
                        height: 50
                        font.pointSize: 20
                        color: "#000000"
                        placeholderTextColor: "#808080"
                        placeholderText: qsTr("Apellidos")
                        background: Rectangle {
                            radius: 10
                            color: "#ffffff"
                            border.color: "#CCCCCC"
                        }
                    }

                    TextField {
                        id: userNameField
                        width: formColumn.width
                        height: 50
                        font.pointSize: 20
                        color: "#000000"
                        placeholderTextColor: "#808080"
                        placeholderText: qsTr("Nombre de usuario")
                        background: Rectangle {
                            radius: 10
                            color: "#ffffff"
                            border.color: "#CCCCCC"
                        }
                    }

                    ComboBox {
                        id: roleCombo
                        width: formColumn.width
                        height: 50
                        model: [ "Doctor", "Manager" ]
                    }

                    TextField {
                        id: passwordField
                        width: formColumn.width
                        height: 50
                        font.pointSize: 20
                        color: "#000000"
                        placeholderTextColor: "#808080"
                        echoMode: TextInput.Password
                        placeholderText: qsTr("Contraseña (mín. 8)")
                        background: Rectangle {
                            radius: 10
                            color: "#ffffff"
                            border.color: "#CCCCCC"
                        }
                    }

                    TextField {
                        id: repeatPasswordField
                        width: formColumn.width
                        height: 50
                        font.pointSize: 20
                        color: "#000000"
                        placeholderTextColor: "#808080"
                        echoMode: TextInput.Password
                        placeholderText: qsTr("Repetir contraseña")
                        background: Rectangle {
                            radius: 10
                            color: "#ffffff"
                            border.color: "#CCCCCC"
                        }
                    }

                    Button {
                        id: signInButton
                        width: formColumn.width
                        height: 50
                        text: qsTr("Crear cuenta")
                        font.capitalization: Font.AllUppercase
                        font.bold: true
                        font.pointSize: 20
                        background: Rectangle {
                            radius: 10
                            color: "#aed2ea"
                            border.color: "#aed2ea"
                        }
                    }

                    Row {
                        width: formColumn.width
                        spacing: 20

                        Button {
                            id: loginButton
                            width: Math.max(0, (parent.width - parent.spacing) / 2)
                            height: 50
                            text: qsTr("Login")
                            font.capitalization: Font.AllUppercase
                            font.pointSize: 18
                            background: Rectangle {
                                radius: 10
                                color: "#c2d9e9"
                                border.color: "#c2d9e9"
                            }
                        }

                        Button {
                            id: guestButton
                            width: Math.max(0, (parent.width - parent.spacing) / 2)
                            height: 50
                            text: qsTr("Invitado")
                            font.capitalization: Font.AllUppercase
                            font.pointSize: 18
                            background: Rectangle {
                                radius: 10
                                color: "#c2d9e9"
                                border.color: "#c2d9e9"
                            }
                        }
                    }
                }
            }
        }
    }
}
