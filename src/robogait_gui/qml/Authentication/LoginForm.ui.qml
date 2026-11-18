import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import AppTheme 1.0

import "qrc:/Components"

Page {
    id: root

    property alias usernameField: usernameField
    property alias passwordField: passwordField
    property alias loginButton: loginButton
    property alias authModeDropdown: authModeDropdown
    property real keyboardSafeArea: 0
    property int contentCenterOffset: 0
    property real formContentHeight: formColumn.implicitHeight
    readonly property bool editingActive: usernameField.activeFocus || passwordField.activeFocus
    readonly property bool compactInputsMode: editingActive || root.keyboardSafeArea > 0 || Qt.inputMethod.visible
    readonly property bool hasVerticalScroll: flick.contentHeight > flick.height + 1
    readonly property real dropdownHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.dropdownHeight, 0) : 50
    readonly property real inputHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.inputHeight, 0) : 50
    readonly property real buttonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 50

    Rectangle {
        anchors.fill: parent
        color: AppTheme.core.appBackground

        Flickable {
            id: flick
            anchors.fill: parent
            anchors.topMargin: 20
            anchors.bottomMargin: 30 + root.keyboardSafeArea
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            interactive: root.hasVerticalScroll
            contentWidth: width
            contentHeight: content.implicitHeight

            onContentHeightChanged: {
                if (!root.hasVerticalScroll) {
                    contentY = 0
                }
            }
            onHeightChanged: {
                if (!root.hasVerticalScroll) {
                    contentY = 0
                }
            }

            Item {
                id: content
                width: flick.width
                implicitHeight: Math.max(formColumn.implicitHeight, flick.height)

                Column {
                    id: formColumn
                    width: Math.min(400, Math.max(0, content.width - 40))
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: root.contentCenterOffset
                                                  + (root.compactInputsMode
                                                     ? (Math.abs(root.contentCenterOffset) < 1 ? 0 : 80)
                                                     : 0)

                    Item {
                        width: formColumn.width
                        height: root.compactInputsMode
                                ? Math.max(90, logo.height * 0.62)
                                : logo.height

                        Image {
                            id: logo
                            anchors.centerIn: parent
                            width: root.compactInputsMode
                                   ? Math.min(140, formColumn.width * 0.42)
                                   : Math.min(220, formColumn.width)
                            height: width
                            fillMode: Image.PreserveAspectFit
                            source: "qrc:/qmlresources/logos/robogait_logo.png"
                        }
                    }

                    AuthModeDropdown {
                        id: authModeDropdown
                        width: formColumn.width
                        height: root.dropdownHeightPx
                        controlHeight: root.dropdownHeightPx
                        currentMode: "login"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    TextField {
                        id: usernameField
                        width: formColumn.width
                        height: root.inputHeightPx
                        font.pointSize: 20
                        color: AppTheme.core.dark
                        placeholderTextColor: AppTheme.auth.placeholderText
                        placeholderText: qsTr("Usuario")
                        background: Rectangle {
                            radius: 10
                            color: AppTheme.core.white
                        }
                    }

                    TextField {
                        id: passwordField
                        width: formColumn.width
                        height: root.inputHeightPx
                        font.pointSize: 20
                        color: AppTheme.core.dark
                        placeholderTextColor: AppTheme.auth.placeholderText
                        echoMode: TextInput.Password
                        placeholderText: qsTr("Contraseña")
                        background: Rectangle {
                            radius: 10
                            color: AppTheme.core.white
                        }
                    }

                    Button {
                        id: loginButton
                        width: formColumn.width
                        height: root.buttonHeightPx
                        text: qsTr("Iniciar sesión")
                        font.capitalization: Font.AllUppercase
                        font.bold: true
                        font.pointSize: 22

                        contentItem: Text {
                            text: loginButton.text
                            font: loginButton.font
                            color: AppTheme.core.white
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            radius: 10
                            color: AppTheme.auth.buttonBackground
                        }
                    }
                }
            }
        }

        CustomScrollTrack {
            flickable: flick
            formColumn: formColumn
            startItem: authModeDropdown
            trackVisible: root.hasVerticalScroll
        }

    }
}
