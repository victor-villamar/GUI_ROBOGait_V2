import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

Item {
    id: root

    property real buttonHeightPx: 44
    property bool isPositionStep: false
    property bool isOrientationStep: false
    property bool placementEnabled: false
    property bool orientationEnabled: false
    property var placementController: null

    signal togglePlacementRequested()
    signal clearPlacementRequested()
    signal autoLocalizationRequested()
    signal toggleOrientationRequested()
    signal backOrientationRequested()
    signal confirmPositionRequested()
    signal confirmOrientationRequested()

    Row {
        id: placementButtons
        spacing: 10
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        visible: root.isPositionStep

        Button {
            id: enablePlacementButton
            width: 200
            height: root.buttonHeightPx
            checkable: true
            checked: root.placementEnabled

            background: Rectangle {
                radius: 6
                color: enablePlacementButton.checked ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Text {
                text: qsTr("HABILITAR COLOCACIÓN")
                color: AppTheme.map.white
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.togglePlacementRequested()
        }

        Button {
            id: clearPlacementButton
            width: 140
            height: root.buttonHeightPx
            enabled: root.isPositionStep && root.placementEnabled && root.placementController && root.placementController.hasPosition
            opacity: enabled ? 1.0 : 0.4

            background: Rectangle {
                radius: 6
                color: clearPlacementButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
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

            onClicked: root.clearPlacementRequested()
        }
    }

    Button {
        id: autoLocalizationButton
        width: 200
        height: root.buttonHeightPx
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        visible: root.isPositionStep
        enabled: root.isPositionStep

        background: Rectangle {
            radius: 6
            color: autoLocalizationButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
            border.color: AppTheme.map.white
            border.width: 1
        }

        contentItem: Text {
            text: qsTr("AUTOLOCALIZAR")
            color: AppTheme.map.white
            font.pixelSize: 14
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        onClicked: root.autoLocalizationRequested()
    }

    Button {
        id: enableOrientationButton
        width: 210
        height: root.buttonHeightPx
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        visible: root.isOrientationStep
        enabled: true

        background: Rectangle {
            radius: 6
            color: root.orientationEnabled ? AppTheme.map.panelHeader : AppTheme.map.actionButton
            border.color: AppTheme.map.white
            border.width: 1
        }

        contentItem: Text {
            text: qsTr("HABILITAR ORIENTACIÓN")
            color: AppTheme.map.white
            font.pixelSize: 14
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        onClicked: root.toggleOrientationRequested()
    }

    Button {
        id: backOrientationButton
        width: 140
        height: root.buttonHeightPx
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        visible: root.isOrientationStep
        enabled: true

        background: Rectangle {
            radius: 6
            color: backOrientationButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
            border.color: AppTheme.map.white
            border.width: 1
        }

        contentItem: Text {
            text: qsTr("ATRÁS")
            color: AppTheme.map.white
            font.pixelSize: 14
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        onClicked: root.backOrientationRequested()
    }

    Button {
        id: confirmPlacementButton
        width: 160
        height: root.buttonHeightPx
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        visible: root.isPositionStep || root.isOrientationStep
        enabled: root.isPositionStep ? (root.placementController && root.placementController.hasPosition)
                                     : (root.isOrientationStep && (root.placementController && root.placementController.hasOrientation))
        opacity: enabled ? 1.0 : 0.4

        background: Rectangle {
            radius: 6
            color: confirmPlacementButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
            border.color: AppTheme.map.white
            border.width: 1
        }

        contentItem: Text {
            text: qsTr("CONFIRMAR")
            color: AppTheme.map.white
            font.pixelSize: 14
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        onClicked: {
            if (root.isPositionStep) {
                root.confirmPositionRequested()
            } else if (root.isOrientationStep) {
                root.confirmOrientationRequested()
            }
        }
    }
}
