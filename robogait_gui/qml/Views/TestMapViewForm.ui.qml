import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import MapRendering 1.0

import "qrc:/Dialogs"

Rectangle {
    id: root
    color: "#518bb7"

    property alias infoButton: infoButton
    property alias infoDialog: infoDialog
    property alias mapLayerItem: mapLayerItem
    property alias robotLayerItem: robotLayerItem

    property alias zoomInButton: zoomInButton
    property alias zoomOutButton: zoomOutButton
    property alias fitButton: fitButton
    property alias followButton: followButton

    property int step: 0
    readonly property bool isPositionStep: step === 0
    readonly property bool isOrientationStep: step === 1
    readonly property bool isNavigationStep: step === 2

    signal confirmPlacementRequested()
    signal confirmOrientationRequested()
    signal backOrientationRequested()
    signal autoLocalizationRequested()

    property bool mapAvailable: false
    property int mapContentMargin: 10
    property bool placementEnabled: false
    property bool orientationEnabled: false
    property bool orientationOverride: false
    property var placementController: (userSession.rosManager && userSession.rosManager.robotManager)
                                      ? userSession.rosManager.robotManager.robotPlacementController
                                      : null
    property bool showRobotPose: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

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
                    text: qsTr("Prueba")
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

        Rectangle {
            id: mapDisplayArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1a3a4a"
            radius: 8
            border.color: "#2c5f7c"
            border.width: 2
            clip: true

            RobotLayerItem {
                id: robotLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: showRobotPose
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




            MapLayerItem {
                id: mapLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable
                z: 1
                isPanningEnabled: !placementEnabled

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    gesturePolicy: TapHandler.DragThreshold
                    onTapped: {
                        if (!mapAvailable || !isPositionStep || !placementEnabled || !placementController) {
                            return
                        }
                        if (placementController.hasPosition) {
                            return
                        }
                        placementController.setPositionFromScreenCoordinates(point.position.x, point.position.y)
                    }
                }

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerMapLayerItem(mapLayerItem)
                    }
                }
            }



            Rectangle {
                id: rotationPanel
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 21
                anchors.bottomMargin: 21
                color: orientationEnabled ? "#2c5f7c" : "#3b4a55"
                radius: 13
                border.color: orientationEnabled ? "#6aa3c8" : "#5f707d"
                border.width: 3
                z: 50
                visible: mapAvailable && isOrientationStep && placementController && placementController.hasPosition
                opacity: orientationEnabled ? 1.0 : 0.6

                property int padding: 13
                property real wheelSize: Math.min(240, Math.min(parent.width, parent.height) * 0.32)

                implicitWidth: panelContent.implicitWidth + (padding * 2)
                implicitHeight: panelContent.implicitHeight + (padding * 2)

                Column {
                    id: panelContent
                    anchors.fill: parent
                    anchors.margins: rotationPanel.padding
                    spacing: 10

                    Rectangle {
                        width: rotationPanel.wheelSize
                        height: 36
                        color: orientationEnabled ? "#1a3a4a" : "#2e3a43"
                        radius: 6
                        opacity: 0.9

                        Text {
                            anchors.centerIn: parent
                            color: "#ffffff"
                            font.pixelSize: 16
                            font.bold: true
                            text: qsTr("Orientación: %1°").arg(Math.round(((rotationOverlay.degrees + 360) % 360)))
                        }
                    }

                    Item {
                        id: rotationOverlay
                        width: rotationPanel.wheelSize
                        height: width

                        readonly property real orientationRad: placementController ? placementController.theta : 0.0
                        readonly property real degrees: orientationRad * 180 / Math.PI
                        property real radius: Math.max(0, (width * 0.5) - 14)

                        function updateOrientationFromPoint(px, py) 
                        {
                            if (!placementController || !placementController.hasPosition || !orientationEnabled)
                            {
                                return
                            }
                            var dx = px - width / 2
                            var dy = py - height / 2
                            if (dx === 0 && dy === 0)
                            {
                                return
                            }
                            var angle = Math.atan2(-dy, dx)
                            var deg = angle * 180 / Math.PI
                            var current = rotationOverlay.degrees
                            var delta = deg - current
                            while (delta > 180) delta -= 360
                            while (delta < -180) delta += 360
                            var eased = current + (delta * 0.35)
                            placementController.setOrientationDegrees(eased)
                        }

                        Rectangle {
                            anchors.fill: parent
                            radius: width / 2
                            color: "transparent"
                            border.color: orientationEnabled ? "#ffffff" : "#9aa8b1"
                            border.width: 2
                        }

                        Canvas {
                            id: directionMarker
                            width: 28
                            height: 28
                            x: (rotationOverlay.width / 2) + radius * Math.cos(rotationOverlay.orientationRad) - width / 2
                            y: (rotationOverlay.height / 2) - radius * Math.sin(rotationOverlay.orientationRad) - height / 2
                            rotation: 90 - rotationOverlay.degrees
                            transformOrigin: Item.Center

                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                ctx.fillStyle = orientationEnabled ? "#ffffff" : "#9aa8b1"
                                var cx = width / 2
                                var headY = 0
                                var headW = width * 0.7
                                var tailW = width * 0.35
                                var tailY = height * 0.65
                                ctx.beginPath()
                                ctx.moveTo(cx, headY)
                                ctx.lineTo(cx + headW / 2, tailY)
                                ctx.lineTo(cx + tailW / 2, tailY)
                                ctx.lineTo(cx + tailW / 2, height)
                                ctx.lineTo(cx - tailW / 2, height)
                                ctx.lineTo(cx - tailW / 2, tailY)
                                ctx.lineTo(cx - headW / 2, tailY)
                                ctx.closePath()
                                ctx.fill()
                            }
                        }

                        MultiPointTouchArea {
                            anchors.fill: parent
                            enabled: orientationEnabled
                            minimumTouchPoints: 1
                            maximumTouchPoints: 1
                            onTouchUpdated: {
                                if (touchPoints.length > 0) {
                                    var p = touchPoints[0]
                                    rotationOverlay.updateOrientationFromPoint(p.x, p.y)
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            onPressed: function(mouse) { rotationOverlay.updateOrientationFromPoint(mouse.x, mouse.y) }
                            onPositionChanged: function(mouse) {
                                if (pressed) {
                                    rotationOverlay.updateOrientationFromPoint(mouse.x, mouse.y)
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: emptyMapPlaceholder
                anchors.fill: parent
                anchors.margins: mapContentMargin
                color: "#1a3a4a"
                border.color: "#2c5f7c"
                visible: !mapAvailable
                z: 0
            }

            Text {
                anchors.centerIn: parent
                text: qsTr("Esperando mapa...")
                color: "#ffffff"
                opacity: 0.7
                font.pixelSize: 16
                visible: !mapAvailable
            }
        }

        Rectangle {
            id: bottomBar
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#2c5f7c"
            radius: 6
            visible: !isNavigationStep

            Item {
                id: bottomBarContent
                anchors.fill: parent
                anchors.margins: 8

                Row {
                    id: placementButtons
                    spacing: 10
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    visible: isPositionStep

                    Button {
                        id: enablePlacementButton
                        width: 200
                        height: 44
                        checkable: true
                        checked: placementEnabled

                        background: Rectangle {
                            radius: 6
                            color: enablePlacementButton.checked ? "#1a3a4a" : "#3a7fa0"
                            border.color: "#ffffff"
                            border.width: 1
                        }

                        contentItem: Text {
                            text: qsTr("HABILITAR COLOCACIÓN")
                            color: "#ffffff"
                            font.pixelSize: 14
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: placementEnabled = !placementEnabled
                    }

                    Button {
                        id: clearPlacementButton
                        width: 140
                        height: 44
                        enabled: isPositionStep && placementEnabled && placementController && placementController.hasPosition
                        opacity: enabled ? 1.0 : 0.4

                        background: Rectangle {
                            radius: 6
                            color: clearPlacementButton.pressed ? "#1a3a4a" : "#3a7fa0"
                            border.color: "#ffffff"
                            border.width: 1
                        }

                        contentItem: Text {
                            text: qsTr("BORRAR")
                            color: "#ffffff"
                            font.pixelSize: 14
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (placementController) {
                                placementController.clear()
                            }
                        }
                    }
                }

                Button {
                    id: autoLocalizationButton
                    width: 200
                    height: 44
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    visible: isPositionStep
                    enabled: isPositionStep
                    opacity: enabled ? 1.0 : 1.0

                    background: Rectangle {
                        radius: 6
                        color: autoLocalizationButton.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: qsTr("AUTOLOCALIZAR")
                        color: "#ffffff"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        autoLocalizationRequested()
                    }
                }


                Button {
                    id: enableOrientationButton
                    width: 210
                    height: 44
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    visible: isOrientationStep
                    enabled: true
                    background: Rectangle {
                        radius: 6
                        color: orientationEnabled ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: qsTr("HABILITAR ORIENTACIÓN")
                        color: "#ffffff"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (orientationEnabled) {
                            orientationEnabled = false
                            return
                        }

                        orientationEnabled = true
                        orientationOverride = true
                        if (placementController) {
                            placementController.setOrientationDegrees(0)
                        }
                    }
                }

                Button {
                    id: backOrientationButton
                    width: 140
                    height: 44
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    visible: isOrientationStep
                    enabled: true

                    background: Rectangle {
                        radius: 6
                        color: backOrientationButton.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: qsTr("ATRÁS")
                        color: "#ffffff"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        backOrientationRequested()
                    }
                }

                Button {
                    id: confirmPlacementButton
                    width: 160
                    height: 44
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    visible: isPositionStep || isOrientationStep
                    enabled: isPositionStep ? (placementController && placementController.hasPosition)
                                              : (isOrientationStep && (placementController && placementController.hasOrientation))
                    opacity: enabled ? 1.0 : 0.4

                    background: Rectangle {
                        radius: 6
                        color: confirmPlacementButton.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: qsTr("CONFIRMAR")
                        color: "#ffffff"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (isPositionStep) {
                            confirmPlacementRequested()
                        } else if (isOrientationStep) {
                            confirmOrientationRequested()
                        }
                    }
                }
            }
        }

        Rectangle {
            id: navigationBar
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#2c5f7c"
            radius: 6
            visible: isNavigationStep
            opacity: mapAvailable ? 1.0 : 0.5
            enabled: mapAvailable

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 10

                Button {
                    id: zoomOutButton
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 44

                    background: Rectangle {
                        radius: 6
                        color: parent.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/minus.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }
                }

                Button {
                    id: zoomInButton
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 44

                    background: Rectangle {
                        radius: 6
                        color: parent.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/plus.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }
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
                }

                Button {
                    id: followButton
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 44
                    checkable: true

                    background: Rectangle {
                        radius: 6
                        color: followButton.checked ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/center_view.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }
                }

                Item { Layout.fillWidth: true }
            }
        }

    }

    InformationDialog {
        id: infoDialog
        message: qsTr(
            "<h2>Información de la pantalla</h2>"
            + "<p>Esta pantalla se divide en tres pasos: posición, orientación y navegación.</p>"
            + "<h2>Pasos</h2>"
            + "<ul>"
            + "<li><b>Posición:</b> Coloca el robot tocando el mapa.</li>"
            + "<li><b>Orientación:</b> Gira el robot con la rueda hasta dejar la dirección correcta.</li>"
            + "<li><b>Navegación:</b> Se muestran controles de zoom y centrado.</li>"
            + "</ul>"
            + "<h2>Botones</h2>"
            + "<ul>"
            + "<li><b>Habilitar colocación:</b> Permite colocar el robot tocando el mapa.</li>"
            + "<li><b>Borrar:</b> Elimina la posición del robot para volver a colocarla.</li>"
            + "<li><b>Confirmar:</b> Avanza al siguiente paso.</li>"
            + "</ul>"
            + "<h2>¿Necesita ayuda?</h2>"
            + "<p>Para asistencia técnica, contacte con ETSIDI en: "
            + "<a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p>"
        )
    }
}
