import QtQuick 2.15

CustomToolBarForm {
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
