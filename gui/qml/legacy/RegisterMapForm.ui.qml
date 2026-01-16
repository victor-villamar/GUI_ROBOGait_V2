import QtQuick 2.15
import QtQuick.Controls 2.15

Page {
    id: signinPage
    property alias rm_add_map: addInformation
    property alias rm_cancel_test: cancelRegisterPatient
    property alias mapNameField: mapNameField
    property alias descriptionField: descriptionField
    property alias locationField: locationField

    property int keyboardHeight: Qt.inputMethod.visible ? (parent.height - Qt.inputMethod.keyboardRectangle.height + 20) : 0

    Rectangle {
        id: rectangle
        color: "#518bb7"
        anchors.fill: parent

        ScrollView {
            id: formScroll
            width: 560
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: 20
            anchors.bottomMargin: 70
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true
            // ScrollBar.vertical.policy: ScrollBar.AsNeeded
            bottomPadding: keyboardHeight > 0 ? keyboardHeight : 0

            //            Behavior on bottomPadding {
            //                NumberAnimation {
            //                    duration: 250
            //                }
            //            }
            Column {
                id: column
                width: 450
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: -40
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 15
                padding: 50

                Text {
                    id: text1
                    color: "#ffffff"
                    text: qsTr("DATOS DEL MAPA")
                    font.pixelSize: 40
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.styleName: "Medium"
                    padding: 10
                }

                TextField {
                    id: mapNameField
                    width: 450
                    height: 40
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 15
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    rightPadding: 10

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: "#CCCCCC"

                        Text {
                            text: qsTr("Nombre del mapa")
                            color: "#808080"
                            anchors {
                                right: parent.right
                                rightMargin: 15
                                verticalCenter: parent.verticalCenter
                            }
                            font: mapNameField.font
                        }
                    }
                    Keys.onTabPressed: locationField.focus = true
                    Keys.onReturnPressed: locationField.focus = true
                }

                TextField {
                    id: locationField
                    width: 450
                    height: 40
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 15
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    rightPadding: 10

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: "#CCCCCC"

                        Text {
                            text: qsTr("Localización del mapa")
                            color: "#808080"
                            anchors {
                                right: parent.right
                                rightMargin: 15
                                verticalCenter: parent.verticalCenter
                            }
                            font: locationField.font
                        }
                    }
                    Keys.onTabPressed: descriptionField.focus = true
                    Keys.onReturnPressed: descriptionField.focus = true
                }

                TextField {
                    id: descriptionField
                    width: 450
                    height: 150
                    color: "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 15
                    topPadding: 15
                    leftPadding: 15
                    rightPadding: 15
                    bottomPadding: 15
                    verticalAlignment: TextInput.AlignTop
                    horizontalAlignment: TextInput.AlignLeft
                    wrapMode: TextField.Wrap
                    selectByMouse: true

                    background: Rectangle {
                        radius: 10
                        color: "#FFFFFF"
                        border.color: descriptionField.activeFocus ? "#518bb7" : "#CCCCCC"

                        Text {
                            visible: descriptionField.text === ""
                                     && !descriptionField.activeFocus
                            text: qsTr("Descripción")
                            color: "#808080"
                            anchors {
                                left: parent.left
                                leftMargin: 15
                                top: parent.top
                                topMargin: 15
                            }
                            font: descriptionField.font
                        }
                    }
                    Keys.onTabPressed: addInformation.focus = true
                    Keys.onReturnPressed: addInformation.focus = true
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20

                    Button {
                        id: addInformation
                        width: 215
                        height: 40
                        text: qsTr("HACER MAPA")
                        font.styleName: "Medium"
                        font.capitalization: Font.AllUppercase
                        font.weight: Font.Light
                        font.bold: true
                        font.pointSize: 19
                        background: Rectangle {
                            radius: 10
                            color: "#aed2ea"
                            border.color: "#aed2ea"
                        }
                    }

                    Button {
                        id: cancelRegisterPatient
                        width: 215
                        height: 40
                        text: qsTr("CANCELAR")
                        font.styleName: "Medium"
                        font.capitalization: Font.AllUppercase
                        font.weight: Font.Normal
                        font.bold: true
                        font.pointSize: 19
                        background: Rectangle {
                            radius: 10
                            color: "#aed2ea"
                            border.color: "#aed2ea"
                        }
                    }
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.66;height:700;width:1300}D{i:4}D{i:5}D{i:8}D{i:12}
D{i:14}D{i:11}D{i:3}D{i:2}D{i:1}
}
##^##*/

