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

    readonly property real badgeHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.badgeHeight, 0) : 56
    readonly property real badgeMenuItemHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.badgeMenuItemHeight, 0) : 44
    readonly property real iconGlyphPxRaw: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 34
    readonly property real badgeIconPx: Math.min(iconGlyphPxRaw, badgeHeightPx - 16)

    property int badgeWidth: Math.max(patientRow.implicitWidth + 20, 140)
    property int segmentHeight: Math.round(badgeMenuItemHeightPx)
    property int segmentPadding: 16
    property int dividerSize: 2
    property int segmentWidth: Math.max(120, diagnosticsText.implicitWidth + root.segmentPadding * 2)
    property int menuWidth: Math.max(root.badgeWidth, root.segmentWidth)

    implicitHeight: Math.round(badgeHeightPx)
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
        height: root.implicitHeight
        width: root.badgeWidth
        radius: Math.round(height * 0.32)
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
                Layout.preferredWidth: root.badgeIconPx
                Layout.preferredHeight: root.badgeIconPx
                Layout.minimumWidth: root.badgeIconPx
                Layout.minimumHeight: root.badgeIconPx
                Layout.maximumWidth: root.badgeIconPx
                Layout.maximumHeight: root.badgeIconPx
                Layout.leftMargin: 1
                Layout.alignment: Qt.AlignVCenter

                Image {
                    anchors.fill: parent
                    source: "qrc:/qmlresources/icons/white/patient.svg"
                    sourceSize.width: root.badgeIconPx
                    sourceSize.height: root.badgeIconPx
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
                Layout.preferredWidth: root.badgeIconPx
                Layout.preferredHeight: root.badgeIconPx
                Layout.minimumWidth: root.badgeIconPx
                Layout.minimumHeight: root.badgeIconPx
                Layout.maximumWidth: root.badgeIconPx
                Layout.maximumHeight: root.badgeIconPx
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
