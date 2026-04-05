import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Item {
    id: root
    
    readonly property real iconButtonPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.iconButtonSize, 0) : 56
    readonly property real iconGlyphPx: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 32

    width: Math.round(iconButtonPx)
    height: Math.round(iconButtonPx)
    
    property alias settingsButton: settingsButton
    property bool enabled: true

    signal settingsClicked()

    Button {
        id: settingsButton
        anchors.fill: parent
        
        enabled: root.enabled
        
        icon.source: "qrc:/qmlresources/icons/config.svg"
        icon.width: iconGlyphPx
        icon.height: iconGlyphPx
        icon.color: "#ffffff"
        palette.buttonText: "#ffffff"
        
        background: Rectangle {
            color: settingsButton.pressed ? "#a9cfe8" : settingsButton.hovered ? "#a9cfe8" : "transparent"
            radius: Math.round(root.height / 2)

            Behavior on color {
                ColorAnimation {
                    duration: 150
                    easing.type: Easing.InOutQuad
                }
            }
        }

        onClicked: {
            root.settingsClicked()
        }
        
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Ajustes de desarrollador")
        ToolTip.delay: 500
    }
}
