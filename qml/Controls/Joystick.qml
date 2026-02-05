import QtQuick 2.15
import QtQuick.Controls 2.15

JoystickForm {
    id: joyStick
    property int offset: 30

    signal dirChanged(double linear, double angular)
    signal pressed()
    signal released()

    function isInsideCircle(x, y) {
        var centerX = totalArea.width / 2
        var centerY = totalArea.height / 2
        var dx = x - centerX
        var dy = y - centerY
        var distance = Math.sqrt(dx * dx + dy * dy)
        var maxDistance = (totalArea.width / 2)
        return distance <= maxDistance
    }

    mouseAreaJoystick.onPressed: {
        if (isInsideCircle(mouseAreaJoystick.mouseX, mouseAreaJoystick.mouseY)) {
            joyStick.pressed()
        }
    }

    mouseAreaJoystick.onPositionChanged: {
        if (!mouseAreaJoystick.pressed){
            return
        }
            
        if (!isInsideCircle(mouseAreaJoystick.mouseX, mouseAreaJoystick.mouseY))
        {
            return
        }

        var dx = mouseAreaJoystick.mouseX - totalArea.width / 2
        var dy = mouseAreaJoystick.mouseY - totalArea.height / 2
        var distance = Math.sqrt(dx * dx + dy * dy)
        var maxDistance = (totalArea.width / 2) - (stick.width / 2)

        if (distance > maxDistance) {
            var ratio = maxDistance / distance
            dx *= ratio
            dy *= ratio
        }

        stick.x = totalArea.width / 2 + dx - stick.width / 2
        stick.y = totalArea.height / 2 + dy - stick.height / 2

        var linearVelocity = -(dy / (totalArea.width / 2))
        var angularVelocity = -(dx / (totalArea.width / 2))

        linearVelocity = Math.max(-1.0, Math.min(1.0, linearVelocity))
        angularVelocity = Math.max(-1.0, Math.min(1.0, angularVelocity))

        joyStick.dirChanged(linearVelocity, angularVelocity)
    }

    mouseAreaJoystick.onReleased: {
        stick.x = totalArea.width / 2 - stick.width / 2
        stick.y = totalArea.height / 2 - stick.height / 2
        joyStick.released()
    }
}