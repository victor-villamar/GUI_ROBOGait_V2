import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    height: 700
    width: 1300
    visible: true
    visibility: Window.Windowed

    title:   qsTr("Robogait")

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: "#A0505050"
    }

    ApplicationFlow {
        width: parent.width
        height: parent.height
    }
}
