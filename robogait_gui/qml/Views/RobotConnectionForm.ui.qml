import QtQuick 2.15
import QtQuick.Controls 2.15
import RobotDiscovery 1.0

import "qrc:/Dialogs"

Item {
    id: root
    property alias buttonSearchRobot: buttonSearchRobot
    property alias background: background
    property bool showDiscoveryPanel : userSession.rosManager.robotDiscovery.state === RobotDiscovery.SCANNING ||
                                       userSession.rosManager.robotDiscovery.state === RobotDiscovery.ROBOTS_FOUND
    property int selectedRobotIndex: -1
    property int pendingRobotIndex: -1
    property string selectedRobotNamespace: ""


    Rectangle {
        id: background
        color: "#518bb7"
        anchors.fill: parent

        // Title
        Text {
            id: titleText
            color: "#ffffff"
            text: qsTr("CONEXIÓN AL ROBOT")
            font.pixelSize: 32
            font.styleName: "Medium"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: parent.height < 600 ? 10 : 20
        }

        // Main text with scroll
        ScrollView {
            id: scrollView
            width: Math.min(900, parent.width * 0.90)
            height: Math.min(parent.height * 0.6, instructionsColumn.implicitHeight + 8)
            anchors.top: titleText.bottom
            anchors.topMargin: parent.height < 600 ? 20 : 40
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true

            Column {
                id: instructionsColumn
                width: scrollView.width
                spacing: 20

                // Title 1
                Text {
                    text: qsTr("1º ENCENDER EL ORDENADOR DEL ROBOT MÓVIL")
                    color: "#00C8FF"
                    font.pixelSize: 20
                    font.bold: true
                }

                Text {
                    id: step1Text
                    width: scrollView.width
                    color: "#ffffff"
                    font.pixelSize: 17
                    wrapMode: Text.WordWrap
                    text: qsTr("Para encenderlo, sigue estos pasos:\n\n    1.  Conecta el ordenador (NUC) a la batería portátil (power bank).\n    2.  Enciende la batería portátil y ajusta el voltaje hasta que marque 20V.\n    3.  Comprueba que los dos cables de las antenas WiFi estén bien conectadas al ordenador.\n    4.  Enciende el ordenador (NUC).\n    5.  Espera unos segundos hasta que el indicador LED de la conexión WiFi se mantenga en verde fijo (sin parpadear).")
                }

                // Title 2
                Text {
                    text: qsTr("2º CONEXIÓN A LA RED WIFI")
                    color: "#00C8FF"
                    font.pixelSize: 20
                    font.bold: true
                }

                Text {
                    id: step2Text
                    width: scrollView.width
                    color: "#ffffff"
                    font.pixelSize: 17
                    wrapMode: Text.WordWrap
                    text: qsTr("Conéctate a la red WiFi del robot móvil desde la tablet.\nEnciende la tablet, abre la configuración de WiFi y selecciona la red con el nombre robogait2024.")
                }

                // Title 3
                Text {
                    text: qsTr("3º ENCENDER LA BASE DEL ROBOT")
                    color: "#00C8FF"
                    font.pixelSize: 20
                    font.bold: true
                }

                Text {
                    id: step3Text
                    width: scrollView.width
                    color: "#ffffff"
                    font.pixelSize: 17
                    wrapMode: Text.WordWrap
                    text: qsTr("Presiona el botón que se encuentra en la base del robot.\nSabrás que se ha encendido correctamente cuando:\n\n     1.  Se encienda un luz LED azul.\n     2.  Escuches un pitido agudo.")
                }
            }
        }

        Item {
            id: bottomSlot
            width: scrollView.width
            anchors.top: scrollView.bottom
            anchors.topMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height < 600 ? 20 : 40
            anchors.horizontalCenter: parent.horizontalCenter

            // Search button
            Button {
                id: buttonSearchRobot
                height: 52
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                visible: !root.showDiscoveryPanel

                background: Rectangle {
                    color: buttonSearchRobot.down ? "#00C8FF" : "#ffffff"
                    radius: 8
                    border.color: "#045671"
                    border.width: 2

                    // Animation only for color change
                    Behavior on color {
                        ColorAnimation { duration: 100 }  // Very short duration for immediate response
                    }
                }

                contentItem: Label {
                    id: buttonLabel
                    text: qsTr("Buscar Robot")
                    color: "#045671"
                    font.pixelSize: 17
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.fill: parent
                    padding: 0
                }
            }

            Item {
                id: robotsPanel
                width: scrollView.width
                anchors.fill: parent
                anchors.horizontalCenter: parent.horizontalCenter
                visible: root.showDiscoveryPanel
                clip: true

                Column {
                    anchors.fill: parent
                    anchors.topMargin: 8
                    spacing: 8
                    Text {
                            text: qsTr("ROBOTS DETECTADOS")
                            color: "#00C8FF"
                            font.pixelSize: 20
                            font.bold: true
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            visible: userSession.rosManager.robotDiscovery.state === RobotDiscovery.ROBOTS_FOUND
                    }

                    BusyIndicator {
                        id: searchingIndicator
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 180
                        height: 180
                        running: true
                        topPadding: 20
                        visible: userSession.rosManager.robotDiscovery.state === RobotDiscovery.SCANNING
                    }

                    Text {
                        text: qsTr("Buscando robots…")
                        color: "#ffffff"
                        font.pixelSize: 14
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        visible: searchingIndicator.visible
                    }

                    Rectangle {
                        id: robotsListBox
                        width: Math.min(parent.width, Math.max(460, robotsGrid.implicitWidth + 16))
                        height: Math.min(robotsGrid.implicitHeight + 20, parent.height - 60)
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: "transparent"
                        border.color: "#ffffff"
                        border.width: 1
                        radius: 6
                        clip: true
                        visible: userSession.rosManager.robotDiscovery.state === RobotDiscovery.ROBOTS_FOUND

                        Flickable {
                            anchors.fill: parent
                            contentWidth: robotsGrid.implicitWidth
                            contentHeight: robotsGrid.implicitHeight
                            clip: true

                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                            }

                            ScrollBar.horizontal: ScrollBar {
                                policy: ScrollBar.AsNeeded
                            }

                            Grid {
                                id: robotsGrid
                                columns: 2
                                columnSpacing: 12
                                rowSpacing: 10
                                padding: 4
                                property int maxItemWidth: 220

                                function recalcMaxWidth() {
                                    var maxWidth = 220
                                    for (var i = 0; i < robotsRepeater.count; ++i) {
                                        var item = robotsRepeater.itemAt(i)
                                        if (item && item.implicitWidth > maxWidth) {
                                            maxWidth = item.implicitWidth
                                        }
                                    }
                                    maxItemWidth = maxWidth
                                }

                                Repeater {
                                    id: robotsRepeater
                                    model: userSession.rosManager.robotDiscovery.robotsNamespaces
                                    onItemAdded: robotsGrid.recalcMaxWidth()
                                    onItemRemoved: robotsGrid.recalcMaxWidth()
                                    delegate: Rectangle {
                                        id: robotItem
                                        height: 52
                                        radius: 14

                                        property bool selected: root.pendingRobotIndex === index
  
                                        color: selected ? "#ffffff" : "#a9cfe8"
                                        border.color: selected ? "#00C8FF" : "#ffffff"
                                        border.width: 2
                                        implicitWidth: Math.max(220, contentRow.implicitWidth + 30)
                                        width: Math.min(parent.width, robotsGrid.maxItemWidth)

                                        Component.onCompleted: robotsGrid.recalcMaxWidth()
                                        onImplicitWidthChanged: robotsGrid.recalcMaxWidth()

                                        Row {
                                            id: contentRow
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.left: parent.left
                                            anchors.leftMargin: 10
                                            spacing: 25
                                            
                                            Rectangle {
                                                id: robotIcon
                                                width: 70
                                                height: 42
                                                radius: 10
                                                clip: true
                                                color: robotLabel.color
                                                border.color: "#ffffff"
                                                border.width: 1

                                                Image {
                                                    anchors.centerIn: parent
                                                    width: 40
                                                    height: 40
                                                    source: "qrc:/qmlresources/robot/robot.png"
                                                    fillMode: Image.PreserveAspectFit
                                                    smooth: true
                                                }
                                            }


                                            Text {
                                                id: robotLabel
                                                text: modelData
                                                color: "#4f86b4"
                                                font.pixelSize: 16
                                                font.bold: true
                                                elide: Text.ElideRight
                                                verticalAlignment: Text.AlignVCenter
                                                anchors.verticalCenter: robotIcon.verticalCenter
                                            }
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                root.pendingRobotIndex = index
                                                confirmDialog.openWithMessage( 
                                                                              qsTr("Se ha seleccionado %1.\n¿Está seguro que quiere conectarse a este robot?")
                                                                              .arg(modelData))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ConfirmationDialog {
        id: confirmDialog
        anchors.centerIn: parent

        onAccepted: {
            if(root.pendingRobotIndex < 0)
            {
                return
            }

            root.selectedRobotIndex = root.pendingRobotIndex
            root.selectedRobotNamespace = userSession.rosManager.robotDiscovery.robotsNamespaces[root.pendingRobotIndex]
            userSession.assignRobot(root.selectedRobotNamespace)
            root.goToMainMenu()
            root.pendingRobotIndex = -1
        }

        onRejected: {
            root.pendingRobotIndex = -1
            root.selectedRobotIndex = -1
            root.selectedRobotNamespace = ""
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:2}D{i:4}D{i:3}D{i:5}D{i:1}
}
##^##*/
