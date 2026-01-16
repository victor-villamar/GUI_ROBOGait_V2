import QtQuick 2.15
import QtQuick.Controls 2.15
import "extras"

Rectangle {
    id: root
    color: "#518bb7"
    property alias mapPageForm_buttonStop: buttonStop

    property alias mapPageForm_mystackview: mapPageForm_mystackview
    property alias mapPageForm_buttonNext: mapPageForm_buttonNext
    property alias mapPageForm_buttonPrevious: mapPageForm_buttonPrevious
    property alias mapPageForm_text: mapPageForm_text
    property alias mapPageForm_orientationCircleForm: orientationCircle
    property alias mapPageForm_boxImages: boxImages

    property alias infoForm: infoForm
    property alias infoButton: infoButton

    property string mapPageForm_previousState: ""
    property string mapPageForm_nextState: ""

    property string mapPageForm_previousState_text: "Anterior"
    property string mapPageForm_nextState_text: "Siguiente"

    property alias mp_map: map

    Item {
        id: rectagle_conteiner
        width: parent.width / 3
        height: parent.height
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.right: parent.right
        Text {
            id: mapPageForm_text
            x: 922
            width: parent.width - 20
            height: parent.width / 8
            color: "#ffffff"
            text: qsTr("Al hacer clic en el mapa, se coloca el robot sobre él")
            anchors.right: parent.right
            anchors.top: parent.top
            font.pixelSize: 30
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.styleName: "Medium"
            font.bold: true
            wrapMode: Text.Wrap
            anchors.rightMargin: 18
            anchors.topMargin: -20
        }

        OrientationCircle {
            id: orientationCircle
            anchors.top: mapPageForm_text.bottom
            anchors.horizontalCenter: mapPageForm_text.horizontalCenter
            anchors.topMargin: 100
        }

        BoxImages {
            id: boxImages
            anchors.top: mapPageForm_text.bottom
            anchors.horizontalCenterOffset: 40
            anchors.horizontalCenter: mapPageForm_text.horizontalCenter
            anchors.topMargin: 70
        }

        Button {
            id: mapPageForm_buttonNext
            x: 8
            y: 376
            width: 196
            height: 48
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 9
            anchors.bottomMargin: 16
            font.pointSize: 40
            background: Rectangle {
                color: "transparent" // Fondo transparente
                border.color: "transparent" // Sin bordes
            }

            contentItem: Item {
                id: row
                anchors.centerIn: parent

                Text {
                    anchors.right: svgImage.left
                    anchors.verticalCenter: parent.verticalCenter
                    font.pointSize: 25
                    color: "white"
                    text: qsTr(mapPageForm_nextState_text)
                    anchors.rightMargin: 10
                }
                Image {
                    id: svgImage
                    source: "../images/icon_app/keyboard_left.svg"
                    rotation: 180
                    width: 40
                    height: 40
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    fillMode: Image.PreserveAspectFit
                }
            }
        }

        Button {
            id: mapPageForm_buttonPrevious
            width: 196
            height: 48
            anchors.right: mapPageForm_buttonNext.left
            anchors.bottom: parent.bottom
            anchors.rightMargin: 20
            anchors.bottomMargin: 16
            font.pointSize: 40
            background: Rectangle {
                color: "transparent" // Fondo transparente
                border.color: "transparent" // Sin bordes
            }

            contentItem: Row {
                id: row2
                // Contenido del botón
                anchors.centerIn: parent
                spacing: 10

                Image {
                    id: svgImage2
                    source: "../images/icon_app/keyboard_left.svg"
                    anchors.verticalCenterOffset: 3
                    width: 40
                    height: 40
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                    // Alto de la imagen
                }
                Text {
                    text: qsTr(mapPageForm_previousState_text)
                    horizontalAlignment: Text.AlignRight
                    font.pointSize: 25
                    color: "white" // Color del texto
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    StackView {
        id: mapPageForm_mystackview
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        initialItem: Map {
            id: map
        }
        pushEnter: null
        pushExit: null
        popEnter: null
        popExit: null
    }
    Button {
        id: buttonStop
        height: 100
        anchors.right: rectagle_conteiner.left
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
    Button {
        id: infoButton
        width: 37
        height: 37
        anchors.verticalCenter: explain.verticalCenter
        anchors.right: rectagle_conteiner.left
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.rightMargin: -7
        background: Image {
            source: "../images/icon_app/circle-info-solid.svg"
            fillMode: Image.PreserveAspectFit
        }
        //        Behavior on scale {
        //            NumberAnimation {
        //                duration: 150
        //                easing.type: Easing.OutQuad
        //            }
        //        }

        //        // Change scale when pressed/released
        //        onPressed: scale = 1.2
        //        onReleased: scale = 1.0
    }

    PopUpInfoForm {
        id: infoForm
        anchors.centerIn: parent
        visible: false

        textInfo.text: "<h2>Información de la pantalla</h2>\
<p>Esta pantalla permite controlar el robot a través del joystick.</p>\
<h2><span style='color:red;'>¡¡ADVERTENCIA!!</span></h2>\
<p>Este robot no cuenta con un sistema de gestión de colisiones. Asegúrese de evitar obstáculos y supervisar su desplazamiento en todo momento.</p>\
<h2>Movimientos del robot</h2>\
<ul>\
<li><b>Hacia adelante:</b> Avanza en línea recta.</li>\
<li><b>Hacia atrás:</b> Retrocede en línea recta.</li>\
<li><b>Hacia la izquierda:</b> Gira en sentido horario (hacia la derecha).</li>\
<li><b>Hacia la derecha:</b> Gira en sentido antihorario (hacia la izquierda).</li>\
<li><b>Posiciones intermedias:</b> El robot se moverá en una combinación de traslación y giro, ajustando su dirección según la inclinación del joystick.</li>\
</ul>\
<h2>¿Necesita ayuda?</h2>\
<p>Para asistencia técnica, contacte con ETSIDI en: <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.9;height:700;width:1300}D{i:2}D{i:3}D{i:4}D{i:5}
D{i:10}D{i:1}D{i:15}D{i:17}D{i:19}D{i:21}
}
##^##*/
