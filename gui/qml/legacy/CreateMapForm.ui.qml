import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "extras"

Rectangle {
    id: rectangle
    color: "#518bb7"
    property alias resetMapButton: resetMapButton
    property alias saveMapButton: saveMapButton
    property alias imageDisplay: imageDisplay
    property alias joystick: joystick
    property alias mapPageForm_buttonStop: buttonStop

    property real linear_value
    property real angular_value

    property alias save_page: save_page

    Text {
        id: mapPageForm_text
        height: parent.width / 8
        color: "#ffffff"
        text: qsTr("Desplace el círculo pequeño para comenzar a crear el mapa. La velocidad de movimiento aumentará cuanto más se aleje del centro. Para más detalles presione el botón de información.")
        anchors.left: mapa.right
        anchors.right: parent.right
        anchors.top: parent.top
        font.pixelSize: 18
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.styleName: "Medium"
        font.bold: true
        wrapMode: Text.Wrap
        anchors.topMargin: 0
        anchors.leftMargin: 10
        anchors.rightMargin: 10
    }

    Rectangle {
        id: mapa
        width: 2 * parent.width / 3
        height: parent.height
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        color: "#518bb7"

        Image {
            id: imageDisplay
            width: Math.min(parent.width,
                            parent.height * imageDisplay.sourceSize.width
                            / imageDisplay.sourceSize.height)
            height: Math.min(parent.height,
                             parent.width * imageDisplay.sourceSize.height
                             / imageDisplay.sourceSize.width)
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectCrop
        }
    }

    Item {
        id: manualControl
        width: Math.min(275, Math.max(175, parent.width / 4))
        height: width
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenterOffset: mapa.width / 2
        anchors.horizontalCenter: parent.horizontalCenter

        Joystick {
            id: joystick
            width: parent.width
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
    Button {
        id: resetMapButton
        width: 160
        height: 35
        text: qsTr("RESETEAR")
        anchors.left: manualControl.horizontalCenter
        anchors.top: manualControl.bottom
        anchors.leftMargin: 7
        anchors.topMargin: 30
        font.styleName: "Medium"
        font.capitalization: Font.AllUppercase
        font.weight: Font.Normal
        font.bold: true
        font.pointSize: 15
        background: Rectangle {
            radius: 10
            color: "#aed2ea"
            border.color: "#aed2ea"
        }
    }

    Button {
        id: buttonStop
        height: 100
        anchors.right: mapPageForm_text.left
        width: height
        anchors.top: parent.top
        anchors.rightMargin: 0
        anchors.topMargin: 5
        property string defaultImage: "../images/stop.png"
        property string pressedImage: "../images/stop_light.png"

        background: Image {
            source: buttonStop.pressed ? buttonStop.pressedImage : buttonStop.defaultImage
            fillMode: Image.PreserveAspectFit
        }
    }

    SavePage {
        id: save_page
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -60
        anchors.horizontalCenter: parent.horizontalCenter
    }
    Button {
        id: saveMapButton
        x: 885
        width: 160
        height: 35
        text: qsTr("GUARDAR")
        anchors.right: resetMapButton.left
        anchors.top: manualControl.bottom
        anchors.rightMargin: 15
        anchors.topMargin: 30
        font.styleName: "Medium"
        font.capitalization: Font.AllUppercase
        font.weight: Font.Normal
        font.bold: true
        font.pointSize: 15
        background: Rectangle {
            radius: 10
            color: "#aed2ea"
            border.color: "#aed2ea"
        }
    }
    Text {
        id: text1
        x: 1018
        y: 174
        width: 182
        height: 32
        color: "#ffffff"

        //        text: qsTr("%1 m/s  %2 rad/s").arg(linear_value.toFixed(3)).arg(
        //                  angular_value.toFixed(3))
        anchors.right: manualControl.right
        anchors.bottom: manualControl.top
        font.pixelSize: 20
        anchors.rightMargin: 0
        anchors.bottomMargin: 0
    }
    Button {
        id: infoButton
        width: 37
        height: 37
        anchors.left: mapa.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 0
        anchors.bottomMargin: 20
        anchors.rightMargin: -7
        background: Image {
            source: "../images/icon_app/circle-info-solid.svg"
            fillMode: Image.PreserveAspectFit
        }
        Behavior on scale {
            NumberAnimation {
                duration: 150
                easing.type: Easing.OutQuad
            }
        }

        // Change scale when pressed/released
        onPressed: scale = 1.2
        onReleased: scale = 1.0
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:1}D{i:3}D{i:2}D{i:5}D{i:4}D{i:6}D{i:8;invisible:true}
D{i:9}D{i:11}D{i:12}
}
##^##*/
