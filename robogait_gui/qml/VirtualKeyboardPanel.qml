import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.VirtualKeyboard 2.15
import QtQuick.VirtualKeyboard.Settings 2.15

Item {
    id: root
    parent: Overlay.overlay
    z: 1000
    clip: true

    property real keyboardScale: 0.6
    readonly property bool keyboardVisible: Qt.inputMethod.visible
    readonly property real windowWidth: {
        var win = Qt.application.activeWindow
        return win ? win.width : (parent ? parent.width : 0)
    }
    readonly property real windowHeight: {
        var win = Qt.application.activeWindow
        return win ? win.height : (parent ? parent.height : 0)
    }
    readonly property real fallbackHeight: windowHeight * 0.4
    readonly property real fallbackWidth: windowWidth
    readonly property real widthScale: keyboardScale
    readonly property real heightScale: keyboardScale
    readonly property real baseWidth: inputPanel.implicitWidth > 0 ? inputPanel.implicitWidth : fallbackWidth
    readonly property real baseHeight: inputPanel.implicitHeight > 0 ? inputPanel.implicitHeight : fallbackHeight
    readonly property real reservedHeight: keyboardVisible ? (baseHeight * heightScale) : 0

    width: keyboardVisible ? (baseWidth * widthScale) : 0
    height: reservedHeight
    x: Math.round((windowWidth - width) / 2)
    y: Math.round(windowHeight - height)

    Component.onCompleted: {
        VirtualKeyboardSettings.locale = "es_ES"
    }

    InputPanel {
        id: inputPanel
        width: root.baseWidth
        height: root.baseHeight
        visible: root.keyboardVisible

        transform: Scale {
            origin.x: 0
            origin.y: 0
            xScale: root.widthScale
            yScale: root.heightScale
        }
    }
}
