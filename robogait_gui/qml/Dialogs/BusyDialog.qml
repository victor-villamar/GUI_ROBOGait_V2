import QtQuick 2.15
import QtQuick.Controls 2.15

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

    width: parent ? Math.min(720, parent.width * 0.85) : 720
    height: parent ? Math.min(320, parent.height * 0.45) : 320

    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0

    property string message: "Procesando..."

    function openWithMessage(text) {
        message = text || ""
        open()
    }

    background: Rectangle {
        color: "#045671"
        border.color: "#045671"
        border.width: 2
        radius: 12
    }

    contentItem: Item {
        anchors.fill: parent

        Rectangle {
            id: card
            width: parent.width - 32
            height: parent.height - 32
            anchors.centerIn: parent
            color: "#a9cfe8"
            radius: 12
            border.color: "#045671"
            border.width: 2

            Column {
                anchors.centerIn: parent
                spacing: 24
                width: parent.width - 64

                Label {
                    text: root.message
                    color: "#045671"
                    font.pixelSize: 18
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                    width: parent.width
                }

                BusyIndicator {
                    width: 180
                    height: 180
                    running: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }
}
