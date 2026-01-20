import QtQuick 2.15
import QtQuick.Controls 2.15

Popup {
    id: root
    width: parent.width * 0.8
    height: 100
    modal: true
    focus: true
    anchors.centerIn: parent
    background: null

    // Permite controlar el comportamiento del cierre
    property string mode: "instant" // "instant" o "delayed"
    property alias errorRectangleTextError: textError

    // Temporizador para habilitar el cierre
    property bool allowClose: true
    closePolicy: mode === "instant" ? Popup.CloseOnPressOutside : Popup.NoAutoClose
    Timer {
        id: delayTimer
        interval: 5000 // 5 segundos
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
        color: "#f8d7da"
        border.color: "#f5c6cb"
        radius: 10

        Text {
            id: textError
            anchors.centerIn: parent
            text: "Error: Has puesto el robot en una posicion donde esta prohibido."
            color: "#721c24"
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
