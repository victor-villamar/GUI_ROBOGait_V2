import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

Dialog {
    id: root
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose

    parent: Overlay.overlay

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: AppTheme.selection.overlayDim
    }

    width: parent ? Math.min(760, parent.width * 0.90) : 760
    height: parent ? Math.min(620, parent.height * 0.85) : 620

    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0

    property string mapName: ""
    property var details: ({})
    property string previewPath: ""
    readonly property real buttonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 44

    signal acceptedSelection(string mapName)

    function openForMap(mapNameValue, detailsValue, previewPathValue) {
        mapName = mapNameValue || ""
        details = detailsValue || ({})
        previewPath = previewPathValue || ""
        open()
    }

    background: Rectangle {
        color: AppTheme.selection.primary
        border.color: AppTheme.selection.white
        border.width: 2
        radius: 12
    }

    contentItem: Item {
        anchors.fill: parent

        Item {
            id: header
            height: 32
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16

            Button {
                id: closeButton
                width: 26
                height: 26
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                padding: 0

                background: Rectangle {
                    color: closeButton.down ? AppTheme.selection.primary : AppTheme.selection.white
                    radius: 8
                    border.color: AppTheme.selection.primaryDark
                    border.width: 2
                }

                contentItem: Image {
                    source: "qrc:/qmlresources/icons/black/xmark_solid.svg"
                    fillMode: Image.PreserveAspectFit
                    sourceSize.width: 12
                    sourceSize.height: 12
                    anchors.centerIn: parent
                }

                onClicked: root.close()
            }
        }

        Rectangle {
            id: detailsCard
            width: parent.width - 32
            height: Math.min(440, parent.height - header.height - footer.height - detailsSpacer.height - 64)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: header.bottom
            anchors.topMargin: 12
            color: AppTheme.selection.detailsCardBackground
            radius: 12
            border.color: AppTheme.selection.detailsCardBorder
            border.width: 2
            clip: true

            Column {
                id: detailsColumn
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Column {
                    id: infoColumn
                    width: parent.width
                    spacing: 8

                    Text {
                        text: qsTr("Mapa creado por: %1").arg(details.create_by_name || "")
                        font.pixelSize: 16
                        width: parent.width
                        elide: Text.ElideRight
                    }

                    Text {
                        text: qsTr("Fecha de creación: %1").arg(details.create_day || "")
                        font.pixelSize: 16
                        width: parent.width
                        elide: Text.ElideRight
                    }

                    Text {
                        text: qsTr("Nombre del mapa: %1").arg(details.map_name || mapName)
                        font.pixelSize: 16
                        width: parent.width
                        elide: Text.ElideRight
                    }

                    Text {
                        text: qsTr("Localización del mapa: %1").arg(details.location || "")
                        font.pixelSize: 16
                        width: parent.width
                        elide: Text.ElideRight
                    }

                    Text {
                        text: qsTr("Descripción: %1").arg(details.details || "")
                        font.pixelSize: 16
                        width: parent.width
                        wrapMode: Text.WordWrap
                    }
                }

                Rectangle {
                    id: previewBox
                    width: parent.width
                    height: Math.max(180, detailsCard.height - infoColumn.height - 32)
                    color: AppTheme.selection.previewBackground
                    radius: 8
                    border.color: AppTheme.selection.previewBorder
                    border.width: 1

                    Image {
                        anchors.centerIn: parent
                        source: previewPath !== "" ? previewPath : "qrc:/qmlresources/icons/color/map.svg"
                        width: parent.width * 0.9
                        height: parent.height * 0.9
                        opacity: previewPath !== "" ? 1.0 : 0.4
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }
        }

        Rectangle {
            id: detailsSpacer
            height: 10
            width: detailsCard.width
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: detailsCard.bottom
            color: "transparent"
        }

        Item {
            id: footer
            height: Math.max(76, root.buttonHeightPx + 20)
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 10

            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10

                Button {
                    id: acceptButton
                    width: 180
                    height: root.buttonHeightPx
                    padding: 0

                    background: Rectangle {
                        color: acceptButton.down ? AppTheme.selection.primary : AppTheme.selection.white
                        radius: 8
                        border.color: AppTheme.selection.primaryDark
                        border.width: 2
                        Behavior on color { ColorAnimation { duration: 100 } }
                    }

                    contentItem: Label {
                        text: qsTr("ACEPTAR")
                        color: AppTheme.selection.primaryDark
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        root.close()
                        root.acceptedSelection(root.mapName)
                    }
                }

                Rectangle {
                    width: acceptButton.width
                    height: 10
                    color: "transparent"
                }
            }
        }
    }
}
