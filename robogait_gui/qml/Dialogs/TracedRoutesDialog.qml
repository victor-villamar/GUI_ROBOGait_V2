import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import AppTheme 1.0

Dialog {
    id: root
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose

    parent: Overlay.overlay

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: AppTheme.dialogsCommon.overlayDim
    }

    property var routesModel: []
    property int selectedRouteId: -1
    readonly property real nameColumnRatio: 0.70
    readonly property real typeColumnRatio: 0.30

    signal routeSelected(var route)

    width: parent ? Math.min(900, parent.width * 0.9) : 900
    height: parent ? Math.min(560, parent.height * 0.82) : 560
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    function routeTypeLabel(route)
    {
        if (!route || !route.type) {
            return ""
        }

        return route.type === "goal" ? qsTr("Objetivo") : qsTr("Ruta")
    }

    function reposition()
    {
        if (!parent) {
            return
        }

        x = Math.round((parent.width - width) / 2)
        y = Math.round((parent.height - height) / 2)
    }

    function openWithRoutes(routes, selectedId)
    {
        routesModel = routes || []
        selectedRouteId = selectedId !== undefined ? selectedId : -1
        reposition()
        open()
    }

    onOpened: reposition()
    onWidthChanged: reposition()
    onHeightChanged: reposition()
    onParentChanged: reposition()

    background: Rectangle {
        color: AppTheme.dialogsCommon.panelBackground
        border.color: AppTheme.dialogsCommon.light
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
                    color: closeButton.down ? AppTheme.dialogsCommon.closeButtonPressed : AppTheme.dialogsCommon.light
                    radius: 8
                    border.color: AppTheme.dialogsCommon.primary
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

            Text {
                anchors.centerIn: parent
                text: qsTr("Rutas trazadas")
                color: AppTheme.dialogsCommon.light
                font.pixelSize: 20
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Rectangle {
            id: tableContainer
            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 16
            radius: 10
            color: AppTheme.dialogsCommon.light
            clip: true
            border.color: AppTheme.dialogsCommon.primary
            border.width: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    color: AppTheme.dialogsCommon.primary

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12

                        Text {
                            width: parent.width * root.nameColumnRatio
                            height: parent.height
                            text: qsTr("Nombre")
                            color: AppTheme.dialogsCommon.light
                            font.pixelSize: 16
                            font.bold: true
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 2
                        }

                        Text {
                            width: parent.width * root.typeColumnRatio
                            height: parent.height
                            text: qsTr("Tipo")
                            color: AppTheme.dialogsCommon.light
                            font.pixelSize: 16
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                ListView {
                    id: routesList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: root.routesModel

                    delegate: Rectangle {
                        id: routeDelegate
                        readonly property bool isSelectedRoute: modelData.id === root.selectedRouteId

                        width: routesList.width
                        height: 48
                        color: isSelectedRoute ? AppTheme.dialogsCommon.closeButtonPressed
                                               : ((index % 2 === 0) ? AppTheme.dialogsCommon.panelBackground : AppTheme.dialogsCommon.light)
                        border.color: isSelectedRoute ? AppTheme.dialogsCommon.primary : "transparent"
                        border.width: isSelectedRoute ? 2 : 0

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12

                            Text {
                                width: parent.width * root.nameColumnRatio
                                height: parent.height
                                text: modelData.name || ""
                                color: AppTheme.dialogsCommon.black
                                font.pixelSize: 15
                                font.bold: routeDelegate.isSelectedRoute
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 2
                            }

                            Text {
                                width: parent.width * root.typeColumnRatio
                                height: parent.height
                                text: root.routeTypeLabel(modelData)
                                color: AppTheme.dialogsCommon.black
                                font.pixelSize: 15
                                font.bold: routeDelegate.isSelectedRoute
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                root.selectedRouteId = modelData.id
                                root.routeSelected(modelData)
                                root.close()
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        visible: routesList.count === 0
                        text: qsTr("No hay rutas trazadas")
                        color: AppTheme.dialogsCommon.primary
                        font.pixelSize: 16
                        font.bold: true
                    }
                }
            }
        }
    }
}
