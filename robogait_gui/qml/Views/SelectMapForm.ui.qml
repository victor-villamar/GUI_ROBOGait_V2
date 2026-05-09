import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import AppTheme 1.0

Item {
    id: root

    property alias addMapButton: addMapButton
    property alias mapsListView: mapsListView
    property alias mapsScrollView: mapsScrollView
    property string selectedMapName: ""
    readonly property real buttonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 60
    readonly property real listItemHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.listItemHeight, 0) : 60
    readonly property real iconGlyphPx: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 25
    readonly property real iconButtonPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.iconButtonSize, 0) : 44

    signal addMapRequested()
    signal mapClicked(int mapIndex, string mapName)
    signal mapDeleteRequested(int mapIndex, string mapName)

    Rectangle {
        anchors.fill: parent
        color: AppTheme.selection.background
    }

    Text {
        id: titleText
        text: qsTr("SELECCIONE UN MAPA")
        color: AppTheme.selection.white
        font.pixelSize: 30
        horizontalAlignment: Text.AlignHCenter
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 20
    }

    Button {
        id: addMapButton
        width: mapsScrollView.width
        height: root.buttonHeightPx
        anchors.top: titleText.bottom
        anchors.topMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter

        background: Rectangle {
            color: AppTheme.selection.white
            radius: 15
        }

        contentItem: Item {
            anchors.fill: parent

            Text {
                text: qsTr("Crear nuevo mapa")
                font.pixelSize: 22
                color: AppTheme.selection.black
                anchors.centerIn: parent
            }

            Image {
                source: "qrc:/qmlresources/icons/color/plus_solid.svg"
                sourceSize.width: root.iconGlyphPx
                sourceSize.height: root.iconGlyphPx
                fillMode: Image.PreserveAspectFit
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 20
            }
        }

        onClicked: root.addMapRequested()
    }

    ScrollView {
        id: mapsScrollView
        width: Math.min(720, parent.width * 0.70)
        anchors.top: addMapButton.bottom
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.horizontalCenter: parent.horizontalCenter
        clip: true

        ListView {
            id: mapsListView
            currentIndex: -1
            anchors.fill: parent
            clip: true
            spacing: 10

            delegate: Item {
                width: mapsListView.width
                height: root.listItemHeightPx

                Rectangle {
                    id: rowRect
                    width: parent.width
                    height: root.listItemHeightPx
                    radius: 15
                    color: map_name === root.selectedMapName ? AppTheme.selection.lightBlue : AppTheme.selection.white

                    Text {
                        text: map_name
                        font.pixelSize: 22
                        color: AppTheme.selection.black
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.mapClicked(index, map_name)
                    }

                    Button {
                        id: deleteMapButton
                        width: root.iconButtonPx
                        height: root.iconButtonPx
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 20

                        background: Rectangle { color: "transparent" }

                        contentItem: Image {
                            source: "qrc:/qmlresources/icons/color/trash_solid.svg"
                            sourceSize.width: root.iconGlyphPx
                            sourceSize.height: root.iconGlyphPx
                            fillMode: Image.PreserveAspectFit
                        }

                        onClicked: root.mapDeleteRequested(index, map_name)
                    }
                }
            }
        }
    }
}
