import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

Popup {
    id: root
    width: parent ? Math.min(560, parent.width * 0.80) : 560
    height: 100
    modal: true
    focus: true
    parent: Overlay.overlay
    anchors.centerIn: parent
    background: null

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: AppTheme.dialogsCommon.overlayDim
    }

    // Permite controlar el comportamiento del cierre
    property string mode: "instant" // "instant" o "delayed"
    property alias errorRectangleTextError: textError

    // Temporizador para habilitar el cierre
    property bool allowClose: true
    closePolicy: mode === "instant" ? Popup.CloseOnPressOutside : Popup.NoAutoClose
    Timer {
        id: delayTimer
        interval: timeoutSettings ? timeoutSettings.errorRectangleDelayedCloseMs : 5000
        running: false
        repeat: false
        onTriggered: {
            allowClose = true
        }
    }

    // Cuando se abre el popup
    onOpened: {
        if (mode === "delayed") {
            console.log("IAM INSIDE DELAY")
            allowClose = false
            delayTimer.start()
        } else {
            allowClose = true
        }
    }

    Rectangle {
        width: parent.width
        height: parent.height
        color: AppTheme.dialogsCommon.errorBackground
        border.color: AppTheme.dialogsCommon.errorBorder
        radius: 10

        Text {
            id: textError
            anchors.centerIn: parent
            text: "Error: Has puesto el robot en una posicion donde esta prohibido."
            color: AppTheme.dialogsCommon.errorText
            font.pixelSize: 16
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (allowClose)
                    root.close()
            }
        }
    }
}
