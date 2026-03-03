import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 275
    height: 275
    
    property alias mouseAreaJoystick: mouseAreaJoystick
    property alias totalArea: totalArea
    property alias stick: stick

    Rectangle {
        id: totalArea
        width: Math.min(parent.width, parent.height)
        height: width
        radius: width / 2
        color: "#a9cfe8"
        border.color: "#ffffff"
        border.width: 4
        anchors.centerIn: parent

        Rectangle {
            id: stick
            width: totalArea.width / 4
            height: width
            radius: width / 2
            color: "#045671"
            border.color: "#ffffff"
            border.width: 3
            x: (totalArea.width - width) / 2
            y: (totalArea.height - height) / 2

            property real stickRadius: width / 2
        }

        MouseArea {
            id: mouseAreaJoystick
            anchors.fill: parent
        }
    }
}