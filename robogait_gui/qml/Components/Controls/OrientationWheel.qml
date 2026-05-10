import QtQuick 2.15
import AppTheme 1.0

Rectangle {
    id: root

    property real angleDeg: 0
    property bool enabled: true
    property bool visiblePanel: true
    property real wheelSize: 200
    property string title: qsTr("Orientación")
    property color panelEnabledColor: AppTheme.orientationWheel.panelEnabled
    property color panelDisabledColor: AppTheme.orientationWheel.panelDisabled
    property color borderEnabledColor: AppTheme.orientationWheel.borderEnabled
    property color borderDisabledColor: AppTheme.orientationWheel.borderDisabled
    property color headerEnabledColor: AppTheme.orientationWheel.headerEnabled
    property color headerDisabledColor: AppTheme.orientationWheel.headerDisabled
    property color markerEnabledColor: AppTheme.orientationWheel.white
    property color markerDisabledColor: AppTheme.orientationWheel.markerDisabled
    property real easingFactor: 0.35
    property int padding: 13

    signal angleChanged(real deg)
    onAngleDegChanged: directionMarker.requestPaint()
    onEnabledChanged: directionMarker.requestPaint()
    onWheelSizeChanged: directionMarker.requestPaint()

    color: enabled ? panelEnabledColor : panelDisabledColor
    radius: 13
    border.color: enabled ? borderEnabledColor : borderDisabledColor
    border.width: 3
    opacity: enabled ? 1.0 : 0.6
    visible: visiblePanel

    implicitWidth: panelContent.implicitWidth + (padding * 2)
    implicitHeight: panelContent.implicitHeight + (padding * 2)

    function normalizedDeg(value) {
        var deg = value
        while (deg > 180) deg -= 360
        while (deg < -180) deg += 360
        return deg
    }

    function updateAngleFromPoint(px, py) {
        if (!enabled)
        {
            return
        }

        var dx = px - rotationOverlay.width / 2
        var dy = py - rotationOverlay.height / 2
        if (dx === 0 && dy === 0)
        {
            return
        }

        var targetDeg = Math.atan2(-dy, dx) * 180 / Math.PI
        var current = angleDeg
        var delta = targetDeg - current

        while (delta > 180) delta -= 360
        while (delta < -180) delta += 360

        var eased = normalizedDeg(current + (delta * easingFactor))
        angleChanged(eased)
    }

    Column {
        id: panelContent
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: 10

        Rectangle {
            width: root.wheelSize
            height: 36
            color: root.enabled ? root.headerEnabledColor : root.headerDisabledColor
            radius: 6
            opacity: 0.9

            Text {
                anchors.centerIn: parent
                color: AppTheme.orientationWheel.white
                font.pixelSize: 16
                font.bold: true
                text: qsTr("%1: %2°").arg(root.title).arg(Math.round(((root.angleDeg + 360) % 360)))
            }
        }

        Item {
            id: rotationOverlay
            width: root.wheelSize
            height: width

            readonly property real orientationRad: root.angleDeg * Math.PI / 180
            property real markerRadius: Math.max(0, (width * 0.5) - 14)

            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: "transparent"
                border.color: root.enabled ? root.markerEnabledColor : root.markerDisabledColor
                border.width: 2
            }

            Canvas {
                id: directionMarker
                anchors.fill: parent

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    ctx.strokeStyle = root.enabled ? root.markerEnabledColor : root.markerDisabledColor
                    ctx.fillStyle = root.enabled ? root.markerEnabledColor : root.markerDisabledColor
                    ctx.lineWidth = Math.max(3, width * 0.028)
                    ctx.lineCap = "round"

                    var cx = width / 2
                    var cy = height / 2
                    var shaftRadius = Math.max(22, width * 0.28)
                    var ex = cx + shaftRadius * Math.cos(rotationOverlay.orientationRad)
                    var ey = cy - shaftRadius * Math.sin(rotationOverlay.orientationRad)

                    // Shaft from center to near the head
                    var headLen = Math.max(10, width * 0.10)
                    var vx = ex - cx
                    var vy = ey - cy
                    var vlen = Math.sqrt((vx * vx) + (vy * vy))
                    if (vlen < 1e-6)
                    {
                        return
                    }

                    var ux = vx / vlen
                    var uy = vy / vlen
                    var sx = ex - (ux * headLen)
                    var sy = ey - (uy * headLen)

                    ctx.beginPath()
                    ctx.moveTo(cx, cy)
                    ctx.lineTo(sx, sy)
                    ctx.stroke()

                    // Arrow head
                    var nx = -uy
                    var ny = ux
                    var halfW = Math.max(8, width * 0.045)
                    ctx.beginPath()
                    ctx.moveTo(ex, ey)
                    ctx.lineTo(sx + (nx * halfW), sy + (ny * halfW))
                    ctx.lineTo(sx - (nx * halfW), sy - (ny * halfW))
                    ctx.closePath()
                    ctx.fill()
                }
            }

            MultiPointTouchArea {
                anchors.fill: parent
                enabled: root.enabled
                minimumTouchPoints: 1
                maximumTouchPoints: 1
                onTouchUpdated: {
                    if (touchPoints.length > 0) {
                        var p = touchPoints[0]
                        root.updateAngleFromPoint(p.x, p.y)
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onPressed: function(mouse) { root.updateAngleFromPoint(mouse.x, mouse.y) }
                onPositionChanged: function(mouse) {
                    if (pressed)
                    {
                        root.updateAngleFromPoint(mouse.x, mouse.y)
                    }
                }
            }
        }
    }
}
