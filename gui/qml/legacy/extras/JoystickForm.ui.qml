import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    height: 500
    width: 500
    id: joyStick
    property alias mouseAreaJoystick: mouseAreaJoystick
    property alias totalArea: totalArea
    property alias stick: stick

    Rectangle {
        id: totalArea
        color: "#a6c6de"
        radius: parent.width / 2
        opacity: 0.5
        width: parent.width
        height: parent.height
    }

    Rectangle {
        id: stick
        width: totalArea.width / 3.5
        height: width
        radius: width / 2
        x: totalArea.width / 2 - radius
        y: totalArea.height / 2 - radius
        color: "white"
    }

    MouseArea {
        id: mouseAreaJoystick
        anchors.fill: parent
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.66;height:480;width:640}D{i:1}D{i:2}D{i:3}
}
##^##*/

