import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "qrc:/Dialogs"
import "qrc:/Components"
import "qrc:/Controls"

Rectangle {
    id: root
    color: "#518bb7"

    property alias mapImage: mapImage
    property alias robotItem: robotItem
    property alias infoButton: infoButton
    property alias infoDialog: infoDialog
    property alias joystick: joystick
    property alias lockButton: lockButton
    property alias mapContainer: mapContainer
    property alias zoomInButton: zoomInButton
    property alias zoomOutButton: zoomOutButton
    property alias fitButton: fitButton

    // Properties for map manager state
    property bool mapAvailable: false
    property int mapWidth: 0
    property int mapHeight: 0
    property double mapResolution: 0.0

    property point robotScreenPos: Qt.point(0, 0)
    property real robotScreenRotation: 0.0
    property bool robotPoseAvailable: false

    // Manual control properties
    property bool manualUnlocked: false
    property real linearValue: 0.0
    property real angularValue: 0.0

    // Map navigation properties
    property real zoomLevel: 1.0
    property real panX: 0.0
    property real panY: 0.0

    // Smooth animations for zoom and pan
    Behavior on zoomLevel {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutCubic
        }
    }

    Behavior on panX {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutCubic
        }
    }

    Behavior on panY {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutCubic
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#2c5f7c"
            radius: 8

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Visualizacion del Mapa")
                    font.pixelSize: 24
                    font.bold: true
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Button {
                    id: infoButton
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 40

                    background: Rectangle {
                        radius: width / 2
                        color: "transparent"
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/circle_info_solid.svg"
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
            }
        }

        // Map Display Area
        Rectangle {
            id: mapDisplayArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1a3a4a"
            radius: 8
            border.color: "#2c5f7c"
            border.width: 2
            clip: true

            Item {
                id: mapContainer
                anchors.fill: parent
                anchors.margins: 10

                Item {
                    id: transformableContent
                    width: parent.width
                    height: parent.height
                    scale: zoomLevel
                    x: panX
                    y: panY
                    transformOrigin: Item.Center

                    Image {
                        id: mapImage
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        width: mapContainer.width
                        height: mapContainer.height
                        visible: mapAvailable
                        smooth: true
                        cache: false
                    }

                    RobotItem {
                        id: robotItem
                        anchors.fill: parent
                        robotScreenPos: root.robotScreenPos
                        robotScreenRotation: root.robotScreenRotation
                        robotPoseAvailable: root.robotPoseAvailable
                        robotSize: 60
                        mapWidth: root.mapWidth
                        mapHeight: root.mapHeight
                        visible: mapAvailable
                    }
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 20
                    visible: !mapAvailable
                    z: 100

                    Image {
                        source: "qrc:/qmlresources/icons/map.svg"
                        width: 96
                        height: 96
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: qsTr("No hay mapa disponible")
                        font.pixelSize: 20
                        font.bold: true
                        color: "#ffffff"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: qsTr("Asegúrese de:\n• El robot tiene SLAM/mapa activo\n• El robot publica en /map")
                        font.pixelSize: 14
                        color: "#aaaaaa"
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                // Multi-touch gesture handling for pan and zoom
                PinchArea {
                    anchors.fill: parent
                    enabled: mapAvailable
                    pinch.target: null
                    pinch.minimumScale: 0.5
                    pinch.maximumScale: 5.0

                    onPinchUpdated: function(pinch) {
                        // Calculate new zoom level
                        var newZoom = zoomLevel * pinch.scale
                        newZoom = Math.max(0.5, Math.min(5.0, newZoom))
                        
                        // Update zoom level
                        zoomLevel = newZoom
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: mapAvailable
                        preventStealing: false

                        property point lastPos: Qt.point(0, 0)

                        onPressed: function(mouse) {
                            lastPos = Qt.point(mouse.x, mouse.y)
                        }

                        onPositionChanged: function(mouse) {
                            if (pressed) {
                                var dx = mouse.x - lastPos.x
                                var dy = mouse.y - lastPos.y
                                
                                panX += dx
                                panY += dy
                                
                                lastPos = Qt.point(mouse.x, mouse.y)
                            }
                        }

                        onWheel: function(wheel) {
                            var delta = wheel.angleDelta.y / 120
                            var newZoom = zoomLevel + (delta * 0.1)
                            zoomLevel = Math.max(0.5, Math.min(5.0, newZoom))
                        }
                    }
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 10
                    width: 80
                    height: 30
                    color: "#2c5f7c"
                    radius: 6
                    opacity: 0.9
                    visible: mapAvailable

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Zoom: %1x").arg(zoomLevel.toFixed(1))
                        font.pixelSize: 12
                        color: "#ffffff"
                        font.bold: true
                    }
                }

            }

            // Joystick Control Panel
            Rectangle {
                id: joystickPanel
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 16
                anchors.bottomMargin: 16
                color: "#2c5f7c"
                radius: 10
                border.color: "#6aa3c8"
                border.width: 2
                z: 50
                visible: mapAvailable
                opacity: robotPoseAvailable ? 1.0 : 0.4

                property int padding: 10
                implicitWidth: panelContent.implicitWidth + (padding * 2)
                implicitHeight: panelContent.implicitHeight + (padding * 2)

                Column {
                    id: panelContent
                    anchors.fill: parent
                    anchors.margins: joystickPanel.padding
                    spacing: 8

                    Rectangle {
                        width: joystick.width
                        height: 28
                        color: "#1a3a4a"
                        radius: 6
                        visible: manualUnlocked
                        opacity: 0.9

                        Text {
                            anchors.centerIn: parent
                            text: qsTr("Lin: %1 | Ang: %2")
                                  .arg(linearValue.toFixed(2))
                                  .arg(angularValue.toFixed(2))
                            font.pixelSize: 12
                            color: "#ffffff"
                            font.bold: true
                        }
                    }

                    Row {
                        spacing: 12

                        Joystick {
                            id: joystick
                            width: Math.min(200, mapDisplayArea.width * 0.2)
                            height: width
                            mouseAreaJoystick.enabled: manualUnlocked && robotPoseAvailable
                        }

                        Button {
                            id: lockButton
                            width: 36
                            height: 36
                            anchors.verticalCenter: parent.verticalCenter

                            background: Rectangle {
                                radius: width / 2
                                color: "transparent"
                            }

                            contentItem: Image {
                                source: manualUnlocked
                                        ? "qrc:/qmlresources/icons/unlock.svg"
                                        : "qrc:/qmlresources/icons/lock.svg"
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }

                            ToolTip.visible: hovered
                            ToolTip.text: manualUnlocked 
                                          ? qsTr("Bloquear joystick") 
                                          : qsTr("Desbloquear joystick")
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#2c5f7c"
            radius: 6
            visible: mapAvailable

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 10

                Button {
                    id: zoomOutButton
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 44
                    enabled: zoomLevel > 0.5

                    background: Rectangle {
                        radius: 6
                        color: parent.enabled ? (parent.pressed ? "#1a3a4a" : "#3a7fa0") : "#555555"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/minus.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Alejar (Zoom Out)")
                }

                // Zoom In Button
                Button {
                    id: zoomInButton
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 44
                    enabled: zoomLevel < 5.0

                    background: Rectangle {
                        radius: 6
                        color: parent.enabled ? (parent.pressed ? "#1a3a4a" : "#3a7fa0") : "#555555"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/plus.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Acercar (Zoom In)")
                }

                Button {
                    id: fitButton
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 44

                    background: Rectangle {
                        radius: 6
                        color: parent.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/center_to_fit.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Ajustar a la vista")
                }

                Item {
                    Layout.fillWidth: true
                }

                Text {
                    text: qsTr("Pellizque para zoom • Arrastre para mover")
                    font.pixelSize: 12
                    color: "#ffffff"
                    opacity: 0.7
                }
            }
        }
    }

    // Information Dialog
    InformationDialog {
        id: infoDialog
        message: qsTr(
            "<h2>Visualización del Mapa</h2>"
            + "<p>Esta pantalla muestra el mapa generado por SLAM y permite controlar el robot manualmente.</p>"
            + "<h2>Requisitos</h2>"
            + "<ul>"
            + "<li><b>Robot seleccionado:</b> Debe haber un robot activo en el sistema.</li>"
            + "<li><b>SLAM activo:</b> El robot debe estar ejecutando un nodo de SLAM (cartographer, slam_toolbox, etc.).</li>"
            + "<li><b>Topic /map:</b> El robot debe publicar el mapa en el topic estándar.</li>"
            + "</ul>"
            + "<h2>Desbloqueo del joystick</h2>"
            + "<p>Pulse el candado para desbloquear el joystick.</p>"
            + "<h2>Movimiento del joystick</h2>"
            + "<p>Arrastre el joystick para mover el robot (arriba=adelante, abajo=atrás, laterales=giro). "
            + "La distancia desde el centro determina la velocidad de movimiento.</p>"
            + "<h2>Bloqueo del joystick</h2>"
            + "<p>Pulse nuevamente el candado para bloquear el joystick.</p>"
            + "<h2>Estados del mapa</h2>"
            + "<ul>"
            + "<li><b>Ocupado (negro):</b> Obstáculo detectado.</li>"
            + "<li><b>Libre (blanco):</b> Espacio navegable.</li>"
            + "<li><b>Desconocido (gris):</b> Área no explorada.</li>"
            + "</ul>"
            + "<h2><span style='color:#c52020'>¡¡ADVERTENCIA!!</span></h2>"
            + "<p>Este robot no cuenta con sistema de gestión de colisiones. Supervise el desplazamiento en todo momento "
            + "y evite obstáculos.</p>"
            + "<h2>¿Necesita ayuda?</h2>"
            + "<p>Para asistencia técnica, contacte con ETSIDI en: "
            + "<a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p>"
        )
    }
}