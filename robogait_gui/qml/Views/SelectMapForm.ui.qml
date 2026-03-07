import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property alias addMapButton: addMapButton
    property alias mapsListView: mapsListView
    property alias mapsScrollView: mapsScrollView
    property int selectedMapIndex: -1

    signal addMapRequested()
    signal mapClicked(int mapIndex, string mapName)
    signal mapDeleteRequested(int mapIndex, string mapName)

    Rectangle {
        anchors.fill: parent
        color: "#518bb7"
    }

    Text {
        id: titleText
        text: qsTr("SELECCIONE UN MAPA")
        color: "#ffffff"
        font.pixelSize: 30
        horizontalAlignment: Text.AlignHCenter
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 20
    }

    Button {
        id: addMapButton
        width: mapsScrollView.width
        height: 60
        anchors.top: titleText.bottom
        anchors.topMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter

        background: Rectangle {
            color: "#ffffff"
            border.color: "#cccccc"
            radius: 15
        }

        contentItem: Item {
            anchors.fill: parent

            Text {
                text: qsTr("Crear nuevo mapa")
                font.pixelSize: 22
                color: "#000000"
                anchors.centerIn: parent
            }

            Image {
                source: "qrc:/qmlresources/icons/plus_solid.svg"
                sourceSize.width: 25
                sourceSize.height: 25
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

            delegate: Item {
                width: mapsListView.width
                height: 70

                Rectangle {
                    id: rowRect
                    width: parent.width
                    height: 60
                    radius: 15
                    color: index === root.selectedMapIndex ? "#aed2ea" : "#ffffff"
                    border.color: "#cccccc"

                    Text {
                        text: map_name
                        font.pixelSize: 22
                        color: "#000000"
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.mapClicked(index, map_name)
                    }

                    Button {
                        id: deleteMapButton
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 20

                        background: Rectangle { color: "transparent" }

                        contentItem: Image {
                            source: "qrc:/qmlresources/icons/trash_solid.svg"
                            sourceSize.width: 25
                            sourceSize.height: 25
                            fillMode: Image.PreserveAspectFit
                        }

                        onClicked: root.mapDeleteRequested(index, map_name)
                    }
                }
            }
        }
    }
}