import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: home
    property alias buttonStart: buttonStart

    Rectangle {
        id: rectangle
        color: "#518bb7"
        anchors.top: home.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        Image {
            id: image
            width: parent.height < 500 ? 150 : 197
            height: width
            anchors.verticalCenter: parent.verticalCenter
            source: "qrc:/qmlresources/logos/robogait_logo.png"
            anchors.horizontalCenterOffset: 0
            anchors.verticalCenterOffset: -110
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
        }

        Button {
            id: buttonStart
            height: parent.height < 500 ? 80 : 100
            anchors.top: image.bottom
            anchors.topMargin: parent.height < 500 ? 20 : 40
            anchors.horizontalCenter: parent.horizontalCenter
            width: contentItem.implicitWidth + (parent.height < 500 ? 60 : 80)

            background: Rectangle {
                color: buttonStart.down ? "#e9e9e9" : "#aed2ea"
                radius: 15
                border.color: "#e9e9e9"
                border.width: 2
            }

            contentItem: Label {
                id: buttonLabel
                text: qsTr("START")
                color: "#518bb7"
                font.pixelSize: rectangle.height < 500 ? 40 : 60
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.styleName: "Regular"
                font.weight: Font.Light
                anchors.fill: parent
                padding: 0
            }
        }

	        Image {
	            id: upm
	            width: 110
	            height: 90
	            anchors.left: parent.left
	            anchors.top: parent.top
	            source: "qrc:/qmlresources/logos/upm.png"
	            anchors.topMargin: -50
	            anchors.leftMargin: -10
	            fillMode: Image.PreserveAspectFit
	        }
	        Image {
	            id: car
	            width: 110
	            height: 90
	            anchors.left: upm.right
	            anchors.top: parent.top
	            source: "qrc:/qmlresources/logos/car.png"
	            anchors.topMargin: -50
	            anchors.leftMargin: 0

	            fillMode: Image.PreserveAspectFit
	        }
	        Image {
	            id: etsidi
	            width: 110
	            height: 90
	            anchors.right: parent.right
	            anchors.top: parent.top
	            source: "qrc:/qmlresources/logos/etsidi.png"
	            anchors.rightMargin: 20
	            anchors.topMargin: -60

	            fillMode: Image.PreserveAspectFit
	        }
        Image {
            id: ministerio
            width: 360
            height: 130
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            source: "qrc:/qmlresources/logos/ministerio.png"
            anchors.leftMargin: -20
            anchors.bottomMargin: -20
            fillMode: Image.PreserveAspectFit
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.5;height:700;width:1300}D{i:2}D{i:3}D{i:6}D{i:7}
D{i:8}D{i:9}D{i:1}
}
##^##*/
