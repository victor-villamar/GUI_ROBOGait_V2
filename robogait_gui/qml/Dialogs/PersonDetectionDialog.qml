import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

Popup {
    id: root
    modal: false
    focus: false
    closePolicy: Popup.CloseOnEscape

    parent: Overlay.overlay

    width: parent ? Math.min(520, parent.width * 0.72) : 520
    height: parent ? Math.min(500, parent.height * 0.72) : 500
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round(((parent.height - height) / 2) + (parent.height * 0.04)) : 0

    property string message: ""
    property string imageSource: ""

    signal redetectRequested()

    function reposition()
    {
        if (!parent) {
            return
        }

        x = Math.round((parent.width - width) / 2)
        y = Math.round(((parent.height - height) / 2) + (parent.height * 0.04))
    }

    function openWithDetection(messageText, imageSourceText)
    {
        message = messageText
        imageSource = imageSourceText
        reposition()
        open()
    }

    onOpened: reposition()
    onWidthChanged: reposition()
    onHeightChanged: reposition()
    onParentChanged: reposition()

    background: Rectangle {
        color: AppTheme.dialogsCommon.panelBackground
        border.color: AppTheme.dialogsCommon.light
        border.width: 2
        radius: 8
    }

    contentItem: Item {
        anchors.fill: parent

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                width: parent.width
                text: root.message
                color: AppTheme.dialogsCommon.titleText
                font.pixelSize: 16
                font.bold: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Rectangle {
                width: parent.width
                height: parent.height - 92
                color: AppTheme.dialogsCommon.black
                radius: 6
                clip: true

                Image {
                    anchors.fill: parent
                    anchors.margins: 8
                    source: root.imageSource
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    cache: false
                }
            }

            Button {
                id: redetectButton
                width: parent.width
                height: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 44
                text: qsTr("Volver a detectar")

                background: Rectangle {
                    color: redetectButton.down ? AppTheme.dialogsCommon.closeButtonPressed : AppTheme.dialogsCommon.light
                    radius: 8
                    border.color: AppTheme.dialogsCommon.primary
                    border.width: 2
                }

                contentItem: Label {
                    text: redetectButton.text
                    color: AppTheme.dialogsCommon.primary
                    font.pixelSize: 16
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    root.close()
                    root.redetectRequested()
                }
            }
        }
    }
}
