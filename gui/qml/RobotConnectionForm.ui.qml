import QtQuick 2.15
import QtQuick.Controls 2.15
import RobotDiscovery 1.0
import "extras"

Item {
    id: root
    property alias buttonSearchRobot: buttonSearchRobot
    property alias background: background
    property bool showDiscoveryPanel : rosManager.robotDiscovery.state === RobotDiscovery.SCANNING ||
                                       rosManager.robotDiscovery.state === RobotDiscovery.ROBOTS_FOUND

    Rectangle {
        id: background
        color: "#518bb7"
        anchors.fill: parent

        // Title
        Text {
            id: titleText
            color: "#ffffff"
            text: qsTr("CONEXIÓN AL ROBOT")
            font.pixelSize: 32
            font.styleName: "Medium"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: parent.height < 600 ? 10 : 20
        }

        // Main text with scroll
        ScrollView {
            id: scrollView
            width: Math.min(900, parent.width * 0.90)
            height: Math.min(parent.height * 0.6, instructionsColumn.implicitHeight + 8)
            anchors.top: titleText.bottom
            anchors.topMargin: parent.height < 600 ? 20 : 40
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true

            Column {
                id: instructionsColumn
                width: scrollView.width
                spacing: 20

                // Title 1
                Text {
                    text: qsTr("1º ENCENDER EL ORDENADOR DEL ROBOT MÓVIL")
                    color: "#00C8FF"
                    font.pixelSize: 20
                    font.bold: true
                }

                Text {
                    id: step1Text
                    width: scrollView.width
                    color: "#ffffff"
                    font.pixelSize: 17
                    wrapMode: Text.WordWrap
                    text: qsTr("Para encenderlo, sigue estos pasos:\n\n    1.  Conecta el ordenador (NUC) a la batería portátil (power bank).\n    2.  Enciende la batería portátil y ajusta el voltaje hasta que marque 20V.\n    3.  Comprueba que los dos cables de las antenas WiFi estén bien conectadas al ordenador.\n    4.  Enciende el ordenador (NUC).\n    5.  Espera unos segundos hasta que el indicador LED de la conexión WiFi se mantenga en verde fijo (sin parpadear).")
                }

                // Title 2
                Text {
                    text: qsTr("2º CONEXIÓN A LA RED WIFI")
                    color: "#00C8FF"
                    font.pixelSize: 20
                    font.bold: true
                }

                Text {
                    id: step2Text
                    width: scrollView.width
                    color: "#ffffff"
                    font.pixelSize: 17
                    wrapMode: Text.WordWrap
                    text: qsTr("Conéctate a la red WiFi del robot móvil desde la tablet.\nEnciende la tablet, abre la configuración de WiFi y selecciona la red con el nombre robogait2024.")
                }

                // Title 3
                Text {
                    text: qsTr("3º ENCENDER LA BASE DEL ROBOT")
                    color: "#00C8FF"
                    font.pixelSize: 20
                    font.bold: true
                }

                Text {
                    id: step3Text
                    width: scrollView.width
                    color: "#ffffff"
                    font.pixelSize: 17
                    wrapMode: Text.WordWrap
                    text: qsTr("Presiona el botón que se encuentra en la base del robot.\nSabrás que se ha encendido correctamente cuando:\n\n     1.  Se encienda un luz LED azul.\n     2.  Escuches un pitido agudo.")
                }
            }
        }

        Item {
            id: bottomSlot
            width: scrollView.width
            anchors.top: scrollView.bottom
            anchors.topMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height < 600 ? 20 : 40
            anchors.horizontalCenter: parent.horizontalCenter

            // Search button
            Button {
                id: buttonSearchRobot
                height: 52
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                visible: !root.showDiscoveryPanel

                // Definimos los colores
                // property color normalColor: "#ffffff"
                // property color pressedColor: "#00C8FF"  // Color cuando está presionado
                // property color textColor: "#045671"
                // property color borderColor: "#045671"

                background: Rectangle {
                    color: buttonSearchRobot.down ? "#00C8FF" : "#ffffff"
                    radius: 8
                    border.color: "#045671"
                    border.width: 2

                    // Animación solo para el cambio de color
                    Behavior on color {
                        ColorAnimation { duration: 100 }  // Duración muy corta para respuesta inmediata
                    }
                }

                contentItem: Label {
                    id: buttonLabel
                    text: qsTr("Buscar Robot")
                    color: "#045671"
                    font.pixelSize: 17
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.fill: parent
                    padding: 0
                }
            }

            Item {
                id: robotsPanel
                width: scrollView.width
                anchors.fill: parent
                anchors.horizontalCenter: parent.horizontalCenter
                visible: root.showDiscoveryPanel
                clip: true

                Column {
                    anchors.fill: parent
                    anchors.topMargin: 8
                    spacing: 8
                    Text {
                            text: qsTr("ROBOTS DETECTADOS")
                            color: "#00C8FF"
                            font.pixelSize: 20
                            font.bold: true
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            visible: rosManager.robotDiscovery.state === RobotDiscovery.ROBOTS_FOUND
                    }

                    BusyIndicator {
                        id: searchingIndicator
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 180
                        height: 180
                        running: true
                        topPadding: 20
                        visible: rosManager.robotDiscovery.state === RobotDiscovery.SCANNING
                    }

                    Text {
                        text: qsTr("Buscando robots…")
                        color: "#ffffff"
                        font.pixelSize: 14
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        visible: searchingIndicator.visible
                    }

                    Rectangle {
                        width: parent.width
                        height: Math.max(0, parent.height - 60)
                        color: "transparent"
                        border.color: "#ffffff"
                        border.width: 1
                        radius: 6
                        clip: true
                        visible: rosManager.robotDiscovery.state === RobotDiscovery.ROBOTS_FOUND

                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: robotsColumn.implicitHeight
                            clip: true

                            Column {
                                id: robotsColumn
                                width: parent.width
                                spacing: 6
                                padding: 10

                                Repeater {
                                    model: rosManager.robotDiscovery.robots
                                    delegate: Text {
                                        text: modelData
                                        color: "#ffffff"
                                        font.pixelSize: 16
                                        width: parent.width
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:2}D{i:4}D{i:3}D{i:5}D{i:1}
}
##^##*/

