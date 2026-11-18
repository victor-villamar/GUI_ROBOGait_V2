import QtQuick 2.15
import AppTheme 1.0

CustomToolBarForm {
    readonly property real backButtonSizePx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.iconButtonSize, 0) : 32
    readonly property real backIconSizePx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.iconGlyphSize, 0) : 24

    backButton.width: backButtonSizePx
    backButton.height: backButtonSizePx
    backButton.icon.width: backIconSizePx
    backButton.icon.height: backIconSizePx

    backButton.onClicked: applicationFlow.backButton()
    backButton.visible: applicationFlow && applicationFlow.state === "main_menu" && applicationFlow.mystackview.depth > 3
    backButton.enabled: backButton.visible
    backButton.opacity: backButton.visible ? 1 : 0
    backButton.states: State {
        name: "pressed"
        when: backButton.pressed
        PropertyChanges {
            target: backButton
            scale: 1.1
        }
    }
}
