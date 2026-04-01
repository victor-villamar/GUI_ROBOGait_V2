import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"
import "qrc:/Views"

MainMenuForm {
    id: root
    readonly property real computedIconButtonSizePx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.iconButtonSize, 0) : 37
    readonly property real computedIconGlyphSizePx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.iconGlyphSize, 0) : 37

    iconButtonSizePx: computedIconButtonSizePx
    iconGlyphSizePx: computedIconGlyphSizePx

    ErrorRectangle {
        id: infoPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: ""
    }

    function showNotImplemented(message) {
        infoPopup.errorRectangleTextError.text = message
        infoPopup.open()
    }

    manualControlArea.onClicked: {
        if (StackView.view) {
            StackView.view.push(manualControlPage)
        }
    }

    selectMapArea.onClicked: {
        if (StackView.view) {
            if (userSession.rosManager &&
                userSession.rosManager.robotManager &&
                userSession.rosManager.robotManager.selectedRobotNamespace) {
                StackView.view.push(selectMapPage)
            }
        }
    }

    selectPatientArea.onClicked: {
        if (StackView.view) {
            StackView.view.push(selectPatientPage)
        }
    }

    testArea.onClicked: {
        if (StackView.view) {
            StackView.view.push(testMapViewPage)
        }
    }

    Component {
        id: selectMapPage
        SelectMap { }
    }

    Component {
        id: selectPatientPage
        SelectPatient { }
    }

    Component {
        id: manualControlPage
        ManualControl { }
    }

    Component {
        id: testMapViewPage
        TestMapView { }
    }
}
