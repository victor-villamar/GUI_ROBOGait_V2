import QtQuick 2.15

import "qrc:/Components"

OrientationWheel {
    id: root

    property bool mapAvailable: false
    property bool isNavigationStep: false
    property bool pathPlacementEnabled: false
    property bool pathTerminalPoseSet: false
    property bool testStarted: false
    property real wheelSizePx: 0
    property real inputAngleDeg: 0

    signal orientationChanged(real deg)

    anchors.left: parent.left
    anchors.bottom: parent.bottom
    anchors.leftMargin: 21
    anchors.bottomMargin: 21
    z: 50
    visiblePanel: root.mapAvailable &&
                  root.isNavigationStep &&
                  root.pathPlacementEnabled &&
                  root.pathTerminalPoseSet &&
                  !root.testStarted
    enabled: root.pathTerminalPoseSet
    wheelSize: root.wheelSizePx > 0 ? root.wheelSizePx : Math.min(240, Math.min(parent.width, parent.height) * 0.32)
    title: qsTr("Orientación")
    angleDeg: root.inputAngleDeg

    onAngleChanged: function(deg) {
        if (!root.pathTerminalPoseSet) {
            return
        }
        root.orientationChanged(deg)
    }
}
