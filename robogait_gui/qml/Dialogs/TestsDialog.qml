import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0
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

    width: parent ? Math.min(900, parent.width * 0.9) : 900
    height: parent ? Math.min(560, parent.height * 0.82) : 560

    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0

    property var testsModel: (userSession && userSession.currentPatient)
                             ? userSession.currentPatient.tests
                             : []
    property var selectedTest: null
    readonly property real infoColumnWidth: 36
    readonly property real dateColumnRatio: 0.60
    readonly property real locationColumnRatio: 0.40

    function refreshTestsModel()
    {
        if (!dbManager || !userSession || !userSession.currentPatient || !userSession.currentPatient.isActive) {
            testsModel = []
            return
        }

        testsModel = dbManager.getPatientTests(userSession.currentPatient.id)
    }

    onOpened: refreshTestsModel()

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
                text: qsTr("Pruebas del paciente")
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
                            width: Math.max(120, (parent.width - root.infoColumnWidth) * root.dateColumnRatio)
                            height: parent.height
                            text: qsTr("Fecha")
                            color: AppTheme.dialogsCommon.light
                            font.pixelSize: 16
                            font.bold: true
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 2
                        }

                        Text {
                            width: Math.max(90, (parent.width - root.infoColumnWidth) * root.locationColumnRatio)
                            height: parent.height
                            text: qsTr("Localizacion")
                            color: AppTheme.dialogsCommon.light
                            font.pixelSize: 16
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item {
                            width: root.infoColumnWidth
                            height: parent.height
                        }
                    }
                }

                ListView {
                    id: testsList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: root.testsModel

                    delegate: Rectangle {
                        width: testsList.width
                        height: 46
                        color: (index % 2 === 0) ? AppTheme.dialogsCommon.panelBackground : AppTheme.dialogsCommon.light

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12

                            Text {
                                width: Math.max(120, (parent.width - root.infoColumnWidth) * root.dateColumnRatio)
                                height: parent.height
                                text: modelData.date || ""
                                color: AppTheme.dialogsCommon.black
                                font.pixelSize: 15
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 2
                            }

                            Text {
                                width: Math.max(90, (parent.width - root.infoColumnWidth) * root.locationColumnRatio)
                                height: parent.height
                                text: modelData.map_name || ""
                                color: AppTheme.dialogsCommon.black
                                font.pixelSize: 15
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }

                            Button {
                                width: root.infoColumnWidth
                                height: parent.height
                                padding: 0

                                background: Rectangle {
                                    color: "transparent"
                                }

                                contentItem: Image {
                                    source: "qrc:/qmlresources/icons/black/circle_info_solid.svg"
                                    fillMode: Image.PreserveAspectFit
                                    sourceSize.width: 18
                                    sourceSize.height: 18
                                }

                                onClicked: {
                                    root.selectedTest = modelData
                                    testDetailsDialog.open()
                                }
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        visible: testsList.count === 0
                        text: qsTr("No hay pruebas disponibles para el paciente")
                        color: AppTheme.dialogsCommon.primary
                        font.pixelSize: 16
                    }
                }
            }
        }
    }

    Dialog {
        id: testDetailsDialog
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose
        parent: Overlay.overlay

        width: root.parent ? Math.min(640, root.parent.width * 0.84) : 640
        height: root.parent ? Math.min(380, root.parent.height * 0.72) : 380
        x: root.parent ? (root.parent.width - width) / 2 : 0
        y: root.parent ? (root.parent.height - height) / 2 : 0

        Overlay.modal: Rectangle {
            anchors.fill: parent
            color: AppTheme.dialogsCommon.overlayDim
        }

        background: Rectangle {
            color: AppTheme.dialogsCommon.panelBackground
            border.color: AppTheme.dialogsCommon.light
            border.width: 2
            radius: 12
        }

        contentItem: Item {
            anchors.fill: parent

            Item {
                id: detailsHeader
                height: 32
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: 16
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                Button {
                    id: closeDetailsButton
                    width: 26
                    height: 26
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    padding: 0

                    background: Rectangle {
                        color: closeDetailsButton.down ? AppTheme.dialogsCommon.closeButtonPressed : AppTheme.dialogsCommon.light
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

                    onClicked: testDetailsDialog.close()
                }
            }

            Rectangle {
                anchors.top: detailsHeader.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 16
                color: AppTheme.dialogsCommon.light
                radius: 10
                border.color: AppTheme.dialogsCommon.primary
                border.width: 2

                Column {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 10

                    Text {
                        text: qsTr("Paciente: %1 %2")
                              .arg(root.selectedTest ? (root.selectedTest.patient_name || "") : "")
                              .arg(root.selectedTest ? (root.selectedTest.patient_last_name || "") : "")
                        color: AppTheme.dialogsCommon.black
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        text: qsTr("Doctor: %1 %2")
                              .arg(root.selectedTest ? (root.selectedTest.doctor_name || "") : "")
                              .arg(root.selectedTest ? (root.selectedTest.doctor_last_name || "") : "")
                        color: AppTheme.dialogsCommon.black
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        text: qsTr("Fecha: %1").arg(root.selectedTest ? (root.selectedTest.date || "") : "")
                        color: AppTheme.dialogsCommon.black
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        text: qsTr("Localizacion: %1").arg(root.selectedTest ? (root.selectedTest.location || "") : "")
                        color: AppTheme.dialogsCommon.black
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        text: qsTr("Comentario: %1").arg(root.selectedTest ? (root.selectedTest.comment || "") : "")
                        color: AppTheme.dialogsCommon.black
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }
}
