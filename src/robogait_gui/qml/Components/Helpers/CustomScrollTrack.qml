import QtQuick 2.15
import AppTheme 1.0

Item {
    id: root

    property var flickable: null
    property var formColumn: null
    property var startItem: null
    property bool trackVisible: false

    property real edgeMargin: 2
    property real gapFromForm: 6
    property real trackWidth: 10
    property real minTrackHeight: 80
    property real minThumbHeight: 36

    property color trackColor: AppTheme.core.scrollTrack
    property color thumbColor: AppTheme.core.white
    property real trackOpacity: 0.35
    property real thumbOpacity: 0.95

    readonly property bool ready: !!flickable && !!formColumn && !!startItem
    readonly property real sectionTop: ready ? formColumn.y + startItem.y : 0
    readonly property real sectionBottom: ready ? formColumn.y + formColumn.height : 0
    readonly property real viewportTop: ready ? sectionTop - flickable.contentY : 0
    readonly property real viewportBottom: ready ? sectionBottom - flickable.contentY : 0
    readonly property real viewportSpan: Math.max(0, viewportBottom - viewportTop)
    readonly property real viewportOffset: Math.max(0, viewportTop)
    readonly property real availableHeight: Math.max(0, flickable.height - viewportOffset)
    readonly property real clampedRatio: ready ? Math.max(0.0, Math.min(1.0, flickable.visibleArea.heightRatio)) : 0
    readonly property real clampedPos: ready ? Math.max(0.0, Math.min(1.0, flickable.visibleArea.yPosition)) : 0

    visible: trackVisible && ready
    z: 20
    width: trackWidth
    height: ready ? Math.max(minTrackHeight, Math.min(availableHeight, viewportSpan)) : 0

    x: ready ? Math.max(
                   flickable.x + edgeMargin,
                   Math.min(
                       flickable.x + flickable.width - width - edgeMargin,
                       flickable.x + formColumn.x + formColumn.width + gapFromForm
                   )
               ) : 0

    y: ready ? flickable.y + Math.max(0, Math.min(flickable.height - height, viewportTop)) : 0

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: root.trackColor
        opacity: root.trackOpacity
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        radius: width / 2
        color: root.thumbColor
        opacity: root.thumbOpacity
        height: Math.max(root.minThumbHeight, parent.height * root.clampedRatio)
        y: (parent.height - height) * root.clampedPos
    }
}
