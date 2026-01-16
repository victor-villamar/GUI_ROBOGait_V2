import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: imageText
    width: 120
    height: 120

    property alias text_label: text_label
    property alias button_id: button_id
    property string image_source

    Column {
        id: column
        spacing: 5 // Espacio entre el botón y el texto, puedes ajustarlo

        Button {
            id: button_id
            width: 70
            height: 70
            anchors.horizontalCenter: parent.horizontalCenter
            background: Image {
                source: imageText.image_source
                fillMode: Image.PreserveAspectFit
            }
            Behavior on scale {
                NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
            }

            // Change scale when pressed/released
            onPressed: scale = 1.2
            onReleased: scale = 1.0
        }

        Text {
            id: text_label
            text: qsTr("Texto debajo")
            font.pixelSize: 16
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WordWrap  // Esto permite que el texto se divida en varias líneas
        }

    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:1.66}D{i:2}D{i:4}D{i:1}
}
##^##*/
