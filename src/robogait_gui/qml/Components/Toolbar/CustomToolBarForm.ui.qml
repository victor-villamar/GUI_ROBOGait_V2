import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0
import AppTheme 1.0

Item {
    id: root
    property alias backButton: backButton
    property alias logo: logo
    property alias title: title
    property alias robotBadge: robotBadge
    property alias patientBadge: patientBadge
    property alias userBadge: userBadge
    property alias settingsBadge: settingsBadge
    property bool showRobotBadge: false
    property bool showUserBadge: false

    Rectangle {
        anchors.fill: parent
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10

            Button {
                id: backButton
                Layout.alignment: Qt.AlignVCenter
                icon.source: "qrc:/qmlresources/icons/white/keyboard_left.svg"
                icon.color: "white"
                icon.width: 24
                icon.height: 24
                background: null
                transitions: Transition {
                    NumberAnimation {
                        properties: "scale"
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 0
            }

            Row {
                id: rightBadges
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                spacing: 10

                SettingsBadge {
                    id: settingsBadge
                }

                RobotBadge {
                    id: robotBadge
                    visible: root.showRobotBadge && robotConnected
                    enabled: visible
                }

                PatientBadge {
                    id: patientBadge
                    visible: userSession ? userSession.hasPatientAssigned : false
                    enabled: visible
                }

                UserBadge {
                    id: userBadge
                    visible: root.showUserBadge
                    enabled: visible
                }
            }
        }

        Row {
            id: brandRow
            readonly property real leftLimit: backButton.visible
                                                ? backButton.x + backButton.width + 10
                                                : 10
            readonly property real rightLimit: rightBadges.x - 10
            readonly property real availableWidth: Math.max(0, rightLimit - leftLimit)
            readonly property real fullBrandWidth: logo.implicitWidth + spacing + title.implicitWidth

            visible: availableWidth >= logo.implicitWidth
            x: Math.round(Math.max(leftLimit,
                                  Math.min((parent.width - implicitWidth) / 2,
                                           rightLimit - implicitWidth)))
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10

            Image {
                id: logo
                source: "qrc:/qmlresources/logos/robogait_logo.png"
                sourceSize.height: 25
                sourceSize.width: height * 1.4
            }

            Text {
                id: title
                visible: brandRow.availableWidth >= brandRow.fullBrandWidth
                text: "ROBOGAIT"
                font.pixelSize: 20
                color: "white"
                font.bold: true
            }
        }
    }
}
