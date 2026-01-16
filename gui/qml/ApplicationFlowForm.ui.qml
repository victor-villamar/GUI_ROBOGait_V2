import QtQuick 2.15
import QtQuick.Controls 2.15
import "extras"

Rectangle {
    id: root
    color: "#518bb7"
    property alias home: home
    property alias robot_connection: robot_connection
    // property alias register_page: register_page
    // property alias menu_app: menu_app
    // property alias select_patient: select_patient
    // property alias register_patient: register_patient
    // property alias select_map: select_map
    // property alias register_map: register_map
    // property alias create_map: create_map
    // property alias manualControl: manualControl
    // property alias map_path: map_path

    property string previousState: ""
    property alias mystackview: mystackview
    property alias toolbar: toolbar
    property alias bottomBar: bottomBar

    CustomToolBar {
        id: toolbar
        anchors.topMargin: parent.height / 120
        width: parent.width
        height: 20
        anchors.top: parent.top
    }

    CustomBottomBar {
        id: bottomBar
        width: parent.width
        height: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height / 120
    }

    StackView {
        id: mystackview
        anchors.top: toolbar.bottom
        anchors.bottom: bottomBar.top
        anchors.bottomMargin: 0
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.right: parent.right
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
        //! [mystackview view]
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
    // Component {
    //     id: register_page
    //     RegisterPage {
    //         visible: true
    //         // anchors.fill: parent
    //     }
    // }
    // Component {
    //     id: menu_app
    //     MainMenu {
    //         visible: true
    //     }
    // }
    // Component {
    //     id: manualControl
    //     ManualControl {
    //         // visible_value: applicationFlow.visible_image
    //         // visible: true
    //         // visible_save: applicationFlow.visible_save
    //         // type_save: applicationFlow.type_save
    //     }
    // }
    // Component {
    //     id: select_patient
    //     SelectPatient {
    //         visible: true
    //     }
    // }
    // Component {
    //     id: register_patient
    //     RegisterPatient {
    //         visible: true
    //     }
    // }
    // Component {
    //     id: select_map
    //     SelectMap {
    //         visible: true
    //     }
    // }
    // Component {
    //     id: register_map
    //     RegisterMap{
    //         visible: true
    //     }
    // }
    // Component {
    //     id: create_map
    //     CreateMap{
    //         visible: true
    //     }
    // }
    // Component {
    //     id: map_path
    //     MapPage {
    //         visible: true
    //     }
    // }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.5;height:700;width:1300}D{i:1}D{i:2}D{i:3}D{i:13}
D{i:15}D{i:17}D{i:19}D{i:21}D{i:23}D{i:25}D{i:27}D{i:29}
}
##^##*/

