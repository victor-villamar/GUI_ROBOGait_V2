import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    visible: true

    // property alias orientation_rectangle: orientation_rectangle
    property alias robotImage: robotImage
    width: 250
    height: 250
    property alias mouseAreaCircle: mouseAreaCircle
    property alias orientationCircle_rectangle: orientationCircle_rectangle
    property string orientation: orientation

    Rectangle {
        id: orientationCircle_rectangle
        anchors.centerIn: parent
        width: 250
        height: 250
        color: "lightgray"
        radius: width / 2

        // Circle boundary for reference
        Rectangle {
            id: orientation_rectangle
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            radius: width / 2
            color: "#a6c6de"
            border.color: "#a6c6de"
            border.width: 2
        }

        // Robot Image
        Image {
            rotation: 90
            id: robotImage
            source: "../../images/robot/robot_with_arrow.png"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            scale: 0.4

            // Animación al rotar
            RotationAnimator on rotation {
                duration: 200
            }
        }

        MouseArea {
            id: mouseAreaCircle
            anchors.fill: parent

            onPositionChanged: {
                let dx = mouseX - width / 2;
                let dy = mouseY - height / 2;
                let angleDegrees = Math.atan2(dy, dx) * (180 / Math.PI);
                if (angleDegrees < 0) {
                    angleDegrees += 360;
                }
                robotImage.rotation = angleDegrees;
                angleDegrees = 360 - angleDegrees;
                orientation = angleDegrees;
                angleDisplay.text = "Ángulo: " + angleDegrees.toFixed(2) + "º";
            }

            onClicked: {
                let dx = mouseX - width / 2;
                let dy = mouseY - height / 2;
                let angleDegrees = Math.atan2(dy, dx) * (180 / Math.PI);
                if (angleDegrees < 0) {
                    angleDegrees += 360;
                }
                robotImage.rotation = angleDegrees;
                // console.log("Orientacion en ORIENTATIONCIRCLEFORM.QML" + angleDegrees)
                angleDegrees = 360 - angleDegrees;
                orientation = angleDegrees;
                angleDisplay.text = "Ángulo: " + angleDegrees.toFixed(2) + "º";
            }
        }



        // Angle Display
        Text {
            id: angleDisplay
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            font.pixelSize: 16
            color: "#104452"
            text: "Angle: 0 rad"
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.5;height:480;width:640}D{i:2}D{i:3}D{i:6}D{i:7}
D{i:1}
}
##^##*/

