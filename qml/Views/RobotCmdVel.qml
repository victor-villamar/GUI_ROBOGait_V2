import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: "#518bb7"

        Rectangle {
            width: parent.width * 0.8
            height: 120
            radius: 10
            color: "#a9cfe8"
            border.color: "#ffffff"
            border.width: 2
            anchors.centerIn: parent

            Text {
                anchors.centerIn: parent
                text: rosManager.robotManager.cmdVelText
                color: "#045671"
                font.pixelSize: 16
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                width: parent.width - 20
            }
        }
    }
}
