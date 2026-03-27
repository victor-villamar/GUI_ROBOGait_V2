import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Common"

Page {
    id: root

    property alias nameField: nameField
    property alias lastNameField: lastNameField
    property alias userNameField: userNameField
    property alias roleCombo: roleCombo
    property alias passwordField: passwordField
    property alias repeatPasswordField: repeatPasswordField

    property alias signInButton: signInButton
    property alias authModeDropdown: authModeDropdown
    property real keyboardSafeArea: 0
    property int contentCenterOffset: 0
    property real formContentHeight: formColumn.implicitHeight

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
                height: Math.max(formColumn.implicitHeight, scroll.height)

                Column {
                    id: formColumn
                    width: Math.min(400, Math.max(0, content.width - 40))
                    spacing: 16
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: root.contentCenterOffset

                    AuthModeDropdown {
                        id: authModeDropdown
                        width: formColumn.width
                        currentMode: "sign_in"
                        anchors.horizontalCenter: parent.horizontalCenter
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
                        currentIndex: 0

                        background: Rectangle {
                            color: "#333333"
                            border.color: "#ffffff"
                            border.width: 1
                        }

                        Component.onCompleted: {
                            if(currentIndex < 0 && count  > 0)
                            {
                                currentIndex = 0
                            }
                        }

                        onCountChanged: {
                            if (currentIndex < 0 && count > 0)
                            {
                                currentIndex = 0
                            }
                        }

                        contentItem: Text {
                            text: roleCombo.displayText.length  > 0
                                  ? roleCombo.currentText
                                  : (roleCombo.count > 0 ? roleCombo.textAt(0) : "")
                            color: "#ffffff"
                            font.pointSize: 20
                            leftPadding: 12
                            rightPadding: 32
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }

                        indicator: Text {
                            text: "▼"
                            color: "#ffffff"
                            font.pixelSize: 10
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        popup: Popup {
                            y: roleCombo.height - 1
                            width: roleCombo.width
                            padding: 0

                            contentItem: ListView {
                                implicitHeight: contentHeight
                                model: roleCombo.popup.visible ? roleCombo.delegateModel : null

                                delegate: ItemDelegate {
                                    width: roleCombo.width
                                    height: 44

                                    contentItem: Text {
                                        text: modelData
                                        color: "#ffffff"
                                        font.pointSize: 16
                                        leftPadding: 12
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    background: Rectangle {
                                        color: highlighted ? "#4a4a4a" : "#333333"
                                        border.color: "#ffffff"
                                        border.width: 1
                                    }
                                }
                            }

                            background: Rectangle {
                                color: "#333333"
                                border.color: "#ffffff"
                                border.width: 1
                            }
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

                        contentItem: Text {
                            text: signInButton.text
                            font: signInButton.font
                            color: "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

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
