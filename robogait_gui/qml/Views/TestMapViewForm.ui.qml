import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import MapRendering 1.0

import "qrc:/Dialogs"

Rectangle {
    id: root
    color: "#518bb7"

    property alias infoButton: infoButton
    property alias emergencyButton: emergencyButton
    property alias infoDialog: infoDialog
    property alias mapLayerItem: mapLayerItem
    property alias robotLayerItem: robotLayerItem
    property alias pathLayerItem: pathLayerItem
    property alias livePathLayerItem: livePathLayerItem

    property alias zoomInButton: zoomInButton
    property alias zoomOutButton: zoomOutButton
    property alias fitButton: fitButton
    property alias followButton: followButton

    property int step: 0
    readonly property bool isPositionStep: step === 0
    readonly property bool isOrientationStep: step === 1
    readonly property bool isNavigationStep: step === 2
    property real iconButtonSizePx: 0
    property real iconGlyphSizePx: 0
    property real headerTopInsetPx: 0
    property real buttonHeightPx: 0
    property real wheelSizePx: 0
    property real emergencyButtonSizePx: iconButtonSizePx * 2
    property real emergencyIconSizePx: emergencyButtonSizePx * 0.6
    property real headerButtonSpacing: 8

    signal confirmPlacementRequested()
    signal confirmOrientationRequested()
    signal backOrientationRequested()
    signal autoLocalizationRequested()
    signal goalAcceptRequested()
    signal goalClearRequested()
    signal pathClearRequested()
    signal startTestRequested()
    signal goHomeRequested()

    property bool mapAvailable: false
    property bool particleCloudAvailable: false
    property int mapContentMargin: 10
    property bool placementEnabled: false
    property bool orientationEnabled: false
    property bool orientationOverride: false
    property bool goalPlacementEnabled: false
    property bool pathPlacementEnabled: false
    property bool goalPointSet: false
    property bool goalOrientationEnabled: false
    property bool goalOrientationSet: false
    property bool goalAccepted: false
    property bool goalPathReady: false
    property bool testStarted: false
    property real goalOrientationDeg: 0
    property var goalMapPosition: Qt.point(0, 0)
    property var placementController: (userSession.rosManager && userSession.rosManager.robotManager)
                                      ? userSession.rosManager.robotManager.robotPlacementController
                                      : null
    property bool showRobotPose: false
    property bool showParticleCloud: false


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        anchors.topMargin: 20 + headerTopInsetPx
        spacing: 15

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
                    text: qsTr("Prueba")
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

            Rectangle {
                id: goalActionPanel
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 21
                anchors.bottomMargin: 21
                color: "#2c5f7c"
                radius: 10
                border.color: "#6aa3c8"
                border.width: 2
                z: 70
                visible: mapAvailable && isNavigationStep && goalPlacementEnabled && !testStarted

                property int padding: 10
                property real buttonWidth: 160
                width: buttonWidth + (padding * 2)
                height: (root.buttonHeightPx * 2) + (padding * 2) + 8

                Column {
                    anchors.fill: parent
                    anchors.margins: goalActionPanel.padding
                    spacing: 8

                    Button {
                        id: goalAcceptButton
                        width: goalActionPanel.buttonWidth
                        height: root.buttonHeightPx
                        enabled: goalPointSet && goalOrientationSet
                        opacity: enabled ? 1.0 : 0.4

                        background: Rectangle {
                            radius: 6
                            color: goalAcceptButton.pressed ? "#1a3a4a" : "#3a7fa0"
                            border.color: "#ffffff"
                            border.width: 1
                        }

                        contentItem: Text {
                            text: qsTr("ACEPTAR")
                            color: "#ffffff"
                            font.pixelSize: 14
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: goalAcceptRequested()
                    }

                    Button {
                        id: goalClearButton
                        width: goalActionPanel.buttonWidth
                        height: root.buttonHeightPx
                        enabled: goalPointSet
                        opacity: enabled ? 1.0 : 0.4

                        background: Rectangle {
                            radius: 6
                            color: goalClearButton.pressed ? "#1a3a4a" : "#3a7fa0"
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

                        onClicked: goalClearRequested()
                    }
                }
            }

            Rectangle {
                id: pathActionPanel
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 21
                anchors.bottomMargin: 21
                color: "#2c5f7c"
                radius: 10
                border.color: "#6aa3c8"
                border.width: 2
                z: 70
                visible: mapAvailable && isNavigationStep && pathPlacementEnabled && !testStarted

                property int padding: 10
                property real buttonWidth: 160
                width: buttonWidth + (padding * 2)
                height: root.buttonHeightPx + (padding * 2)

                Button {
                    id: pathClearButton
                    anchors.fill: parent
                    anchors.margins: pathActionPanel.padding
                    enabled: mapVisualizationManager &&
                             mapVisualizationManager.manualPathEditor &&
                             mapVisualizationManager.manualPathEditor.hasPath
                    opacity: enabled ? 1.0 : 0.4

                    background: Rectangle {
                        radius: 6
                        color: pathClearButton.pressed ? "#1a3a4a" : "#3a7fa0"
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

                    onClicked: pathClearRequested()
                }
            }

            MapLayerItem {
                id: mapLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable
                z: 1
                isPanningEnabled: !placementEnabled && !goalPlacementEnabled && !pathPlacementEnabled && !testStarted

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    gesturePolicy: TapHandler.DragThreshold
                    onTapped: {
                        if (testStarted) {
                            return
                        }
                        if (mapAvailable && isNavigationStep && pathPlacementEnabled) {
                            return
                        }
                        if (mapAvailable && isNavigationStep && goalPlacementEnabled) {
                            if (root.handleGoalTap) {
                                root.handleGoalTap(point.position.x, point.position.y)
                            }
                            return
                        }
                        if (!mapAvailable || !isPositionStep || !placementEnabled || !placementController) {
                            return
                        }
                        if (placementController.hasPosition) {
                            return
                        }
                        placementController.setPositionFromScreenCoordinates(point.position.x, point.position.y)
                    }
                }

                DragHandler {
                    id: pathDragHandler
                    target: null
                    acceptedButtons: Qt.LeftButton
                    dragThreshold: 0
                    enabled: mapAvailable && isNavigationStep && pathPlacementEnabled && !testStarted

                    onActiveChanged: {
                        if (active) {
                            if (root.handlePathStrokeStart) {
                                root.handlePathStrokeStart(centroid.position.x, centroid.position.y)
                            }
                        }
                        else {
                            if (root.handlePathStrokeEnd) {
                                root.handlePathStrokeEnd()
                            }
                        }
                    }

                    onCentroidChanged: {
                        if (!active) {
                            return
                        }
                        if (root.handlePathStrokeMove) {
                            root.handlePathStrokeMove(centroid.position.x, centroid.position.y)
                        }
                    }

                    onCanceled: function() {
                        if (root.handlePathStrokeEnd) {
                            root.handlePathStrokeEnd()
                        }
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

            RobotLayerItem {
                id: goalRobotLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable && isNavigationStep && goalPlacementEnabled && goalPointSet && !testStarted
                z: 2
                headColor: "#3b82f6"

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerGoalRobotLayerItem(goalRobotLayerItem)
                    }
                }
            }

            PathLayerItem {
                id: pathLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable && isNavigationStep
                z: 1.45
                pathColor: "#9118DB"

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerPathLayerItem(pathLayerItem)
                    }
                }
            }

            PathLayerItem {
                id: livePathLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable && isNavigationStep
                z: 1.46
                pathColor: "#18DB22"

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerLivePathLayerItem(livePathLayerItem)
                    }
                }
            }

            ParticleCloudLayerItem {
                id: particleCloudLayerItem
                anchors.fill: parent
                anchors.margins: mapContentMargin
                visible: mapAvailable && particleCloudAvailable && showParticleCloud
                z: 1.4

                Component.onCompleted: {
                    if (userSession.rosManager &&
                        userSession.rosManager.robotManager &&
                        userSession.rosManager.robotManager.mapVisualizationManager)
                    {
                        userSession.rosManager.robotManager.mapVisualizationManager.registerParticleCloudLayerItem(particleCloudLayerItem)
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
                property real wheelSize: wheelSizePx > 0 ? wheelSizePx
                                                       : Math.min(240, Math.min(parent.width, parent.height) * 0.32)

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
                            width: rotationPanel.wheelSize / 4
                            height: width
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
                id: goalRotationPanel
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.leftMargin: 21
                anchors.bottomMargin: 21
                color: goalOrientationEnabled ? "#2c5f7c" : "#3b4a55"
                radius: 13
                border.color: goalOrientationEnabled ? "#6aa3c8" : "#5f707d"
                border.width: 3
                z: 50
                visible: mapAvailable && isNavigationStep && goalPlacementEnabled && goalPointSet && !goalAccepted && !testStarted
                opacity: goalOrientationEnabled ? 1.0 : 0.6

                property int padding: 13
                property real wheelSize: wheelSizePx > 0 ? wheelSizePx
                                                       : Math.min(240, Math.min(parent.width, parent.height) * 0.32)

                implicitWidth: goalPanelContent.implicitWidth + (padding * 2)
                implicitHeight: goalPanelContent.implicitHeight + (padding * 2)

                Column {
                    id: goalPanelContent
                    anchors.fill: parent
                    anchors.margins: goalRotationPanel.padding
                    spacing: 10

                    Rectangle {
                        width: goalRotationPanel.wheelSize
                        height: 36
                        color: goalOrientationEnabled ? "#1a3a4a" : "#2e3a43"
                        radius: 6
                        opacity: 0.9

                        Text {
                            anchors.centerIn: parent
                            color: "#ffffff"
                            font.pixelSize: 16
                            font.bold: true
                            text: qsTr("Orientación: %1°").arg(Math.round(((goalRotationOverlay.degrees + 360) % 360)))
                        }
                    }

                    Item {
                        id: goalRotationOverlay
                        width: goalRotationPanel.wheelSize
                        height: width

                        readonly property real orientationRad: goalOrientationDeg * Math.PI / 180
                        readonly property real degrees: goalOrientationDeg
                        property real radius: Math.max(0, (width * 0.5) - 14)

                        function updateOrientationFromPoint(px, py)
                        {
                            if (!goalOrientationEnabled)
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
                            var current = goalOrientationDeg
                            var delta = deg - current
                            while (delta > 180) delta -= 360
                            while (delta < -180) delta += 360
                            var eased = current + (delta * 0.35)
                            goalOrientationDeg = eased
                            goalOrientationSet = true
                        }

                        Rectangle {
                            anchors.fill: parent
                            radius: width / 2
                            color: "transparent"
                            border.color: goalOrientationEnabled ? "#ffffff" : "#9aa8b1"
                            border.width: 2
                        }

                        Canvas {
                            id: goalDirectionMarker
                            width: goalRotationPanel.wheelSize / 4
                            height: width
                            x: (goalRotationOverlay.width / 2) + radius * Math.cos(goalRotationOverlay.orientationRad) - width / 2
                            y: (goalRotationOverlay.height / 2) - radius * Math.sin(goalRotationOverlay.orientationRad) - height / 2
                            rotation: 90 - goalRotationOverlay.degrees
                            transformOrigin: Item.Center

                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                ctx.fillStyle = goalOrientationEnabled ? "#ffffff" : "#9aa8b1"
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
                            enabled: goalOrientationEnabled
                            minimumTouchPoints: 1
                            maximumTouchPoints: 1
                            onTouchUpdated: {
                                if (touchPoints.length > 0) {
                                    var p = touchPoints[0]
                                    goalRotationOverlay.updateOrientationFromPoint(p.x, p.y)
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            onPressed: function(mouse) { goalRotationOverlay.updateOrientationFromPoint(mouse.x, mouse.y) }
                            onPositionChanged: function(mouse) {
                                if (pressed) {
                                    goalRotationOverlay.updateOrientationFromPoint(mouse.x, mouse.y)
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
            Layout.preferredHeight: Math.max(60, root.buttonHeightPx + 16)
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
                    height: root.buttonHeightPx
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
                    height: root.buttonHeightPx
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
                    height: root.buttonHeightPx
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
                    height: root.buttonHeightPx
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
                    height: root.buttonHeightPx
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
                    height: root.buttonHeightPx
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
            Layout.preferredHeight: Math.max(60, root.iconButtonSizePx + 16)
            color: "#2c5f7c"
            radius: 6
            visible: isNavigationStep
            opacity: mapAvailable ? 1.0 : 0.5
            enabled: mapAvailable

            RowLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 8
                anchors.rightMargin: 8
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
                }

                Button {
                    id: followButton
                    Layout.preferredWidth: root.iconButtonSizePx
                    Layout.preferredHeight: root.iconButtonSizePx
                    Layout.alignment: Qt.AlignVCenter
                    checkable: true

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
                }

                Item { Layout.fillWidth: true }

                Button {
                    id: startTestButton
                    Layout.preferredWidth: 150
                    Layout.preferredHeight: root.buttonHeightPx
                    Layout.alignment: Qt.AlignVCenter
                    visible: goalAccepted && !testStarted
                    enabled: goalPathReady
                    opacity: enabled ? 1.0 : 0.4

                    background: Rectangle {
                        radius: 6
                        color: startTestButton.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: qsTr("INICIAR TEST")
                        color: "#ffffff"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: startTestRequested()
                }
            }

            Row {
                id: navigationGoalButtons
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12
                z: 10
                visible: !testStarted

                Button {
                    id: goalModeButton
                    width: root.iconButtonSizePx
                    height: root.iconButtonSizePx

                    background: Rectangle {
                        radius: 6
                        color: goalPlacementEnabled ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/white/goal.svg"
                        width: root.iconGlyphSizePx
                        height: root.iconGlyphSizePx
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    onClicked: {
                        if (goalPlacementEnabled) {
                            goalPlacementEnabled = false
                        }
                        else {
                            goalPlacementEnabled = true
                            pathPlacementEnabled = false
                        }
                    }
                }

                Button {
                    id: pathModeButton
                    width: root.iconButtonSizePx
                    height: root.iconButtonSizePx
                    opacity: enabled ? 1.0 : 0.5
                    enabled: mapAvailable && isNavigationStep

                    background: Rectangle {
                        radius: 6
                        color: pathPlacementEnabled ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/white/path.svg"
                        width: root.iconGlyphSizePx
                        height: root.iconGlyphSizePx
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    onClicked: {
                        if (pathPlacementEnabled) {
                            pathPlacementEnabled = false
                        }
                        else {
                            pathPlacementEnabled = true
                            goalPlacementEnabled = false
                        }
                    }
                }

                Button {
                    id: placeholderModeButton
                    width: root.iconButtonSizePx
                    height: root.iconButtonSizePx
                    enabled: false
                    opacity: 0.35

                    background: Rectangle {
                        radius: 6
                        color: "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Item {}
                }
            }

            Row {
                id: navigationHomeButton
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                z: 10
                visible: testStarted

                Button {
                    id: homeButton
                    width: root.iconButtonSizePx
                    height: root.iconButtonSizePx

                    background: Rectangle {
                        radius: 6
                        color: homeButton.pressed ? "#1a3a4a" : "#3a7fa0"
                        border.color: "#ffffff"
                        border.width: 1
                    }

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/white/home.svg"
                        width: root.iconGlyphSizePx
                        height: root.iconGlyphSizePx
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    onClicked: goHomeRequested()
                }
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
