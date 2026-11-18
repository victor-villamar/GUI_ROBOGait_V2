import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

Rectangle {
    id: root

    property real buttonHeightPx: 44
    property bool mapAvailable: false
    property bool isNavigationStep: false
    property bool goalPlacementEnabled: false
    property bool testStarted: false
    property bool goalPointSet: false
    property bool goalOrientationSet: false

    signal calculateRequested()
    signal clearRequested()

    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.rightMargin: 21
    anchors.bottomMargin: 21
    color: AppTheme.map.panel
    radius: 10
    border.color: AppTheme.map.panelBorder
    border.width: 2
    z: 70
    visible: root.mapAvailable && root.isNavigationStep && root.goalPlacementEnabled && !root.testStarted

    property int padding: 10
    property real buttonWidth: 200
    width: buttonWidth + (padding * 2)
    height: (root.buttonHeightPx * 2) + (padding * 2) + 8

    Column {
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: 8

        Button {
            id: goalAcceptButton
            width: root.buttonWidth
            height: root.buttonHeightPx
            enabled: root.goalPointSet && root.goalOrientationSet
            opacity: enabled ? 1.0 : 0.4

            background: Rectangle {
                radius: 6
                color: goalAcceptButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Text {
                text: qsTr("CALCULAR")
                color: AppTheme.map.white
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.calculateRequested()
        }

        Button {
            id: goalClearButton
            width: root.buttonWidth
            height: root.buttonHeightPx
            enabled: root.goalPointSet
            opacity: enabled ? 1.0 : 0.4

            background: Rectangle {
                radius: 6
                color: goalClearButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Text {
                text: qsTr("BORRAR")
                color: AppTheme.map.white
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.clearRequested()
        }
    }
}
