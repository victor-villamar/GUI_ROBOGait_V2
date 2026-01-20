import QtQuick 2.15
import QtQuick.Controls 2.15
import "extras"

Rectangle {
    id: rectangle
    color: "#518bb7"
    property alias buttonAccept: buttonAccept
    property alias listView: listView
    property alias buttonRow: buttonRow
    property alias buttonCancel: buttonCancel
    property alias descriptionMap: descriptionMap
    property alias addPatient: addPatient

    // Título
    Text {
        id: text1
        text: qsTr("SELECCIONE UN MAPA")
        color: "#ffffff"
        font.pixelSize: 30
        horizontalAlignment: Text.AlignHCenter
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 20
    }

    Button {
        id: addPatient
        width: scrollView.width
        height: 60
        anchors.left: parent.left
        anchors.top: text1.bottom
        anchors.leftMargin: 20
        anchors.topMargin: 30
        background: Rectangle {
            color: "#ffffff"
            border.color: "#cccccc"
            radius: 15
        }

        Text {
            id: textMapName
            text: qsTr("Crear nuevo mapa")
            font.pixelSize: 22
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#000000"
            anchors.verticalCenter: parent.verticalCenter
        }

        Button {
            id: mapDelete
            background: Image {
                source: "../images/icon_app/plus-solid.svg"
                sourceSize.width: 25
                sourceSize.height: 25
                fillMode: Image.PreserveAspectFit
            }

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 20
        }
    }

    // ScrollView que contiene la lista, centrado en la pantalla
    ScrollView {
        id: scrollView
        width: parent.width * 0.5
        anchors.left: parent.left
        anchors.top: addPatient.bottom
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.topMargin: 10
        anchors.leftMargin: 20

        ListView {
            id: listView
            currentIndex: -1
            anchors.fill: parent
            clip: true
            model: ddbb.maps

            // model: ListModel {
            //     ListElement {
            //         display: "Paciente 1"
            //     }
            //     ListElement {
            //         display: "Paciente 2"
            //     }
            //     ListElement {
            //         display: "Paciente 3"
            //     }
            //     ListElement {
            //         display: "Paciente 4"
            //     }
            //     ListElement {
            //         display: "Paciente 5"
            //     }
            //     ListElement {
            //         display: "Paciente 6"
            //     }
            //     ListElement {
            //         display: "Paciente 7"
            //     }
            // }
            delegate: Item {
                width: listView.width
                height: 70

                Button {
                    id: mapButton
                    width: parent.width
                    height: 60
                    background: Rectangle {
                        color: index === listView.currentIndex ? "#aed2ea" : "#ffffff"
                        border.color: "#cccccc"
                        radius: 15
                    }

                    onClicked: {
                        listView.currentIndex = index
                        ddbb.getMapInformation(model.display)
                        mapInfo.setMapName(model.display);
                        descriptionMap.visible = true
                        buttonRow.visible = true
                        // applicationFlow.menu_push()
                    }
                    Text {
                        id: textAddPatient
                        text: display
                        font.pixelSize: 22
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: "#000000"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    // // Botón de Editar
                    // Button {
                    //     id: mapEdit
                    //     text: "Editar"
                    //     anchors.verticalCenter: parent.verticalCenter
                    //     anchors.right: mapDelete.left
                    //     anchors.rightMargin: 20
                    //     //                                                        onClicked: console.log("Editar:", model.name)
                    // }

                    // Botón de Borrar
                    Button {
                        id: deletePatient
                        background: Image {
                            source: "../images/icon_app/trash-solid.svg"
                            sourceSize.width: 25
                            sourceSize.height: 25
                            fillMode: Image.PreserveAspectFit
                        }

                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 20
                    }
                }
            }
        }
    }
    DescriptionMapForm {
        id: descriptionMap
        visible: false
        anchors.left: scrollView.right
        anchors.right: parent.right
        anchors.top: text1.bottom
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 140
        anchors.topMargin: 30
        anchors.rightMargin: 40
        anchors.leftMargin: 40
    }

    Row {
        id: buttonRow
        visible: false
        spacing: parent.width > 500 ? 40 : 20
        anchors.horizontalCenter: descriptionMap.horizontalCenter
        anchors.top: descriptionMap.bottom
        anchors.topMargin: 20
        anchors.bottomMargin: 40

        Button {
            id: buttonCancel
            width: 140
            height: 40

            background: Rectangle {
                color: buttonCancel.down ? "#e9e9e9" : "#aed2ea"
                radius: 15
                border.color: "#e9e9e9"
                border.width: 2
            }

            contentItem: Label {
                id: buttonLabelCancel
                text: qsTr("CANCELAR")
                color: "#518bb7"
                font.pixelSize: 20
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            // onClicked: {
            //     listView.currentIndex = -1
            //     descriptionPatient.visible = false
            //     buttonRow.visible = false
            // }
        }

        Button {
            id: buttonAccept
            width: 140
            height: 40

            background: Rectangle {
                color: buttonAccept.down ? "#e9e9e9" : "#aed2ea"
                radius: 15
                border.color: "#e9e9e9"
                border.width: 2
            }

            contentItem: Label {
                id: buttonLabelAccept
                text: qsTr("ACEPTAR")
                color: "#518bb7"
                font.pixelSize: 20
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
} // import QtQuick 2.15// import QtQuick.Controls 2.15
// import "extras"

// Rectangle {
//     id: rectangle
//     // width: 1300
//     // height: 700
//     color: "#518bb7"
//     // property alias mapButton: mapButton
//     // // property alias mapEdit: mapEdit
//     // // property alias mapDelete: mapDelete
//     // property alias listView: listView

//     // Título
//     Text {
//         id: text1
//         text: qsTr("SELECCIONE UN MAPA")
//         color: "#ffffff"
//         font.pixelSize: 30
//         horizontalAlignment: Text.AlignHCenter
//         anchors.top: parent.top
//         anchors.horizontalCenter: parent.horizontalCenter
//         anchors.topMargin: 1
//     }

//     // ScrollView que contiene la lista
//     ScrollView {
//         id: scrollView
//         anchors {
//             left: parent.left
//             right: parent.right
//             top: parent.top
//             bottom: parent.bottom
//             margins: 60
//         }

//         // ListView para mostrar los elementos
//         ListView {
//             id: listView
//             anchors {
//                 left: parent.left
//                 right: parent.right
//                 top: parent.top
//                 bottom: parent.bottom
//                 rightMargin: 10
//             }
//             clip: true
//             model: stringHandler.model

//             //            model: ListModel {
//             //                ListElement {
//             //                    name: "MAP1"
//             //                }
//             //            }
//             delegate: Item {
//                 width: listView.width
//                 height: 70

//                 Button {
//                     id: mapButton
//                     width: parent.width
//                     height: 60
//                     background: Rectangle {
//                         color: "#ffffff"
//                         border.color: "#cccccc"
//                         radius: 10
//                     }
//                     onClicked: {
//                         mapInfo.setMapName(model.display); // Mostrará el texto asociado al botón
//                         applicationFlow.map_path_push()
//                     }

//                     // Acción al hacer clic en el Button (Rectangle)
//                     //                                        onClicked: {
//                     //                                            console.log("Seleccionado:", model.name)
//                     //                                        }
//                     // Row {
//                     //     id: row
//                     //     // spacing: 20
//                     //     anchors.fill: parent
//                     //     // anchors.margins: 10
//                     //     // Texto del nombre del elemento
//                     //     // Component.onCompleted: {
//                     //     //             console.log("modelData value:", modelData)
//                     //     //         }
//                     Text {
//                         id: textMapName
//                         text: display
//                         font.pixelSize: 18
//                         anchors.horizontalCenter: parent.horizontalCenter
//                         color: "#000000"
//                         anchors.verticalCenter: parent.verticalCenter
//                     }

//                     // Botón de Editar
//                     // Button {
//                     //     id: mapEdit
//                     //     text: "Editar"
//                     //     anchors.verticalCenter: parent.verticalCenter
//                     //     anchors.right: mapDelete.left
//                     //     anchors.rightMargin: 20
//                     //     //                                                        onClicked: console.log("Editar:", model.name)
//                     // }

//                     // Botón de Borrar
//                     Button {
//                         id: mapDelete
//                         background: Image {
//                             source: "../images/icon_app/trash-solid.svg"
//                             sourceSize.width: 25
//                             sourceSize.height: 25
//                             fillMode: Image.PreserveAspectFit
//                         }
//                         anchors.verticalCenter: parent.verticalCenter
//                         anchors.right: parent.right
//                         anchors.rightMargin: 20
//                         //                                                        onClicked: listView.model.remove(index)
//                     }
//                     // }
//                 }
//             }
//         }
//     }
// }

