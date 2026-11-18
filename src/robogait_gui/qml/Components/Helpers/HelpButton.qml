import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: root

    property real glyphSize: 24
    property url iconSource: "qrc:/qmlresources/icons/white/circle_info_solid.svg"

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
            source: root.iconSource
            width: root.glyphSize
            height: root.glyphSize
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
    onCanceled: scale = 1.0
}
