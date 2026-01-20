import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: descriptionMapForm
    color: "#e9e9e9"
    radius: 15
    border.color: "#235c87"
    border.width: 2

    property var mapData: {
        "first_name": "Desconocido",
        "last_name": "Desconocido",
        "create_day": "No disponible",
        "map_name": "No disponible",
        "location": "No disponible",
        "details": "No disponible",
        "map": "No disponible"
    } // Default values for MapDescription data
    property var imageSource

    Column {
        anchors.fill: parent
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 15
        spacing: 10

        Text {
            id: createByPersonText
            text: qsTr("Mapa creado por: ") + (mapData.create_by_name || "")
            font.pixelSize: 16
        }
        Text {
            id: creationDateText
            text: qsTr("Fecha de creación: ") + (mapData.create_day || "")
            wrapMode: Text.Wrap
            width: parent.width - 40
            font.pixelSize: 16
        }
        Text {
            id: mapNameText
            text: qsTr("Nombre del mapa: ") + (mapData.map_name || "")
            font.pixelSize: 16
        }
        Text {
            id: mapLocationText
            text: qsTr("Localización del mapa: ") + (mapData.location || "")
            font.pixelSize: 16
        }
        Text {
            id: descriptionText
            text: qsTr("Descripción: ") + (mapData.details || "")
            font.pixelSize: 16
            wrapMode: Text.Wrap
            width: parent.width - 40
        }
        Image {
            id: image
            width: parent.width
            height: parent.height - mapNameText.height - descriptionText.height - mapLocationText.height - createByPersonText.height -creationDateText.height - 60
            source: descriptionMapForm.imageSource
            fillMode: Image.PreserveAspectFit
        }
    }

    Connections {
        target: ddbb // Replace with the actual object that emits the signal
        function onMapDescriptionChanged() {
            descriptionMapForm.mapData = ddbb.mapDescription
        }
    }
    Connections {
        target: mapInfo // Replace with the actual object that emits the signal
        function onImgSourceChanged() {
            descriptionMapForm.imageSource =  mapInfo.imgSource
        }
    }
}
