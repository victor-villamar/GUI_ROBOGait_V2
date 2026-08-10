import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import AppTheme 1.0

Item {
    id: root

    property string currentMode: "login"
    property real controlHeight: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.dropdownHeight, 0) : 50
    property real popupItemHeight: controlHeight
    property int fontPixelSize: 26
    property color textColor: AppTheme.auth.light
    property bool open: popup.opened
    property bool ignoreNextClick: false

    signal modeSelected(string mode)

    implicitWidth: labelRow.implicitWidth
    implicitHeight: Math.max(controlHeight, Math.max(labelText.implicitHeight, 34))

    function labelForMode(mode) {
        switch (mode) {
        case "login": return qsTr("Login")
        case "sign_in": return qsTr("Registrarse")
        case "guest": return qsTr("Invitado")
        default: return mode
        }
    }

    function orderedModes() {
        var all = ["login", "sign_in", "guest"]
        var out = [root.currentMode]
        for (var i = 0; i < all.length; ++i) {
            if (all[i] !== root.currentMode) {
                out.push(all[i])
            }
        }
        return out
    }

    function openPopup() {
        if (!popup.parent) {
            return
        }
        repositionPopup()
        popup.open()
    }

    function repositionPopup() {
        if (!popup.parent) {
            return
        }

        var p = root.mapToItem(popup.parent, 0, root.height)
        popup.x = Math.round(p.x + (root.width - popup.width) / 2)
        popup.y = Math.round(p.y + 6)
    }

    function closePopup() {
        popup.close()
    }

    onVisibleChanged: {
        if (!visible && popup.opened) {
            closePopup()
        }
    }

    Component.onDestruction: {
        if (popup.opened) {
            popup.close()
        }
    }

    Timer {
        id: autoCloseTimer
        interval: timeoutSettings ? timeoutSettings.authModeDropdownAutoCloseMs : 4000
        repeat: false
        onTriggered: root.closePopup()
    }

    function restartAutoCloseTimer() {
        autoCloseTimer.stop()
        if (popup.opened) {
            autoCloseTimer.start()
        }
    }

    function togglePopup() {
        if (popup.opened) {
            closePopup()
        } else {
            openPopup()
        }
    }

    Row {
        id: labelRow
        anchors.centerIn: parent
        height: root.controlHeight
        spacing: 10

        Text {
            id: labelText
            text: root.labelForMode(root.currentMode)
            color: root.textColor
            font.pixelSize: root.fontPixelSize
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }

        Image {
            id: arrowImage
            width: 20
            height: 20
            source: root.open
                  ? "qrc:/qmlresources/icons/white/arrow_up.svg"
                  : "qrc:/qmlresources/icons/white/arrow_down.svg"
            fillMode: Image.PreserveAspectFit
            smooth: true
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        anchors.fill: parent
        onPressed: function(mouse) {
            root.restartAutoCloseTimer()

            if (popup.opened) {
                root.closePopup()
                root.ignoreNextClick = true
                mouse.accepted = true
            }
        }
        onClicked: {
            if (root.ignoreNextClick) {
                root.ignoreNextClick = false
                return
            }

            root.togglePopup()
            root.restartAutoCloseTimer()
        }
    }

    Popup {
        id: popup
        modal: false
        focus: false
        closePolicy: Popup.CloseOnEscape
        parent: Overlay.overlay
        padding: 0

        width: Math.min(320, root.width > 0 ? root.width : 320)
        height: contentColumn.implicitHeight

        background: Rectangle {
            color: AppTheme.auth.light
            radius: 12
            border.color: AppTheme.auth.dropdownPrimary
            border.width: 2
        }

        onOpened: root.restartAutoCloseTimer()
        onClosed: autoCloseTimer.stop()


        contentItem: Column {
            id: contentColumn
            spacing: 0

            Repeater {
                model: root.orderedModes()

                delegate: Rectangle {
                    property var dropdownRoot: root
                    property string modeValue: modelData
                    width: popup.width
                    height: root.popupItemHeight
                    color: itemArea.pressed ? AppTheme.auth.dropdownPressedBackground : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: parent.dropdownRoot.labelForMode(parent.modeValue)
                        color: AppTheme.auth.dropdownPrimary
                        font.pixelSize: 18
                        font.bold: true
                    }

                    MouseArea {
                        id: itemArea
                        anchors.fill: parent
                        onPressed: dropdownRoot.restartAutoCloseTimer()
                        onClicked: {
                            var keyboard_was_visible = Qt.inputMethod.visible
                            var focused_item = root.Window.window ? root.Window.window.activeFocusItem : null
                            parent.dropdownRoot.currentMode = parent.modeValue
                            parent.dropdownRoot.closePopup()
                            parent.dropdownRoot.modeSelected(parent.modeValue)
                            if (keyboard_was_visible) {
                                Qt.callLater(function() {
                                    if (focused_item && focused_item.forceActiveFocus) {
                                        focused_item.forceActiveFocus()
                                    }
                                    Qt.inputMethod.show()
                                })
                            }
                        }
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 1
                        color: AppTheme.auth.dropdownPrimary
                        opacity: 0.20
                        visible: index !== (parent.dropdownRoot.orderedModes().length - 1)
                    }
                }
            }
        }
    }

}
