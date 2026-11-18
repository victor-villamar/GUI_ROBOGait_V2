import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import "qrc:/Application"

ApplicationWindow {
    id: window
    height: 1080
    width: 1920
    visible: true
    visibility: Window.FullScreen

    title:   qsTr("Robogait")
    property bool keyboardVisible: inputPanelLoader.item ? inputPanelLoader.item.keyboardVisible : false
    property real keyboardHeight: inputPanelLoader.item ? inputPanelLoader.item.reservedHeight : 0

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: "#A0505050"
    }

    ApplicationFlow {
        width: parent.width
        height: parent.height
        keyboardVisible: window.keyboardVisible
        keyboardHeight: window.keyboardHeight
    }

    Loader {
        id: inputPanelLoader
        source: "qrc:/Application/VirtualKeyboardPanel.qml"
        asynchronous: false
        active: false
        
        Component.onCompleted: {
            active = true
        }
        
        onStatusChanged: {
            if (status === Loader.Error) {
                console.log("[main.qml] Qt VirtualKeyboard not available - keyboard support disabled")
                active = false
            }
        }
    }
}
