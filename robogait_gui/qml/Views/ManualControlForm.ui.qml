import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "qrc:/Controls"
import "qrc:/Dialogs"

Rectangle {
    id: rectangle
    color: "#518bb7"
    
    property alias joystick: joystick
    property alias infoButton: infoButton
    property alias infoDialog: infoDialog
    property alias lockButton: lockButton
    property bool manualUnlocked: false
    property real linearValue: 0.0
    property real angularValue: 0.0
    property real iconButtonSizePx: 0
    property real iconGlyphSizePx: 0
    property real joystickAreaSizePx: 0

    Rectangle {
        id: information
        width: 2 * parent.width / 3 - 40
        color: "#518bb7"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 30
        anchors.bottomMargin: 30
        anchors.topMargin: 30

        Image {
            id: svgImage
            source: "qrc:/qmlresources/logos/robogait_logo.png"
            anchors.topMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            width: 240
            height: 240
            anchors.top: parent.top
            fillMode: Image.PreserveAspectFit
        }

        Column {
            width: information.width - 40
            anchors.top: svgImage.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 0
            spacing: 20

            Text {
                text: qsTr("Desbloqueo del joystick")
                color: "#00C8FF"
                font.pixelSize: 26
                styleColor: "#e9e9e9"
                font.bold: true
            }

            Text {
                id: step1Text
                width: parent.width
                color: "#ffffff"
                font.pixelSize: 21
                wrapMode: Text.WordWrap
                text: qsTr("Pulse el candado a la derecha del joystick para desbloquearlo. Para bloquearlo, vuelva a pulsar el candado.")
            }

            Text {
                text: qsTr("Movimiento del joystick")
                color: "#00C8FF"
                font.pixelSize: 26
                styleColor: "#e9e9e9"
                font.bold: true
            }

            Text {
                id: step2Text
                width: parent.width
                color: "#ffffff"
                font.pixelSize: 21
                wrapMode: Text.WordWrap
                text: qsTr("Desplace el círculo pequeño para comenzar a moverse. La velocidad de movimiento aumentará cuanto más se aleje del centro.")
            }

            Text {
                id: moreInfoText
                width: parent.width
                color: "#ffffff"
                font.pixelSize: 21
                wrapMode: Text.WordWrap
                text: qsTr("Para más detalles presione el botón de información.")
            }

            Text {
                text: qsTr("¡¡ADVERTENCIA!!")
                color: "#c52020"
                font.pixelSize: 26
                font.bold: true
            }

            Text {
                id: warningText
                width: parent.width
                color: "#ffffff"
                font.pixelSize: 21
                wrapMode: Text.WordWrap
                text: qsTr("Este robot no cuenta con un sistema de gestión de colisiones. Asegúrese de evitar obstáculos y supervisar su desplazamiento en todo momento.")
            }
        }
    }

    Item {
        id: manualControl
        width: joystickAreaSizePx > 0 ? joystickAreaSizePx : Math.min(275, Math.max(175, parent.width / 4))
        height: width
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenterOffset: information.width / 2
        anchors.horizontalCenter: parent.horizontalCenter

        Joystick {
            id: joystick
            width: parent.width
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            mouseAreaJoystick.enabled: manualUnlocked
        }
    }

    Button {
        id: infoButton
        width: rectangle.iconButtonSizePx
        height: rectangle.iconButtonSizePx
        anchors.left: information.right
        anchors.bottom: information.bottom
        anchors.leftMargin: 10
        anchors.bottomMargin: 0

        background: Rectangle {
            radius: width / 2
            color: "transparent"
        }

        contentItem: Image {
            source: "qrc:/qmlresources/icons/white/circle_info_solid.svg"
            width: rectangle.iconGlyphSizePx
            height: rectangle.iconGlyphSizePx
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Behavior on scale {
            NumberAnimation {
                duration: 150
                easing.type: Easing.OutQuad
            }
        }

        onPressed: scale = 1.2
        onReleased: scale = 1.0
    }

    Text {
        id: velocityText
        height: 32
        color: "#ffffff"
        text: qsTr("%1 m/s  %2 rad/s").arg(linearValue.toFixed(3)).arg(angularValue.toFixed(3))
        anchors.horizontalCenter: manualControl.horizontalCenter
        anchors.bottom: manualControl.top
        font.pixelSize: 20
        anchors.bottomMargin: 10
    }

    Button {
        id: lockButton
        width: rectangle.iconButtonSizePx
        height: rectangle.iconButtonSizePx
        anchors.top: manualControl.top
        anchors.left: manualControl.right
        anchors.leftMargin: 12

        background: Rectangle {
            radius: width / 2
            color: "transparent"
        }

        contentItem: Item {
            anchors.fill: parent

            Image {
                id: lockIcon
                anchors.centerIn: parent
                width: rectangle.iconGlyphSizePx
                height: rectangle.iconGlyphSizePx
                source: manualUnlocked ? "qrc:/qmlresources/icons/white/unlock.svg" : "qrc:/qmlresources/icons/white/lock.svg"
                fillMode: Image.PreserveAspectFit
                smooth: true
            }
        }
    }

    InformationDialog {
        id: infoDialog
        message: qsTr(
            "<h2>Control Manual del Robot</h2>"
            + "<p>Esta pantalla permite controlar el robot de forma manual utilizando el joystick virtual.</p>"
            + "<h2>Desbloqueo del joystick</h2>"
            + "<p>Pulse el candado a la derecha del joystick para desbloquearlo.</p>"
            + "<h2>Movimiento del joystick</h2>"
            + "<p>Una vez desbloqueado, desplace el círculo pequeño para comenzar a moverse. La velocidad de movimiento aumentará cuanto más se aleje del centro.</p>"
            + "<ul>"
            + "<li><b>Hacia adelante:</b> Avanza en línea recta.</li>"
            + "<li><b>Hacia atrás:</b> Retrocede en línea recta.</li>"
            + "<li><b>Hacia la izquierda:</b> Gira en sentido horario (hacia la derecha).</li>"
            + "<li><b>Hacia la derecha:</b> Gira en sentido antihorario (hacia la izquierda).</li>"
            + "<li><b>Posiciones intermedias:</b> El robot se moverá en una combinación de traslación y giro, ajustando su dirección según la inclinación del joystick.</li>"
            + "</ul>"
            + "<h2>Bloqueo del joystick</h2>"
            + "<p>Pulse el candado a la derecha del joystick para bloquearlo.</p>"
            + "<h2><span style='color:#c52020'>¡¡ADVERTENCIA!!</span></h2>"
            + "<p>Este robot no cuenta con un sistema de gestión de colisiones. Asegúrese de evitar obstáculos y supervisar su desplazamiento en todo momento.</p>"
            + "<h2>¿Necesita ayuda?</h2>"
            + "<p>Para asistencia técnica, contacte con ETSIDI en: "
            + "<a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p>"
        )
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
