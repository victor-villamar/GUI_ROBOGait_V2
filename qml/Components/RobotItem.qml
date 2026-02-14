import QtQuick 2.15

Item {
    id: root

    property point robotScreenPos: Qt.point(0, 0)
    property real robotScreenRotation: 0.0
    property bool robotPoseAvailable: false
    property int mapWidth: 0  // Map width in pixels
    property int mapHeight: 0  // Map height in pixels

    // Visual properties
    property int robotSize: 60  // Size of robot icon in pixels

    // Calculate scaling factor from map pixels to screen pixels
    // TODO: improve map scaling calculation
    readonly property real mapScale: {
        if (mapWidth === 0 || mapHeight === 0 || width === 0 || height === 0) {
            return 1.0
        }

        var scaleX = width / mapWidth
        var scaleY = height / mapHeight
        return Math.min(scaleX, scaleY)
    }

    // Calculate actual rendered map dimensions
    readonly property real renderedMapWidth: mapWidth * mapScale
    readonly property real renderedMapHeight: mapHeight * mapScale

    // Calculate offset to center the map (PreserveAspectFit centers the image)
    readonly property real mapOffsetX: (width - renderedMapWidth) / 2
    readonly property real mapOffsetY: (height - renderedMapHeight) / 2

    // Robot visualization
    Image {
        id: robotImage
        x: mapOffsetX + (robotScreenPos.x * mapScale) - (robotSize / 2)
        y: mapOffsetY + (robotScreenPos.y * mapScale) - (robotSize / 2)
        rotation: robotScreenRotation
        width: robotSize
        height: robotSize
        source: "qrc:/qmlresources/robot/robot.png"
        fillMode: Image.PreserveAspectFit
        smooth: true
        antialiasing: true
        transformOrigin: Item.Center
        visible: true

        // Smooth transitions
        // TODO: optimize animations
        Behavior on x {
            NumberAnimation {
                duration: 100
                easing.type: Easing.OutQuad
            }
        }

        Behavior on y {
            NumberAnimation {
                duration: 100
                easing.type: Easing.OutQuad
            }
        }

        Behavior on rotation {
            NumberAnimation {
                duration: 100
                easing.type: Easing.OutQuad
            }
        }
    }
}
