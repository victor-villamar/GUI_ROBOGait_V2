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
    property alias register_page: register_page
    property alias lostConnectionPopupItem: lostConnectionPopup

    property string previousState: ""
    property alias mystackview: mystackview
    property alias toolbar: toolbar
    property alias userSwitchDialog: userSwitchDialog

    CustomToolBar {
        id: toolbar
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: 0
        height: 70
        z: 10
    }

    StackView {
        id: mystackview
        z: 0
        anchors.top: toolbar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 0

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

    Component {
        id: register_page
        RegisterPage {
            visible: true
        }
    }

    ErrorRectangle {
        id: lostConnectionPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: ""
    }

    UserSwitchDialog {
        id: userSwitchDialog
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.5;height:700;width:1300}D{i:1}D{i:2}D{i:3}D{i:13}
D{i:15}D{i:17}D{i:19}D{i:21}D{i:23}D{i:25}D{i:27}D{i:29}
}
##^##*/
