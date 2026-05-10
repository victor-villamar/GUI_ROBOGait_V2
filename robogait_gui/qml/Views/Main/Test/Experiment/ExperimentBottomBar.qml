import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

Item {
    id: root

    property real buttonHeightPx: 44
    property real iconButtonSizePx: 50
    property real iconGlyphSizePx: 25
    property bool mapAvailable: false
    property bool followChecked: false
    property bool homeVisible: false
    property bool repeatVisible: false
    property bool exitMode: false

    signal zoomOutRequested()
    signal zoomInRequested()
    signal fitRequested()
    signal followRequested()
    signal goHomeRequested()
    signal repeatRequested()
    signal finishRequested()
    signal exitRequested()

    Row {
        id: leftControls
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Button {
            id: zoomOutButton
            width: root.iconButtonSizePx
            height: root.iconButtonSizePx
            enabled: root.mapAvailable
            opacity: enabled ? 1.0 : 0.5

            background: Rectangle {
                radius: 6
                color: zoomOutButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/white/minus.svg"
                width: root.iconGlyphSizePx
                height: root.iconGlyphSizePx
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: root.zoomOutRequested()
        }

        Button {
            id: zoomInButton
            width: root.iconButtonSizePx
            height: root.iconButtonSizePx
            enabled: root.mapAvailable
            opacity: enabled ? 1.0 : 0.5

            background: Rectangle {
                radius: 6
                color: zoomInButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/white/plus.svg"
                width: root.iconGlyphSizePx
                height: root.iconGlyphSizePx
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: root.zoomInRequested()
        }

        Button {
            id: fitButton
            width: root.iconButtonSizePx
            height: root.iconButtonSizePx
            enabled: root.mapAvailable
            opacity: enabled ? 1.0 : 0.5

            background: Rectangle {
                radius: 6
                color: fitButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/white/center_to_fit.svg"
                width: root.iconGlyphSizePx
                height: root.iconGlyphSizePx
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: root.fitRequested()
        }

        Button {
            id: followButton
            width: root.iconButtonSizePx
            height: root.iconButtonSizePx
            enabled: root.mapAvailable
            opacity: enabled ? 1.0 : 0.5
            checkable: true
            checked: root.followChecked

            background: Rectangle {
                radius: 6
                color: followButton.checked ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/white/center_view.svg"
                width: root.iconGlyphSizePx
                height: root.iconGlyphSizePx
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: root.followRequested()
        }
    }

    Button {
        id: homeButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        visible: root.homeVisible
        width: root.iconButtonSizePx
        height: root.iconButtonSizePx

        background: Rectangle {
            radius: 6
            color: homeButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
            border.color: AppTheme.map.white
            border.width: 1
        }

        contentItem: Image {
            source: "qrc:/qmlresources/icons/white/home.svg"
            width: root.iconGlyphSizePx
            height: root.iconGlyphSizePx
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        onClicked: root.goHomeRequested()
    }

    Row {
        id: rightControls
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Button {
            id: repeatButton
            width: 210
            height: root.iconButtonSizePx
            visible: root.repeatVisible

            background: Rectangle {
                radius: 6
                color: repeatButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Text {
                text: qsTr("REPETIR EXPERIMENTO")
                color: AppTheme.map.white
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.repeatRequested()
        }

        Button {
            id: finishButton
            width: 220
            height: root.iconButtonSizePx

            background: Rectangle {
                radius: 6
                color: finishButton.pressed ? AppTheme.map.panelHeader : AppTheme.map.actionButton
                border.color: AppTheme.map.white
                border.width: 1
            }

            contentItem: Text {
                text: root.exitMode ? qsTr("SALIR") : qsTr("TERMINAR EXPERIMENTO")
                color: AppTheme.map.white
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                if (root.exitMode) {
                    root.exitRequested()
                } else {
                    root.finishRequested()
                }
            }
        }
    }
}
