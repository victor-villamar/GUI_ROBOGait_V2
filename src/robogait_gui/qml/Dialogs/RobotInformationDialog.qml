import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0
import AppTheme 1.0

Dialog {
    id: root

    property var robotManager: null
    property string robotDisplayName: ""

    modal: true
    dim: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay

    readonly property real buttonHeightPx: uiSizingSettings
                                                ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0)
                                                : 52
    readonly property real listItemHeightPx: uiSizingSettings
                                                  ? uiSizingSettings.interactivePx(uiSizingSettings.listItemHeight, 0)
                                                  : 48
    readonly property var statusItems: robotManager && robotManager.robotStatusItems
                                       ? robotManager.robotStatusItems
                                       : []

    width: parent ? Math.min(560, parent.width * 0.9) : 560
    height: parent ? Math.min(Math.max(340, contentColumn.implicitHeight + 40), parent.height * 0.82) : 520
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: AppTheme.settings.overlayDim
    }

    background: Rectangle {
        color: AppTheme.settings.lightBlue
        border.color: AppTheme.settings.white
        border.width: 2
        radius: 12
    }

    header: Rectangle {
        height: 54
        color: AppTheme.settings.primaryDark
        radius: 12

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 12
            color: AppTheme.settings.primaryDark
        }

        Text {
            anchors.left: parent.left
            anchors.right: closeButton.left
            anchors.leftMargin: 20
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Información de %1").arg(root.robotDisplayName)
            color: AppTheme.settings.white
            font.pixelSize: 18
            font.bold: true
            elide: Text.ElideRight
        }

        Button {
            id: closeButton
            width: 40
            height: 34
            anchors.right: parent.right
            anchors.rightMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            padding: 0

            background: Rectangle {
                color: closeButton.down ? AppTheme.settings.closeButtonPressedBg : AppTheme.settings.white
                radius: 8
                border.color: AppTheme.settings.primaryDark
                border.width: 2
            }

            contentItem: Label {
                text: "✕"
                color: AppTheme.settings.primaryDark
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.close()
        }
    }

    contentItem: Flickable {
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: contentColumn.implicitHeight + 40

        ColumnLayout {
            id: contentColumn
            x: 20
            y: 20
            width: Math.max(0, parent.width - 40)
            spacing: 10

            Text {
                Layout.fillWidth: true
                text: root.statusItems.length > 0
                      ? qsTr("Estado actual del robot seleccionado")
                      : qsTr("No hay información disponible del robot")
                color: AppTheme.settings.primaryDark
                font.pixelSize: 16
                font.bold: true
                wrapMode: Text.WordWrap
            }

            Repeater {
                model: root.statusItems

                delegate: Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(root.listItemHeightPx, statusRow.implicitHeight + 20)
                    color: AppTheme.settings.white
                    radius: 8
                    border.color: AppTheme.settings.primary
                    border.width: 1

                    RowLayout {
                        id: statusRow
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Text {
                            Layout.preferredWidth: Math.min(180, implicitWidth)
                            text: modelData.label + ":"
                            color: AppTheme.settings.primaryDark
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Text {
                            Layout.fillWidth: true
                            text: modelData.value
                            color: AppTheme.settings.black
                            font.pixelSize: 14
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 8
                Layout.preferredWidth: 180
                Layout.preferredHeight: root.buttonHeightPx
                text: qsTr("Cerrar")

                background: Rectangle {
                    color: parent.down ? AppTheme.settings.primary : AppTheme.settings.white
                    radius: 8
                    border.color: AppTheme.settings.primaryDark
                    border.width: 2
                }

                contentItem: Label {
                    text: parent.text
                    color: AppTheme.settings.primaryDark
                    font.pixelSize: 16
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: root.close()
            }
        }
    }
}
