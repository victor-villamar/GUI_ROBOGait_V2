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
    height: parent ? Math.min(520, parent.height * 0.8) : 520

    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0

    property int selectedIndex: -1
    property var doctorsModel: (userSession && userSession.currentPatient)
                               ? userSession.currentPatient.doctorDiagnostics
                               : []
    readonly property real listItemHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.listItemHeight, 0) : 44

    function selectedDoctor()
    {
        if (selectedIndex >= 0 && selectedIndex < doctorsModel.length) {
            return doctorsModel[selectedIndex]
        }
        return null
    }

    onOpened: {
        selectedIndex = -1
        doctorsList.currentIndex = -1
    }

    onDoctorsModelChanged: {
        selectedIndex = -1
        doctorsList.currentIndex = -1
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
        }

        RowLayout {
            id: body
            anchors.top: header.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.topMargin: 16
            anchors.bottomMargin: 16
            spacing: 16

            Rectangle {
                id: doctorsPanel
                Layout.preferredWidth: Math.max(220, parent.width * 0.3)
                Layout.fillHeight: true
                radius: 12
                color: AppTheme.dialogsCommon.primary

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Text {
                        text: qsTr("Doctores")
                        color: AppTheme.dialogsCommon.light
                        font.pixelSize: 16
                        font.bold: true
                    }

                    ListView {
                        id: doctorsList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 8
                        model: root.doctorsModel

                        delegate: Rectangle {
                            id: doctorCard
                            width: doctorsList.width
                            height: root.listItemHeightPx
                            radius: 10
                            color: ListView.isCurrentItem ? AppTheme.dialogsCommon.closeButtonPressed : AppTheme.dialogsCommon.panelBackground
                            border.color: AppTheme.dialogsCommon.light
                            border.width: 2

                            Text {
                                anchors.centerIn: parent
                                text: (modelData.display || ((modelData.doctor_last_name || "") + ", " + (modelData.doctor_name || "")).trim())
                                color: ListView.isCurrentItem ? AppTheme.dialogsCommon.light : AppTheme.dialogsCommon.primary
                                font.pixelSize: 14
                                font.bold: true
                                elide: Text.ElideRight
                                horizontalAlignment: Text.AlignHCenter
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.selectedIndex = index
                                    doctorsList.currentIndex = index
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: detailsPanel
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 12
                color: AppTheme.dialogsCommon.diagnosticDetailsBackground
                border.color: AppTheme.dialogsCommon.panelAccentBorder
                border.width: 2

                Item {
                    anchors.fill: parent
                    anchors.margins: 16

                    Column {
                        id: detailsColumn
                        anchors.fill: parent
                        spacing: 12
                        visible: root.selectedDoctor() !== null

                        Text {
                            text: qsTr("Fecha: %1").arg(root.selectedDoctor() ? (root.selectedDoctor().create_day || "") : "")
                            font.pixelSize: 16
                            color: AppTheme.dialogsCommon.primary
                            wrapMode: Text.NoWrap
                            elide: Text.ElideRight
                        }

                        Text {
                            text: qsTr("Descripcion: %1").arg(root.selectedDoctor() ? (root.selectedDoctor().description || "") : "")
                            font.pixelSize: 16
                            color: AppTheme.dialogsCommon.primary
                            wrapMode: Text.WordWrap
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        visible: root.selectedDoctor() === null
                        text: qsTr("Seleccione doctor para comprobar el diagnostico del paciente")
                        color: AppTheme.dialogsCommon.primary
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        width: parent.width * 0.8
                    }
                }
            }
        }

    }
}
