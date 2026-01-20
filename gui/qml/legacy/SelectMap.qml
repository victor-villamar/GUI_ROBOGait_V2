import QtQuick 2.15
import "extras"

SelectMapForm {
    addPatient.onClicked: {
        listView.currentIndex = -1
        descriptionMap.visible = false
        buttonRow.visible = false
        applicationFlow.register_map_push()
    }
    buttonAccept.onClicked: {
        stringHandler.mapNameTest = ddbb.mapDescription.map_name
        applicationFlow.popFunction()
    }
    buttonCancel.onClicked: {
        listView.currentIndex = -1
        descriptionMap.visible = false
        buttonRow.visible = false
    }

}
