import QtQuick 2.15
import QtQuick.Controls 2.15

Dialog {
    id: root
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose

    width: parent ? Math.min(560, parent.width * 0.80) : 560

    property string robotName: ""
    property string message: ""

    function openForRobot(name) {
        robotName = name
        message = ""
        open()
    }

    background: Rectangle {
        color: "#a9cfe8"
        border.color: "#ffffff"
        border.width: 2
        radius: 12
    }

    contentItem: Item {
        width: root.width
        height: contentColumn.implicitHeight + 36

        Column {
            id: contentColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            anchors.topMargin: 18
            spacing: 12

            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                bottomPadding: 20
                color: "#4f86b4"
                font.pixelSize: 16
                font.bold: true
                text: root.message !== ""
                      ? root.message
                      : (qsTr("Se ha seleccionado ") + root.robotName +
                         qsTr(".\n¿Está seguro que quiere conectarse a este robot?"))
            }
        }
    }

    footer: Row {
        id: footerRow
        spacing: 12
        padding: 16
        width: parent.width

        Button {
            id: cancelButton
            width: (footerRow.width - footerRow.padding * 2 - footerRow.spacing) / 2
            height: 44
            text: qsTr("Cancelar")

            background: Rectangle {
                color: cancelButton.down ? "#518bb7" : "#ffffff"
                radius: 8
                border.color: "#045671"
                border.width: 2
                Behavior on color { ColorAnimation { duration: 100 } }
            }
            contentItem: Label {
                text: cancelButton.text
                color: "#045671"
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.reject()
        }

        Button {
            id: acceptButton
            width: (footerRow.width - footerRow.padding * 2 - footerRow.spacing) / 2
            height: 44
            text: qsTr("Aceptar")

            background: Rectangle {
                color: acceptButton.down ? "#00C8FF" : "#ffffff"
                radius: 8
                border.color: "#045671"
                border.width: 2
                Behavior on color { ColorAnimation { duration: 100 } }
            }
            contentItem: Label {
                text: acceptButton.text
                color: "#045671"
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.accept()
        }
    }
}
