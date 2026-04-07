import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

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

    width: parent ? Math.min(560, parent.width * 0.80) : 560
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    property string message: ""
    property string acceptText: qsTr("Aceptar")
    property bool holdToAccept: true
    property int acceptHoldMs: 900
    readonly property real buttonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 44

    function reposition()
    {
        if (!parent) {
            return
        }

        x = Math.round((parent.width - width) / 2)
        y = Math.round((parent.height - height) / 2)
    }

    function openWithMessage(messageText) 
    {
        message = messageText
        reposition()
        open()
    }

    onOpened: reposition()
    onWidthChanged: reposition()
    onHeightChanged: reposition()
    onParentChanged: reposition()

    background: Rectangle {
        color: "#a9cfe8"
        border.color: "#ffffff"
        border.width: 2
        radius: 12
    }

    contentItem: Item {
        implicitWidth: root.width
        implicitHeight: messageText.implicitHeight + 24

        Text {
            id: messageText
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            color: "#4f86b4"
            font.pixelSize: 16
            font.bold: true
            text: root.message
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
            height: root.buttonHeightPx
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

            onClicked: {
                holdAnimation.stop()
                acceptButton.holdProgress = 0
                root.reject()
            }
        }

        Button {
            id: acceptButton
            width: (footerRow.width - footerRow.padding * 2 - footerRow.spacing) / 2
            height: root.buttonHeightPx
            text: root.acceptText

            property real holdProgress: 0

            NumberAnimation {
                id: holdAnimation
                target: acceptButton
                property: "holdProgress"
                from: 0
                to: 1
                duration: root.acceptHoldMs
                onFinished: root.accept()
            }

            background: Rectangle {
                id: acceptButtonBackground
                color: "#ffffff"
                radius: 8
                border.color: "#045671"
                border.width: 2
                clip: true

                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    color: "#00C8FF"
                    width: parent.width * acceptButton.holdProgress
                    radius: acceptButtonBackground.radius
                    antialiasing: true
                }
            }

            contentItem: Label {
                text: acceptButton.text
                color: "#045671"
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onPressed: {
                if(!root.holdToAccept)
                {
                    return
                }

                holdAnimation.stop()
                acceptButton.holdProgress = 0
                holdAnimation.start()
            }

            onReleased: {
                if(!root.holdToAccept)
                {
                    return
                }

                holdAnimation.stop()
                acceptButton.holdProgress = 0
            }

            onCanceled: {
                if(!root.holdToAccept)
                {
                    return
                }

                holdAnimation.stop()
                acceptButton.holdProgress = 0
            }

            onClicked: {
                if(!root.holdToAccept)
                {
                    root.accept()
                }
            }
        }
    }
}
