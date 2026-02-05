import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0

import "qrc:/Dialogs"

Item {
    id: root

    property bool dropDownOpen: false
    property bool patientAssigned: userSession ? userSession.hasPatientAssigned : false
    property string patientLabel: (userSession && userSession.currentPatient && userSession.currentPatient.displayName && userSession.currentPatient.displayName.length)
                                  ? userSession.currentPatient.displayName
                                  : qsTr("Paciente")

    property int badgeWidth: Math.max(patientRow.implicitWidth + 20, 140)
    property int segmentHeight: 44
    property int segmentPadding: 16
    property int dividerSize: 2
    property int segmentWidth: Math.max(120, diagnosticsText.implicitWidth + root.segmentPadding * 2)
    property int menuWidth: Math.max(root.badgeWidth, root.segmentWidth)

    implicitHeight: 56
    implicitWidth: badgeWidth

    visible: patientAssigned

    Timer {
        id: autoCloseTimer
        interval: 3000
        repeat: false
        onTriggered: root.dropDownOpen = false
    }

    function restartAutoCloseTimer()
    {
        autoCloseTimer.stop()
        if (root.dropDownOpen) {
            autoCloseTimer.start()
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.dropDownOpen
        onClicked: root.dropDownOpen = false
        z: 1
    }

    Rectangle {
        id: patientLabelRect
        z: 3
        height: 56
        width: root.badgeWidth
        radius: 18
        color: "#a9cfe8"
        border.color: "#ffffff"
        border.width: 2

        anchors.top: parent.top
        anchors.right: parent.right

        RowLayout {
            id: patientRow
            anchors.fill: parent
            anchors.margins: 10
            spacing: 12

            Item {
                Layout.preferredWidth: 34
                Layout.preferredHeight: 34
                Layout.minimumWidth: 34
                Layout.minimumHeight: 34
                Layout.maximumWidth: 34
                Layout.maximumHeight: 34
                Layout.leftMargin: 1
                Layout.alignment: Qt.AlignVCenter

                Image {
                    anchors.fill: parent
                    source: "qrc:/qmlresources/icons/patient.svg"
                    sourceSize.width: 40
                    sourceSize.height: 40
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }

            Text {
                text: root.patientLabel
                color: "#045671"
                font.pixelSize: 18
                font.bold: true
                elide: Text.ElideRight
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Item {
                Layout.preferredWidth: 34
                Layout.preferredHeight: 34
                Layout.minimumWidth: 34
                Layout.minimumHeight: 34
                Layout.maximumWidth: 34
                Layout.maximumHeight: 34
                Layout.alignment: Qt.AlignVCenter
                visible: false
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.dropDownOpen = !root.dropDownOpen
                root.restartAutoCloseTimer()
            }
        }

        HoverHandler {
            id: patientHover
        }

        ToolTip.visible: patientHover.hovered
        ToolTip.text: qsTr("Paciente")
        ToolTip.delay: 500
    }

    Rectangle {
        id: dropDown
        z: 3
        width: root.menuWidth
        height: root.dropDownOpen ? root.segmentHeight : 0
        opacity: root.dropDownOpen ? 1 : 0
        radius: 14
        color: "#ffffff"
        border.color: "#045671"
        border.width: 2
        clip: true

        anchors.top: patientLabelRect.bottom
        anchors.right: patientLabelRect.right
        anchors.topMargin: 6

        Behavior on height {
            NumberAnimation {
                duration: 150
                easing.type: Easing.InOutQuad
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 120 }
        }

        Column {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                id: diagnosticsSegment
                width: parent.width
                height: root.segmentHeight
                color: diagnosticsArea.pressed ? "#00C8FF" : "transparent"

                Text {
                    id: diagnosticsText
                    anchors.centerIn: parent
                    text: qsTr("Diagnosticos")
                    color: "#045671"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: diagnosticsArea
                    anchors.fill: parent
                    onPressed: root.restartAutoCloseTimer()
                    onClicked: {
                        root.dropDownOpen = false
                        diagnosticDialog.open()
                    }
                }
            }
        }
    }

    DiagnosticDialog {
        id: diagnosticDialog
    }
}
