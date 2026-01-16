import QtQuick 2.15
import QtQuick.Controls 2.15

Page {
    id: loginPage
    property alias lp_singin_button: lp_singin_button
    property alias lp_guest_button: lp_guest_button
    property alias lp_login_button: lp_login_button
    property alias usernameField: usernameField
    property alias passwordField: passwordField

    // Calcula la altura del teclado y ajusta el padding inferior dinámicamente
    // property int keyboardHeight: Qt.inputMethod.visible ? (parent.height - Qt.inputMethod.keyboardRectangle.height + 20) : 0

    Rectangle {
        id: rectangle
        color: "#518bb7"
        anchors.fill: parent

        ScrollView {
            id: scroll
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: 20
            anchors.bottomMargin: 30
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true
            // bottomPadding: keyboardHeight > 0 ? keyboardHeight : 0

            Behavior on bottomPadding {
                NumberAnimation {
                    duration: 250
                }
            }
            Column {
                id: column
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                spacing: 20
                padding: 50

                Image {
                    source: "../../images/logo.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 220
                    height: 220
                }

                TextField {
                    id: usernameField
                    width: 400
                    height: 50
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 20
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    rightPadding: 10

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: usernameField.activeFocus ? "#518bb7" : "#CCCCCC"

                        Text {
                            visible: usernameField.text === ""
                                     && !usernameField.activeFocus
                            text: qsTr("Usuario")
                            color: "#808080"
                            anchors.left: parent.left
                            anchors.leftMargin: 15
                            anchors.top: parent.top
                            anchors.topMargin: 15
                            font: usernameField.font
                        }
                    }

                    Keys.onTabPressed: passwordField.focus = true
                    Keys.onReturnPressed: passwordField.focus = true
                }

                TextField {
                    id: passwordField
                    width: 400
                    height: 50
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 20
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    rightPadding: 10
                    echoMode: TextInput.Password

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: passwordField.activeFocus ? "#518bb7" : "#CCCCCC"

                        Text {
                            visible: passwordField.text === ""
                                     && !passwordField.activeFocus
                            text: qsTr("Contraseña")
                            color: "#808080"
                            anchors.left: parent.left
                            anchors.leftMargin: 15
                            anchors.top: parent.top
                            anchors.topMargin: 15
                            font: passwordField.font
                        }
                    }
                }

                Button {
                    id: lp_login_button
                    width: 400
                    height: 50
                    text: "Iniciar Sesion"
                    font.capitalization: Font.AllUppercase
                    font.weight: Font.Medium
                    font.bold: true
                    font.pointSize: 28
                    background: Rectangle {
                        radius: 10
                        color: "#aed2ea"
                        border.color: "#aed2ea"
                    }
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20

                    Button {
                        id: lp_singin_button
                        text: "Registrarse"
                        font.weight: Font.Light
                        font.capitalization: Font.AllUppercase
                        width: 190
                        height: 50
                        font.pointSize: 22
                        background: Rectangle {
                            radius: 10
                            color: "#c2d9e9"
                            border.color: "#c2d9e9"
                        }
                    }

                    Button {
                        id: lp_guest_button
                        text: "Invitado"
                        font.weight: Font.Light
                        font.capitalization: Font.AllUppercase
                        width: 190
                        height: 50
                        font.pointSize: 22
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

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:4}D{i:5}D{i:8}D{i:11}D{i:13}D{i:3}D{i:2}
D{i:1}
}
##^##*/

