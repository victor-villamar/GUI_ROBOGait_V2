import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0

import "qrc:/Dialogs"

Item {
    id: root

    property alias selectMapArea: selectMapArea
    property alias selectPatientArea: selectPatientArea
    property alias manualControlArea: manualControlArea
    property alias testArea: testArea

    property alias infoButton: infoButton
    property alias infoDialog: infoDialog

    readonly property bool isGuest: (dbManager && dbManager.userRole === "guest")

	    Rectangle {
	        anchors.fill: parent
	        color: "#518bb7"

	        Item {
	            id: topLogoBar
	            height: 190
	            anchors.left: parent.left
	            anchors.right: parent.right
	            anchors.top: parent.top
			            anchors.leftMargin: -10
			            anchors.rightMargin: 10
		            anchors.topMargin: -100
	
		            RowLayout {
		                anchors.fill: parent
		
		                Item {
		                    Layout.preferredWidth: 110
		                    Layout.preferredHeight: topLogoBar.height
		
		                    Image {
		                        id: upm
		                        opacity: 0.7
		                        width: 110
		                        height: 90
		                        anchors.bottom: parent.bottom
		                        anchors.horizontalCenter: parent.horizontalCenter
		                        source: "qrc:/qmlresources/logos/upm.png"
		                        fillMode: Image.PreserveAspectFit
		                        smooth: true
		                    }
		                }
		
		                Item {
		                    Layout.preferredWidth: 110
		                    Layout.preferredHeight: topLogoBar.height
		
		                    Image {
		                        id: car
		                        opacity: 0.7
		                        width: 110
		                        height: 90
		                        anchors.bottom: parent.bottom
		                        anchors.horizontalCenter: parent.horizontalCenter
		                        source: "qrc:/qmlresources/logos/car.png"
		                        fillMode: Image.PreserveAspectFit
		                        smooth: true
		                    }
		                }

		                Item {
		                    Layout.fillWidth: true
		                }
		
		                Item {
		                    Layout.preferredWidth: 110
		                    Layout.preferredHeight: topLogoBar.height
		
		                    Image {
		                        id: etsidi
		                        opacity: 0.7
		                        width: 110
		                        height: 90
		                        anchors.bottom: parent.bottom
		                        anchors.horizontalCenter: parent.horizontalCenter
		                        source: "qrc:/qmlresources/logos/etsidi.png"
		                        fillMode: Image.PreserveAspectFit
		                        smooth: true
		                    }
		                }
		            }
		        }

	        Grid {
	            id: menuGrid
	            anchors.horizontalCenter: parent.horizontalCenter
	            anchors.verticalCenter: parent.verticalCenter
	            anchors.verticalCenterOffset: -40
            columns: 2
            rowSpacing: 40
            columnSpacing: 60

            Rectangle {
                width: 170
                height: 170
                radius: 12
                color: "transparent"

                Image {
                    anchors.fill: parent
                    source: "qrc:/qmlresources/menu/select_map.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: root.isGuest ? 0.45 : 1.0
                }

                MouseArea {
                    id: selectMapArea
                    anchors.fill: parent
                    enabled: !root.isGuest
                }
            }

            Rectangle {
                width: 170
                height: 170
                radius: 12
                color: "transparent"

                Image {
                    anchors.fill: parent
                    source: "qrc:/qmlresources/menu/select_patient.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: root.isGuest ? 0.45 : 1.0
                }

                MouseArea {
                    id: selectPatientArea
                    anchors.fill: parent
                    enabled: !root.isGuest
                }
            }

            Rectangle {
                width: 170
                height: 170
                radius: 12
                color: "transparent"

                Image {
                    anchors.fill: parent
                    source: "qrc:/qmlresources/menu/manual_control.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }

                MouseArea {
                    id: manualControlArea
                    anchors.fill: parent
                }
            }

            Rectangle {
                width: 170
                height: 170
                radius: 12
                color: "transparent"

                Image {
                    anchors.fill: parent
                    source: "qrc:/qmlresources/menu/image_test.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: root.isGuest ? 0.45 : 1.0
                }

                MouseArea {
                    id: testArea
                    anchors.fill: parent
                    enabled: !root.isGuest
	                }
	            }
	        }

	        Image {
	            id: ministerio
	            opacity: 0.7
            width: 360
            height: 130
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: -20
            anchors.bottomMargin: -20
            source: "qrc:/qmlresources/logos/ministerio.png"
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Text {
            id: explain
            width: Math.min(900, parent.width * 0.9)
            color: "#ffffff"
            text: qsTr("Bienvenido a la aplicación de ROBOGait. Para comenzar, seleccione un mapa y un paciente. Puede acceder al control manual desde esta pantalla. Para más detalles, presione el botón de información.")
            wrapMode: Text.Wrap
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 86
        }

        Button {
            id: infoButton
            width: 37
            height: 37
            anchors.verticalCenter: explain.verticalCenter
            anchors.left: explain.right
            anchors.leftMargin: 10

            background: Rectangle {
                radius: width / 2
                color: "transparent"
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/circle_info_solid.svg"
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: infoDialog.openWithMessage(infoDialog.message)
        }

        Text {
            id: bottomStatus
            color: "#cc0000"
            font.pixelSize: 14
            font.bold: true
            text: qsTr("NO HAY MAPA ACTIVO  –  NO HAY PACIENTE ACTIVO")
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 12
        }

        ConfirmationDialog {
            id: infoDialog
            holdToAccept: false
            acceptText: qsTr("Cerrar")
            message: qsTr("Objetivo de la aplicación:\n"
                        + "Controlar el robot ROBOGait y realizar pruebas de la marcha.\n\n"
                        + "Cómo empezar:\n"
                        + "- Seleccionar mapa\n"
                        + "- Seleccionar paciente\n"
                        + "- Control manual\n"
                        + "- Prueba\n\n"
                        + "La selección del mapa y el paciente es obligatoria antes de iniciar una prueba.")
        }
    }
}
