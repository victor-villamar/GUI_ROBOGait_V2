import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Views"
import "qrc:/Common"

Dialog {
    id: root
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: "#A0505050"
    }

    width: parent ? Math.min(700, parent.width * 0.90) : 700
    height: keyboardHelper.computedHeight

    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? keyboardHelper.computedY : 0

    KeyboardAwareHelper {
        id: keyboardHelper
        target: root
        maxDialogHeight: parent ? Math.min(760, parent.height * 0.90) : 760
    }

    onOpened: forceActiveFocus()

    background: Rectangle {
        color: "#a9cfe8"
        border.color: "#ffffff"
        border.width: 2
        radius: 12
    }

    contentItem: Item {
        anchors.fill: parent

        Column {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 6

            Item {
                id: header
                width: parent.width
                height: closeButton.height

                Text {
                    text: qsTr("Cambiar usuario")
                    color: "#045671"
                    font.pixelSize: 18
                    font.bold: true
                    anchors.left: parent.left
                    anchors.right: closeButton.left
                    anchors.rightMargin: 10
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 2
                    elide: Text.ElideRight
                }

                Button {
                    id: closeButton
                    width: 40
                    height: 34
                    text: "✕"
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    background: Rectangle {
                        color: closeButton.down ? "#518bb7" : "#ffffff"
                        radius: 8
                        border.color: "#045671"
                        border.width: 2
                    }
                    contentItem: Label {
                        text: closeButton.text
                        color: "#045671"
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: root.close()
                }
            }

            Rectangle {
                width: parent.width
                height: 2
                radius: 1
                color: "#045671"
                opacity: 0.35
            }

            Item {
                width: parent.width
                height: parent.height - 58

                UserSwitchPage {
                    anchors.fill: parent
                    onUserSwitched: root.close()
                }
            }
        }
    }
}
