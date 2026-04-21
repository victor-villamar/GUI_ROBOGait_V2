import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import MapRendering 1.0

import "qrc:/Dialogs"
import "qrc:/Controls"

Rectangle {
    id: root
    color: "#518bb7"

    property alias infoButton: infoButton
    property alias emergencyButton: emergencyButton
    property alias infoDialog: infoDialog
    property alias joystick: joystick
    property alias lockButton: lockButton
    property alias zoomInButton: zoomInButton
    property alias zoomOutButton: zoomOutButton
    property alias fitButton: fitButton
    property alias followButton: followButton
    property alias saveMapButton: saveMapButton
    property alias resetMapButton: resetMapButton
    property alias mapLayerItem: mapLayerItem
    property alias robotLayerItem: robotLayerItem
    property alias laserLayerItem: laserLayerItem

    // Properties for map manager state
    property bool mapAvailable: false

    // Properties for robot pose availability
    property bool robotPoseAvailable: false

    // Properties for laser availability
    property bool laserAvailable: false

    // Zoom level
    property real zoomLevel: 1.0

    // Map resolution (meters per pixel)
    property real mapResolution: 0.0

    // Scale bar values
    property real scaleMeters: 0.0
    property int scalePixels: 0

    // Manual control properties
    property bool manualUnlocked: false
    property bool emergencyLatched: false
    property real linearValue: 0.0
    property real angularValue: 0.0

    property int mapContentMargin: 10
    property real iconButtonSizePx: 0
    property real iconGlyphSizePx: 0
    property real headerTopInsetPx: 0
    property real buttonHeightPx: 0
    property real joystickAreaSizePx: 0
    property real emergencyButtonSizePx: iconButtonSizePx * 2
    property real emergencyIconSizePx: emergencyButtonSizePx * 0.6
    property real headerButtonSpacing: 8

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        anchors.topMargin: 20 + headerTopInsetPx
        spacing: 15

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(60, root.iconButtonSizePx + 20)
            color: "#2c5f7c"
            radius: 8

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                Item {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: root.iconButtonSizePx
                }

                Button {
                    id: infoButton
                    Layout.preferredWidth: root.iconButtonSizePx
                    Layout.preferredHeight: root.iconButtonSizePx
                    Layout.alignment: Qt.AlignVCenter
                    padding: 0
                    leftPadding: 0
                    rightPadding: 0
                    topPadding: 0
                    bottomPadding: 0

                    background: Rectangle {
                        radius: width / 2
                        color: "transparent"
                    }

                    contentItem: Item {
                        anchors.fill: parent

                        Image {
                            source: "qrc:/qmlresources/icons/white/circle_info_solid.svg"
                            width: root.iconGlyphSizePx
                            height: root.iconGlyphSizePx
                            anchors.centerIn: parent
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }
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
                    Layout.fillWidth: true
                    text: qsTr("Creacion del Mapa")
                    font.pixelSize: 24
                    font.bold: true
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Item {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: root.iconButtonSizePx
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

            Rectangle {
                id: zoomBadge
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 16
                anchors.topMargin: 16
                color: "#2c5f7c"
                radius: 6
                border.color: "#6aa3c8"
                border.width: 1
                z: 80
                visible: mapAvailable

                property int padding: 8
                implicitWidth: zoomText.implicitWidth + (padding * 2)
                implicitHeight: zoomText.implicitHeight + (padding * 2)

                Text {
                    id: zoomText
                    anchors.centerIn: parent
                    text: qsTr("Zoom: %1x").arg(Math.round(zoomLevel))
                    font.pixelSize: 12
                    font.bold: true
                    color: "#ffffff"
                }
            }

            Rectangle {
                id: scaleBadge
                anchors.left: parent.left
                anchors.top: zoomBadge.bottom
                anchors.leftMargin: 16
                anchors.topMargin: 8
                color: "#2c5f7c"
                radius: 6
                border.color: "#6aa3c8"
                border.width: 1
                z: 80
                visible: mapAvailable && scaleMeters > 0 && scalePixels > 0

                property int padding: 8
                property real meters: scaleMeters
                property int pixels: scalePixels

                function formatMeters(value) {
                    if (value < 0.1) {
                        return value.toFixed(2)
                    }
                    if (value < 1) {
                        return value.toFixed(1)
                    }
                    return value.toFixed(0)
                }

                implicitWidth: Math.max(scaleLabel.implicitWidth, scaleBar.width) + (padding * 2)
                implicitHeight: scaleLabel.implicitHeight + scaleBar.height + (padding * 2) + 4

                Column {
                    anchors.fill: parent
                    anchors.margins: scaleBadge.padding
                    spacing: 4

                    Text {
                        id: scaleLabel
                        text: scaleBadge.pixels + " px | " + scaleBadge.formatMeters(scaleBadge.meters) + " m"
                        font.pixelSize: 12
                        font.bold: true
                        color: "#ffffff"
                    }

                    Rectangle {
                        id: scaleBar
                        width: Math.max(24, scaleBadge.pixels)
                        height: 4
                        radius: 2
                        color: "#ffffff"
                    }
                }
            }

            Button {
                id: emergencyButton
                width: root.emergencyButtonSizePx > 0 ? root.emergencyButtonSizePx : root.iconButtonSizePx * 2
                height: width
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 21
                anchors.topMargin: 21
                padding: 0
                checkable: true
                z: 80

                background: Rectangle {
                    radius: width / 2
                    color: emergencyButton.checked ? "#7a8a93" : "transparent"
                    border.color: emergencyButton.checked ? "#cbd6dc" : "transparent"
                    border.width: emergencyButton.checked ? 2 : 0
                }

                contentItem: Image {
                    source: "qrc:/qmlresources/icons/color/emergency_stop.svg"
                    width: root.emergencyIconSizePx > 0 ? root.emergencyIconSizePx : root.iconGlyphSizePx * 2
                    height: width
                    anchors.centerIn: parent
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: emergencyButton.checked ? 0.6 : 1.0
                }
            }

            MapLayerItem {
                id: mapLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable
                z: 1

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerMapLayerItem(mapLayerItem)
                    }
                }
            }

            PathLayerItem {
                id: pathLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable
                z: 1.45

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerPathLayerItem(pathLayerItem)
                    }
                }
            }

            RobotLayerItem {
                id: robotLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable
                z: 2

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerRobotLayerItem(robotLayerItem)
                    }
                }
            }

            LaserLayerItem {
                id: laserLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable && laserAvailable
                z: 1.5

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerLaserLayerItem(laserLayerItem)
                    }
                }
            }

            // Placeholder when map is not available
            Rectangle {
                id: emptyMapPlaceholder
                anchors.fill: parent
                anchors.margins: mapContentMargin
                color: "#1a3a4a"
                border.color: "#2c5f7c"
                visible: !mapAvailable
                z: 0
            }

            // Joystick Control Panel
            Rectangle {
                id: joystickPanel
                anchors.right: !hasFloatPosition ? parent.right : undefined
                anchors.bottom: !hasFloatPosition ? parent.bottom : undefined
                anchors.rightMargin: !hasFloatPosition ? 21 : 0
                anchors.bottomMargin: !hasFloatPosition ? 21 : 0
                color: "#2c5f7c"
                radius: 13
                border.color: "#6aa3c8"
                border.width: 3
                z: 50
                visible: mapAvailable
                opacity: (robotPoseAvailable && !emergencyLatched) ? 1.0 : 0.4
                enabled: !emergencyLatched

                property bool pinned: true
                property bool hasFloatPosition: false
                property real floatX: 0
                property real floatY: 0
                property int floatMargin: 10

                x: hasFloatPosition ? floatX : 0
                y: hasFloatPosition ? floatY : 0

                property int padding: 13
                implicitWidth: panelContent.implicitWidth + (padding * 2)
                implicitHeight: panelContent.implicitHeight + (padding * 2)

                DragHandler {
                    target: joystickPanel
                    enabled: !joystickPanel.pinned
                    xAxis.minimum: joystickPanel.floatMargin
                    xAxis.maximum: mapDisplayArea.width - joystickPanel.width - joystickPanel.floatMargin
                    yAxis.minimum: joystickPanel.floatMargin
                    yAxis.maximum: mapDisplayArea.height - joystickPanel.height - joystickPanel.floatMargin

                    onActiveChanged: {
                        if (!active) {
                            joystickPanel.floatX = joystickPanel.x
                            joystickPanel.floatY = joystickPanel.y
                        }
                    }
                }

                Column {
                    id: panelContent
                    anchors.fill: parent
                    anchors.margins: joystickPanel.padding
                    spacing: 10

                    Rectangle {
                        width: joystick.width
                        height: 36
                        color: "#1a3a4a"
                        radius: 6
                        visible: manualUnlocked
                        opacity: 0.9

                        Text {
                            anchors.centerIn: parent
                            text: qsTr("Lin: %1 | Ang: %2")
                                  .arg(linearValue.toFixed(2))
                                  .arg(angularValue.toFixed(2))
                            font.pixelSize: 16
                            color: "#ffffff"
                            font.bold: true
                        }
                    }

                    Row {
                        spacing: 16

                        Joystick {
                            id: joystick
                            width: joystickAreaSizePx > 0 ? joystickAreaSizePx : Math.min(260, mapDisplayArea.width * 0.26)
                            height: width
                            mouseAreaJoystick.enabled: manualUnlocked && robotPoseAvailable && joystickPanel.pinned && !root.emergencyLatched
                        }

                        Button {
                            id: lockButton
                            width: root.iconButtonSizePx
                            height: root.iconButtonSizePx
                            anchors.verticalCenter: parent.verticalCenter
                            enabled: !root.emergencyLatched

                            background: Rectangle {
                                radius: width / 2
                                color: "transparent"
                            }

                            contentItem: Image {
                                source: manualUnlocked
                                        ? "qrc:/qmlresources/icons/white/unlock.svg"
                                        : "qrc:/qmlresources/icons/white/lock.svg"
                                width: root.iconGlyphSizePx
                                height: root.iconGlyphSizePx
                                anchors.centerIn: parent
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

                Button {
                    id: pinButton
                    width: root.iconButtonSizePx
                    height: root.iconButtonSizePx
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 6

                    background: Rectangle {
                        radius: width / 2
                        color: "transparent"
                    }

                    contentItem: Image {
                        anchors.centerIn: parent
                        width: root.iconGlyphSizePx
                        height: root.iconGlyphSizePx
                        source: joystickPanel.pinned
                                ? "qrc:/qmlresources/icons/white/pin.svg"
                                : "qrc:/qmlresources/icons/white/pin_empty.svg"

                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    onClicked: {
                        if (joystickPanel.pinned) {
                            joystickPanel.floatX = joystickPanel.x
                            joystickPanel.floatY = joystickPanel.y
                            joystickPanel.hasFloatPosition = true
                        }
                        joystickPanel.pinned = !joystickPanel.pinned
                    }
                }
            }
        }

        // Zoom Controls
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(60, Math.max(root.iconButtonSizePx, root.buttonHeightPx) + 16)
            color: "#2c5f7c"
            radius: 6

            opacity: mapAvailable ? 1.0 : 0.5
            enabled: mapAvailable
            visible: true

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 10

                Button {
                    id: zoomOutButton
                    Layout.preferredWidth: root.iconButtonSizePx
                    Layout.preferredHeight: root.iconButtonSizePx
                    Layout.alignment: Qt.AlignVCenter

                    background: Rectangle {
                        radius: 6
                        color: parent.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/white/minus.svg"
                        width: root.iconGlyphSizePx
                        height: root.iconGlyphSizePx
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Alejar (Zoom Out)")
                }

                Button {
                    id: zoomInButton
                    Layout.preferredWidth: root.iconButtonSizePx
                    Layout.preferredHeight: root.iconButtonSizePx
                    Layout.alignment: Qt.AlignVCenter

                    background: Rectangle {
                        radius: 6
                        color: parent.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/white/plus.svg"
                        width: root.iconGlyphSizePx
                        height: root.iconGlyphSizePx
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Acercar (Zoom In)")
                }

                Button {
                    id: fitButton
                    Layout.preferredWidth: root.iconButtonSizePx
                    Layout.preferredHeight: root.iconButtonSizePx
                    Layout.alignment: Qt.AlignVCenter

                    background: Rectangle {
                        radius: 6
                        color: parent.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/white/center_to_fit.svg"
                        width: root.iconGlyphSizePx
                        height: root.iconGlyphSizePx
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Ajustar a la vista")
                }

                Button {
                    id: followButton
                    Layout.preferredWidth: root.iconButtonSizePx
                    Layout.preferredHeight: root.iconButtonSizePx
                    checkable: true
                    Layout.alignment: Qt.AlignVCenter

                    background: Rectangle {
                        radius: 6
                        color: followButton.checked ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/white/center_view.svg"
                        width: root.iconGlyphSizePx
                        height: root.iconGlyphSizePx
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Seguir robot")
                }

                Item { Layout.fillWidth: true }

                RowLayout {
                    id: saveResetRow
                    spacing: 12
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillHeight: true

                    Button {
                        id: saveMapButton
                        Layout.preferredWidth: 110
                        Layout.preferredHeight: root.buttonHeightPx
                        Layout.alignment: Qt.AlignVCenter

                        background: Rectangle {
                            radius: 6
                            color: saveMapButton.pressed ? "#1a3a4a" : "#3a7fa0"
                            border.color: "#ffffff"
                            border.width: 1
                        }

                        contentItem: Label {
                            text: qsTr("GUARDAR")
                            color: "#ffffff"
                            font.pixelSize: 14
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Button {
                        id: resetMapButton
                        Layout.preferredWidth: 110
                        Layout.preferredHeight: root.buttonHeightPx
                        Layout.alignment: Qt.AlignVCenter

                        background: Rectangle {
                            radius: 6
                            color: resetMapButton.pressed ? "#1a3a4a" : "#3a7fa0"
                            border.color: "#ffffff"
                            border.width: 1
                        }

                        contentItem: Label {
                            text: qsTr("RESETEAR")
                            color: "#ffffff"
                            font.pixelSize: 14
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                Item { Layout.fillWidth: true }

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
