import QtQuick 2.15

TapHandler {
    id: root

    property bool mapAvailable: false
    property bool isPositionStep: false
    property bool placementEnabled: false
    property bool testStarted: false
    property var placementController: null

    acceptedButtons: Qt.LeftButton
    gesturePolicy: TapHandler.DragThreshold

    onTapped: {
        if (root.testStarted)
        {
            return
        }
        if (!root.mapAvailable || !root.isPositionStep || !root.placementEnabled || !root.placementController)
        {
            return
        }
        if (root.placementController.hasPosition)
        {
            return
        }
        root.placementController.setPositionFromScreenCoordinates(point.position.x, point.position.y)
    }
}
