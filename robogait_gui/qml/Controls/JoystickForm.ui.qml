import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

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
        color: AppTheme.joystick.baseFill
        border.color: AppTheme.joystick.light
        border.width: 4
        anchors.centerIn: parent

        Rectangle {
            id: stick
            width: totalArea.width / 4
            height: width
            radius: width / 2
            color: AppTheme.joystick.stickFill
            border.color: AppTheme.joystick.light
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
