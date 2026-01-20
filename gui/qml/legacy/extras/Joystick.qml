import QtQuick 2.15
import QtQuick.Controls 2.15

JoystickForm {
    id:joyStick
    property int offset:30;

    signal dirChanged(double linear, double angular)
    signal pressed()
    signal released()

    function isInsideCircle(x, y) {
        var centerX = totalArea.width / 2
        var centerY = totalArea.height / 2
        var dx = x - centerX
        var dy = y - centerY
        var distance = Math.sqrt(dx * dx + dy * dy)
        var maxDistance = 2 * totalArea.radius
        return distance <= maxDistance
    }

    mouseAreaJoystick.onPressed: {
        if (isInsideCircle(mouseAreaJoystick.mouseX, mouseAreaJoystick.mouseY)) {
            joyStick.pressed()
        }
    }

    mouseAreaJoystick.onPositionChanged: {
        if (!isInsideCircle(mouseAreaJoystick.mouseX, mouseAreaJoystick.mouseY))
            return

        var dx = mouseAreaJoystick.mouseX - totalArea.width / 2
        var dy = mouseAreaJoystick.mouseY - totalArea.height / 2
        var distance = Math.sqrt(dx * dx + dy * dy)
        var maxDistance = totalArea.radius - stick.width / 2

        var ratio = Math.min(1.0, maxDistance / distance)
        stick.x = totalArea.width / 2 + dx * ratio - stick.width / 2
        stick.y = totalArea.height / 2 + dy * ratio - stick.height / 2

        var linearVelocity = dy / totalArea.height * -1.0
        var angularVelocity = dx / totalArea.width * -1.0

        joyStick.dirChanged(linearVelocity, angularVelocity)
    }



    mouseAreaJoystick.onReleased: {
        stick.x = totalArea.width / 2 - stick.radius
        stick.y = totalArea.height / 2 - stick.radius
        joyStick.released()

    }
}
