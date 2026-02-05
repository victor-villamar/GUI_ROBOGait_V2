import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Common"

Page {
    id: root

    property alias nameField: nameField
    property alias lastNameField: lastNameField

    property alias guestButton: guestButton
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
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: root.contentCenterOffset

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
                        currentMode: "guest"
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

                    Button {
                        id: guestButton
                        width: formColumn.width
                        height: 50
                        text: qsTr("Entrar como invitado")
                        font.capitalization: Font.AllUppercase
                        font.bold: true
                        font.pointSize: 18
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
