import QtQuick 2.15
import AppTheme 1.0

Item {
    id: root

    property bool running: visible
    property color dotColor: AppTheme.core.white
    property int dotCount: 12
    property real dotSize: Math.max(8, Math.min(width, height) * 0.075)
    property real radiusPx: Math.max(0, Math.min(width, height) * 0.34)
    property int durationMs: 900

    implicitWidth: 96
    implicitHeight: 96

    Repeater {
        model: root.dotCount

        Rectangle {
            width: root.dotSize
            height: root.dotSize
            radius: width / 2
            color: root.dotColor
            opacity: root.dotCount > 1 ? 0.25 + (index / (root.dotCount - 1)) * 0.75 : 1.0
            x: root.width / 2 + Math.cos((index * 2 * Math.PI) / root.dotCount) * root.radiusPx - width / 2
            y: root.height / 2 + Math.sin((index * 2 * Math.PI) / root.dotCount) * root.radiusPx - height / 2
        }
    }

    RotationAnimator on rotation {
        from: 0
        to: 360
        duration: root.durationMs
        loops: Animation.Infinite
        running: root.running && root.visible
    }
}
