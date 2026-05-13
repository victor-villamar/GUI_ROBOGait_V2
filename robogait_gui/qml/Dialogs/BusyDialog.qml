import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

Dialog {
    id: root
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose

    parent: Overlay.overlay

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: AppTheme.dialogsCommon.overlayDim
    }

    width: parent ? Math.min(720, parent.width * 0.85) : 720
    height: parent ? Math.min(320, parent.height * 0.45) : 320

    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0

    property string message: "Procesando..."
    property int timeoutMs: 0
    property bool autoCloseOnTimeout: true

    signal timedOut()

    function restartTimeout()
    {
        if (timeoutMs > 0)
        {
            timeoutTimer.interval = timeoutMs
            timeoutTimer.restart()
        }
        else
        {
            timeoutTimer.stop()
        }
    }

    function openWithMessage(text)
    {
        message = text || ""
        if (visible)
        {
            restartTimeout()
            return
        }
        open()
    }

    onOpened: restartTimeout()
    onClosed: timeoutTimer.stop()

    Timer {
        id: timeoutTimer
        interval: root.timeoutMs > 0 ? root.timeoutMs : 1
        repeat: false
        running: false
        onTriggered: {
            root.timedOut()
            if (root.autoCloseOnTimeout)
            {
                root.close()
            }
        }
    }

    background: Rectangle {
        color: AppTheme.dialogsCommon.primary
        border.color: AppTheme.dialogsCommon.primary
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
            color: AppTheme.dialogsCommon.panelBackground
            radius: 12
            border.color: AppTheme.dialogsCommon.primary
            border.width: 2

            Column {
                anchors.centerIn: parent
                spacing: 24
                width: parent.width - 64

                Label {
                    text: root.message
                    color: AppTheme.dialogsCommon.primary
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
                    palette.highlight: AppTheme.dialogsCommon.light
                    palette.text: AppTheme.dialogsCommon.light
                    palette.buttonText: AppTheme.dialogsCommon.light
                }
            }
        }
    }
}
