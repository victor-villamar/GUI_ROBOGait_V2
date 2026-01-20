import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property alias no_save: no_save
    property alias save: save
    property alias cancel: cancel
    property alias text_name: text_name
    property alias text_header: text_header
    color: "white"
    border.color: "#3A64A5"
    border.width: 2
    radius: 20
    width: 540
    height: 210
    Text {
        id: text_header
        //        width: 492
        //        height: 34
        text: qsTr("¿Guardar los cambios en este archivo?")
        anchors.left: parent.left
        anchors.top: parent.top
        font.pixelSize: 30
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        anchors.topMargin: 18
        anchors.leftMargin: 24
        font.styleName: "Light Italic"
    }

    // Text {
    //     id: text1
    //     x: 309
    //     width: 142
    //     height: 22
    //     text: qsTr("Nombre del archivo")
    //     anchors.left: parent.left
    //     anchors.top: text_header.bottom
    //     font.pixelSize: 16
    //     anchors.leftMargin: 36
    //     anchors.topMargin: 10
    // }
    Rectangle {
        id: rectangle1
        border.color: "black"
        color: parent.color
        border.width: 2
        anchors.left: parent.left
        anchors.top: text_header.bottom
        anchors.leftMargin: 26
        anchors.topMargin: 35
        radius: 4
        Text {
            id: text_name
            color: "#454545"
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            text: (ddbb.mapNameTemporal)
            font.pixelSize: 20
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            anchors.leftMargin: 20
        }
    }

    Button {
        id: cancel
        text: qsTr("Cancelar")
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 28
        anchors.bottomMargin: 28

        background: Rectangle {
            color: "#ffffff" // Color inicial
            radius: 6
            border.color: "#454545" // Borde del mismo color que el texto
            border.width: 2
        }

        contentItem: Label {
            text: parent.text
            color: "#454545"
            font.pixelSize: 18
        }
    }

   Button {
       id: no_save
       text: qsTr("No Guardar")
       anchors.right: cancel.left
       anchors.bottom: parent.bottom
       anchors.bottomMargin: 28
       anchors.rightMargin: 10

       background: Rectangle {
           color: "#ffffff" // Color inicial
           radius: 6
           border.color: "#454545" // Borde del mismo color que el texto
           border.width: 2
       }

       contentItem: Label {
           text: parent.text
           color: "#454545"
           font.pixelSize: 18
       }
   }

    Button {
        id: save
        text: qsTr("Guardar")
        anchors.right: no_save.left
        anchors.bottom: parent.bottom
        // t: save.enabled ? "white" : "#454545";
        anchors.bottomMargin: 28
        anchors.rightMargin: 10
        enabled: text_name.text.length > 0
        background: Rectangle {
            color: save.enabled ? "#3A64A5" : "#B0B0B0"
            radius: 6
        }
        contentItem: Label {
            text: parent.text
            color: save.enabled ? "white" : "#454545" // Cambia el color del texto
            font.pixelSize: 18
        }
    }
}

/*##^##
Designer {
    D{i:0;height:210}D{i:1}D{i:3}D{i:2}D{i:4}D{i:7}
}
##^##*/

