import QtQuick 2.15

import "qrc:/Components"

OrientationWheel {
    id: root

    property bool mapAvailable: false
    property bool isOrientationStep: false
    property bool orientationEnabled: false
    property real wheelSizePx: 0
    property var placementController: null

    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.rightMargin: 21
    anchors.bottomMargin: 21
    z: 50
    visiblePanel: root.mapAvailable && root.isOrientationStep && root.placementController && root.placementController.hasPosition
    enabled: root.orientationEnabled
    wheelSize: root.wheelSizePx > 0 ? root.wheelSizePx : Math.min(240, Math.min(parent.width, parent.height) * 0.32)
    title: qsTr("Orientación")
    angleDeg: root.placementController ? (root.placementController.theta * 180 / Math.PI) : 0

    onAngleChanged: function(deg) {
        if (!root.placementController || !root.placementController.hasPosition || !root.orientationEnabled)
        {
            return
        }
        root.placementController.setOrientationDegrees(deg)
    }
}
