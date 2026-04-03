import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: home
    property alias buttonStart: buttonStart
    signal appExitRequested()

    readonly property real iconButtonSizePx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.iconButtonSize, 0) : 50
    readonly property real iconGlyphSizePx: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 24

    Rectangle {
        id: background
        anchors.fill: parent
        color: "#518bb7"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 0
            spacing: 0

            RowLayout {
                id: topRow
                Layout.fillWidth: true
                Layout.preferredHeight: 90
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.topMargin: 10
                spacing: 12

                Image {
                    id: upm
                    Layout.preferredWidth: 110
                    Layout.preferredHeight: 90
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    source: "qrc:/qmlresources/logos/upm.png"
                    fillMode: Image.PreserveAspectFit
                }

                Image {
                    id: car
                    Layout.preferredWidth: 110
                    Layout.preferredHeight: 90
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    source: "qrc:/qmlresources/logos/car.png"
                    fillMode: Image.PreserveAspectFit
                }

                Item {
                    Layout.fillWidth: true
                }

                Image {
                    id: etsidi
                    Layout.preferredWidth: 110
                    Layout.preferredHeight: 90
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                    source: "qrc:/qmlresources/logos/etsidi.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            Item {
                id: centerArea
                Layout.fillWidth: true
                Layout.fillHeight: true

                Column {
                    anchors.centerIn: parent
                    spacing: 20

                    Image {
                        id: logo
                        width: home.height < 500 ? 150 : 197
                        height: width
                        source: "qrc:/qmlresources/logos/robogait_logo.png"
                        fillMode: Image.PreserveAspectFit
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Button {
                        id: buttonStart
                        height: home.height < 500 ? 80 : 100
                        width: contentItem.implicitWidth + (home.height < 500 ? 60 : 80)
                        anchors.horizontalCenter: parent.horizontalCenter

                        background: Rectangle {
                            color: buttonStart.down ? "#e9e9e9" : "#aed2ea"
                            radius: 15
                            border.color: "#e9e9e9"
                            border.width: 2
                        }

                        contentItem: Label {
                            id: buttonLabel
                            text: qsTr("START")
                            color: "#518bb7"
                            font.pixelSize: home.height < 500 ? 40 : 60
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.styleName: "Regular"
                            font.weight: Font.Light
                            anchors.fill: parent
                            padding: 0
                        }
                    }
                }
            }

            Item {
                id: bottomRow
                Layout.fillWidth: true
                Layout.preferredHeight: 110
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.bottomMargin: 10

                Image {
                    id: ministerio
                    width: 360
                    height: 110
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: -60
                    anchors.bottomMargin: 0
                    source: "qrc:/qmlresources/logos/ministerio.png"
                    fillMode: Image.PreserveAspectFit
                }

                Button {
                    id: shutdownButton
                    width: home.iconButtonSizePx
                    height: home.iconButtonSizePx
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.rightMargin: 10
                    anchors.bottomMargin: 0
                    padding: 0
                    background: null

                    contentItem: Image {
                        source: "qrc:/qmlresources/icons/shutdown.svg"
                        width: home.iconGlyphSizePx
                        height: home.iconGlyphSizePx
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    onClicked: home.appExitRequested()
                }
            }
        }
    }
}
