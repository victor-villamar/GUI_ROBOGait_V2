import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

Rectangle {
    id: root

    property real buttonHeightPx: 44
    property bool mapAvailable: false
    property bool isNavigationStep: false
    property bool pathPlacementEnabled: false
    property bool testStarted: false
    property var splinePathEditor: null

    signal calculateRequested()
    signal smoothRequested()
    signal editToggleRequested()
    signal clearRequested()

    readonly property bool hasPath: root.splinePathEditor && root.splinePathEditor.hasPath
    readonly property bool isSmoothed: root.splinePathEditor && root.splinePathEditor.isSmoothed
    readonly property bool isEditMode: root.splinePathEditor && root.splinePathEditor.isEditMode
    readonly property bool hasEditablePath: root.splinePathEditor && root.splinePathEditor.hasEditablePath

    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.rightMargin: 21
    anchors.bottomMargin: 21
    color: AppTheme.map.panel
    radius: 10
    border.color: AppTheme.map.panelBorder
    border.width: 2
    z: 70
    visible: root.mapAvailable && root.isNavigationStep && root.pathPlacementEnabled && !root.testStarted

    property int padding: 10
    property real buttonWidth: 200
    property int buttonsSpacing: 8
    readonly property int visibleButtons: (pathAcceptButton.visible ? 1 : 0) +
                                          (pathSegmentButton.visible ? 1 : 0) +
                                          (pathEditButton.visible ? 1 : 0) +
                                          (pathClearButton.visible ? 1 : 0)
    width: buttonWidth + (padding * 2)
    height: (root.buttonHeightPx * visibleButtons) +
            (buttonsSpacing * Math.max(0, visibleButtons - 1)) +
            (padding * 2)

    Column {
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: root.buttonsSpacing

        Button {
            id: pathAcceptButton
            width: root.buttonWidth
            height: root.buttonHeightPx
            visible: root.isSmoothed
            enabled: root.hasEditablePath
            opacity: enabled ? 1.0 : 0.4

            background: Rectangle {
                radius: 6
                color: pathAcceptButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
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
            id: pathSegmentButton
            width: root.buttonWidth
            height: root.buttonHeightPx
            visible: !root.isSmoothed
            enabled: root.hasPath && !root.isSmoothed
            opacity: enabled ? 1.0 : 0.4

            background: Rectangle {
                radius: 6
                color: pathSegmentButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Text {
                text: qsTr("SUAVIZAR")
                color: AppTheme.map.white
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.smoothRequested()
        }

        Button {
            id: pathEditButton
            width: root.buttonWidth
            height: root.buttonHeightPx
            visible: root.isSmoothed
            enabled: root.hasEditablePath
            opacity: enabled ? 1.0 : 0.4

            background: Rectangle {
                radius: 6
                color: root.isEditMode || pathEditButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Text {
                text: root.isEditMode ? qsTr("FINALIZAR EDICIÓN") : qsTr("EDITAR")
                color: AppTheme.map.white
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.editToggleRequested()
        }

        Button {
            id: pathClearButton
            width: root.buttonWidth
            height: root.buttonHeightPx
            enabled: root.hasPath
            opacity: enabled ? 1.0 : 0.4

            background: Rectangle {
                radius: 6
                color: pathClearButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
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
