import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0
import QtQuick.Window 2.15
import AppTheme 1.0

Item {
    id: root

    enabled: userSession && userSession.role === "manager" && userSession.isAuthenticated
    readonly property real spinboxButtonPx: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.spinboxButtonSize, 0) : 20
    readonly property real spinboxButtonMargin: Math.max(2, Math.round(spinboxButtonPx * 0.2))
    readonly property real spinboxBaseHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.inputHeight, 0) : 70
    readonly property real spinboxMinHeightPx: Math.round(spinboxButtonPx * 2 + spinboxButtonMargin * 2 + 8)
    readonly property real spinboxHeightPx: Math.max(spinboxBaseHeightPx, spinboxMinHeightPx)
    readonly property real spinboxNumberFontPx: Math.max(16, Math.round(spinboxHeightPx * 0.4))

    implicitWidth: contentLayout.childrenRect.width + 40
    implicitHeight: contentLayout.childrenRect.height + 40

    function applyChanges() {
        if (!userSession || !userSession.isAuthenticated) {
            return false
        }

        if (userSession.role === "manager") {
            return true
        }
        else {
            return false
        }
    }

    function resetChanges() {
        developerSettings.resetChanges()
    }

    Column {
        id: contentLayout
        x: 20
        y: 20
        width: parent.width - 40
        height: implicitHeight
        spacing: 15

        Text {
            text: qsTr("Configuracion del Ros Domain Id")
            font.pixelSize: 18
            font.bold: true
            color: root.enabled ? AppTheme.settings.primaryDark : AppTheme.settings.sectionTitleDisabled
            width: parent.width
        }

        Rectangle {
            height: 2
            color: AppTheme.settings.white
            width: parent.width
        }

        GroupBox {
            id: domainGroup
            title: qsTr("Ros Domain Id")
            label: Text {
                text: domainGroup.title
                font: domainGroup.font
                color: AppTheme.settings.black
            }

            width: parent.width

            contentItem: Column {
                spacing: 10
                width: parent.width

                Text {
                    text: qsTr("Selecciona el dominio de ROS")
                    font.pixelSize: 12
                    color: AppTheme.settings.groupDescription
                    width: parent.width
                    wrapMode: Text.WordWrap
                }

                SpinBox {
                    id: domainSpinBox
                    from: 0
                    to: 232
                    value: developerSettings ? developerSettings.rosDomainId : 0
                    editable: true
                    enabled: root.enabled
                    width: 160
                    height: root.spinboxHeightPx
                    palette.base: AppTheme.settings.white
                    palette.text: AppTheme.settings.primaryDark
                    palette.button: AppTheme.settings.lightBlue
                    palette.buttonText: AppTheme.settings.primaryDark

                    onValueChanged: {
                        if (!userSession || !userSession.isAuthenticated) {
                            return
                        }

                        if (userSession.role === "manager" && developerSettings) {
                            developerSettings.rosDomainId = value
                        }
                    }

                    contentItem: TextInput {
                        text: domainSpinBox.textFromValue(domainSpinBox.value, domainSpinBox.locale)
                        font.pixelSize: root.spinboxNumberFontPx
                        font.family: domainSpinBox.font.family
                        font.bold: domainSpinBox.font.bold
                        color: AppTheme.settings.primaryDark
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        readOnly: !domainSpinBox.editable
                        validator: IntValidator { bottom: domainSpinBox.from; top: domainSpinBox.to }
                        anchors.fill: parent
                        anchors.rightMargin: root.spinboxButtonPx + root.spinboxButtonMargin * 2
                    }

                    up.indicator: Rectangle {
                        implicitWidth: root.spinboxButtonPx
                        implicitHeight: root.spinboxButtonPx
                        color: AppTheme.settings.lightBlue
                        border.color: AppTheme.settings.primaryDark
                        border.width: 1
                        radius: Math.round(root.spinboxButtonPx * 0.2)
                        anchors.right: parent.right
                        anchors.rightMargin: root.spinboxButtonMargin
                        anchors.top: parent.top
                        anchors.topMargin: root.spinboxButtonMargin
                        Text {
                            anchors.centerIn: parent
                            text: "▲"
                            color: AppTheme.settings.primaryDark
                            font.pixelSize: Math.round(root.spinboxButtonPx * 0.6)
                        }
                    }

                    down.indicator: Rectangle {
                        implicitWidth: root.spinboxButtonPx
                        implicitHeight: root.spinboxButtonPx
                        color: AppTheme.settings.lightBlue
                        border.color: AppTheme.settings.primaryDark
                        border.width: 1
                        radius: Math.round(root.spinboxButtonPx * 0.2)
                        anchors.right: parent.right
                        anchors.rightMargin: root.spinboxButtonMargin
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: root.spinboxButtonMargin
                        Text {
                            anchors.centerIn: parent
                            text: "▼"
                            color: AppTheme.settings.primaryDark
                            font.pixelSize: Math.round(root.spinboxButtonPx * 0.6)
                        }
                    }

                    background: Rectangle {
                        implicitWidth: 160
                        implicitHeight: root.spinboxHeightPx
                        radius: 6
                        color: AppTheme.settings.white
                        border.color: AppTheme.settings.primaryDark
                        border.width: 1
                    }
                }

                Text {
                    text: qsTr("El Domain Id define el dominio DDS para el descubrimiento")
                    font.pixelSize: 11
                    color: AppTheme.settings.statusOk
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.bold: true
                }
            }
        }
    }
}
