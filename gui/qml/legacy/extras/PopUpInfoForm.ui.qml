import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: informationPopUp
    width: 800
    height: 450
    radius: 15
    color: "white"
    border.color: "#235c87"
    border.width: 4
    property alias xmark: xmark
    property alias textInfo: textInfo
    clip: true

    // Botón de cerrar
    Button {
        id: xmark
        width: 20
        height: 20
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.rightMargin: 10
        background: Image {
            source: "../../images/icon_app/xmark-solid.svg"
            fillMode: Image.PreserveAspectFit
        }
        onClicked: {
            informationPopUp.visible = false
            informationPopUp.enabled = false
        }
        Behavior on scale {
            NumberAnimation {
                duration: 150
                easing.type: Easing.OutQuad
            }
        }
        onPressed: scale = 1.2
        onReleased: scale = 1.0
    }

    // Imagen
    Image {
        id: mainImage
        width: 40
        height: 40
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../../images/icon_app/circle-question.svg"
        fillMode: Image.PreserveAspectCrop
        smooth: true
    }

    // Título
    Text {
        width: parent.width
        height: 34
        text: "Información"
        anchors.top: mainImage.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#1e4d70"
        font.pixelSize: 25
        horizontalAlignment: Text.AlignHCenter
        font.bold: true
    }

    // Contenido con scroll (solo para el texto largo)
    ScrollView {
        id: scrollView
        width: parent.width - 40
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: mainImage.bottom
        anchors.topMargin: 60
        anchors.bottom: parent.bottom
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        clip: true

        Text {
            id: textInfo
            wrapMode: Text.Wrap
            anchors.rightMargin: 90
            anchors.leftMargin: 20
            anchors.bottomMargin: 20
            anchors.topMargin: 20
            textFormat: Text.RichText

            text: "FFFFFFFFFFFFF"
            anchors.fill: parent
            color: "#1e4d70"
            font.pixelSize: 14
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.66}D{i:1}D{i:3}D{i:4}D{i:6}D{i:5}
}
##^##*/

