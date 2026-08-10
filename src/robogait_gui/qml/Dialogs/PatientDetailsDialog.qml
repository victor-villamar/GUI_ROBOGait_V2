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

    width: parent ? Math.min(720, parent.width * 0.88) : 720
    height: parent ? Math.min(460, parent.height * 0.85) : 460

    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0

    property int patientId: -1
    property string patientDisplay: ""
    property var details: ({})
    readonly property real buttonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 44

    signal acceptedSelection(int patientId, string display)

    function openForPatient(patientIdValue, displayValue, detailsValue)
    {
        patientId = patientIdValue
        patientDisplay = displayValue
        details = detailsValue
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
            height: Math.min(320, parent.height - header.height - footer.height - detailsSpacer.height - 64)
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
                spacing: 10

                Text {
                    text: qsTr("Doctores: %1").arg(details.doctor_names || ((details.user_name || "") + " " + (details.user_last_name || "")).trim())
                    font.pixelSize: 16
                    width: parent.width
                    elide: Text.ElideRight
                }

                Text {
                    text: qsTr("Fecha de creación: %1").arg(details.create_day || "")
                    font.pixelSize: 16
                    width: parent.width
                    wrapMode: Text.NoWrap
                    elide: Text.ElideRight
                }

                Text {
                    text: qsTr("Nombre completo: %1 %2").arg(details.first_name || "").arg(details.last_name || "")
                    font.pixelSize: 16
                    width: parent.width
                    elide: Text.ElideRight
                }

                Text {
                    text: qsTr("Edad: %1 años").arg(details.age || "")
                    font.pixelSize: 16
                }

                Text {
                    text: qsTr("Peso: %1 Kg").arg(details.weight || "")
                    font.pixelSize: 16
                }

                Text {
                    text: qsTr("Altura: %1 cm").arg(details.height || "")
                    font.pixelSize: 16
                }

                Text {
                    text: qsTr("Descripción: %1").arg(details.description || "")
                    font.pixelSize: 16
                    width: parent.width
                    wrapMode: Text.WordWrap
                }
            }
        }

        Rectangle {
            id: detailsSpacer
            height: 12
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
                        root.acceptedSelection(root.patientId, root.patientDisplay)
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
