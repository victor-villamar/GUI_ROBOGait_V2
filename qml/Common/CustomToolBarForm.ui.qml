import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0

Item {
    id: root
    property alias backButton: backButton
    property alias logo: logo
    property alias title: title
    property alias robotBadge: robotBadge
    property alias userBadge: userBadge
    property bool showRobotBadge: false
    property bool showUserBadge: false

    Rectangle {
        anchors.fill: parent
        color: "transparent"

        Button {
            id: backButton
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            icon.source: "qrc:/qmlresources/icons/keyboard_left.svg"
            icon.color: "white"
            icon.width: 24
            icon.height: 24
            background: null
            transitions: Transition {
                NumberAnimation {
                    properties: "scale"
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }
        }

        // Logo and Title
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenterOffset: -5
            spacing: 10

            Image {
                id: logo
                source: "qrc:/qmlresources/logos/robogait_logo.png"
                sourceSize.height: 25
                sourceSize.width: height * 1.4
            }

            Text {
                id: title
                text: "ROBOGAIT"
                font.pixelSize: 20
                color: "white"
                font.bold: true
            }
        }

        Row {
            id: rightBadges
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 10
            spacing: 10

            RobotBadge {
                id: robotBadge
                visible: root.showRobotBadge && robotConnected
                enabled: visible
            }

            UserBadge {
                id: userBadge
                visible: root.showUserBadge && loggedIn
                enabled: visible
            }
        }
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:1.1;height:480;width:640}D{i:2}D{i:5}D{i:8}D{i:1}
}
##^##*/
