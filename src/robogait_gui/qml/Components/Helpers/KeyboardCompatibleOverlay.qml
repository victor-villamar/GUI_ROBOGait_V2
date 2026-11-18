import QtQuick 2.15

Rectangle {
    id: root

    signal backgroundPressed()

    MouseArea {
        anchors.fill: parent
        onClicked: root.backgroundPressed()
    }
}
