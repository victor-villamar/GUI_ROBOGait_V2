import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Views"
import "qrc:/Common"
import "qrc:/Dialogs"

Rectangle {
    id: root
    color: "#518bb7"

    property alias home: home
    property alias robot_connection: robot_connection
    property alias lostConnectionPopupItem: lostConnectionPopup

    property string previousState: ""
    property alias mystackview: mystackview
    property alias toolbar: toolbar

    CustomToolBar {
        id: toolbar
        anchors.topMargin: parent.height / 120
        width: parent.width
        height: 20
        anchors.top: parent.top
    }

    StackView {
        id: mystackview
        anchors.top: toolbar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 20

        initialItem: Home {
            id: home
        }
        pushEnter: Transition {
            PropertyAnimation {
                property: "x"
                from: mystackview.width
                to: 0
                duration: 400
            }
        }
        pushExit: Transition {
            PropertyAnimation {
                property: "x"
                from: 0
                to: -mystackview.width
                duration: 400
            }
        }
        popEnter: Transition {
            PropertyAnimation {
                property: "x"
                from: -mystackview.width
                to: 0
                duration: 400
            }
        }
        popExit: Transition {
            PropertyAnimation {
                property: "x"
                from: 0
                to: mystackview.width
                duration: 400
            }
        }
    }

    Component {
        id: robot_connection
        RobotConnection {
            visible: true
        }
    }

    ErrorRectangle {
        id: lostConnectionPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: ""
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.5;height:700;width:1300}D{i:1}D{i:2}D{i:3}D{i:13}
D{i:15}D{i:17}D{i:19}D{i:21}D{i:23}D{i:25}D{i:27}D{i:29}
}
##^##*/
