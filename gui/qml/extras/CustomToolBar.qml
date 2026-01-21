import QtQuick 2.15

CustomToolBarForm {
    backButton.onClicked: applicationFlow.backButton()
    backButton.states: State {
        name: "pressed"
        when: backButton.pressed
        PropertyChanges {
            target: backButton
            scale: 1.1
        }
    }
}

