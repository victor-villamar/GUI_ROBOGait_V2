import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Common"

Page {
    id: root

    property alias usernameField: usernameField
    property alias passwordField: passwordField
    property alias loginButton: loginButton
    property alias authModeDropdown: authModeDropdown

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
                    spacing: 20
                    anchors.top: parent.top
                    anchors.topMargin: content.topPadding
                    anchors.horizontalCenter: parent.horizontalCenter

                    Item {
                        width: formColumn.width
                        height: logo.height

                        Image {
                            id: logo
                            anchors.centerIn: parent
                            width: Math.min(220, formColumn.width)
                            height: width
                            fillMode: Image.PreserveAspectFit
                            source: "qrc:/qmlresources/logos/robogait_logo.png"
                        }
                    }

                    AuthModeDropdown {
                        id: authModeDropdown
                        width: formColumn.width
                        currentMode: "login"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    TextField {
                        id: usernameField
                        width: formColumn.width
                        height: 50
                        font.pointSize: 20
                        color: "#000000"
                        placeholderTextColor: "#808080"
                        placeholderText: qsTr("Usuario")
                        background: Rectangle {
                            radius: 10
                            color: "#ffffff"
                            border.color: "#CCCCCC"
                        }
                    }

                    TextField {
                        id: passwordField
                        width: formColumn.width
                        height: 50
                        font.pointSize: 20
                        color: "#000000"
                        placeholderTextColor: "#808080"
                        echoMode: TextInput.Password
                        placeholderText: qsTr("Contraseña")
                        background: Rectangle {
                            radius: 10
                            color: "#ffffff"
                            border.color: "#CCCCCC"
                        }
                    }

                    Button {
                        id: loginButton
                        width: formColumn.width
                        height: 50
                        text: qsTr("Iniciar sesión")
                        font.capitalization: Font.AllUppercase
                        font.bold: true
                        font.pointSize: 22
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
