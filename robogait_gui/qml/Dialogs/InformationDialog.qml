import QtQuick 2.15
import QtQuick.Controls 2.15

Dialog {
    id: root
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose

    parent: Overlay.overlay

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: "#A0505050"
    }

    property string message: ""

    readonly property int outerPadding: 24
    readonly property int maxHeight: parent ? Math.round(parent.height * 0.92) : 720
    readonly property int contentDesiredHeight: outerPadding * 2
                                             + headerColumn.implicitHeight
                                             + 16
                                             + bodyText.implicitHeight

    width: parent ? Math.min(900, parent.width * 0.92) : 900
    height: Math.min(maxHeight, Math.max(520, contentDesiredHeight))
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    function reposition()
    {
        if (!parent) {
            return
        }

        x = Math.round((parent.width - width) / 2)
        y = Math.round((parent.height - height) / 2)
    }

    function openWithMessage(messageText)
    {
        message = messageText
        reposition()
        open()
    }

    onOpened: reposition()
    onWidthChanged: reposition()
    onHeightChanged: reposition()
    onParentChanged: reposition()

    background: Rectangle {
        color: "#ffffff"
        border.color: "#235c87"
        border.width: 4
        radius: 15
    }

    contentItem: Item {
        anchors.fill: parent

        Button {
            id: closeButton
            width: 28
            height: 28
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: root.outerPadding
            anchors.rightMargin: root.outerPadding
            padding: 0

            background: Rectangle {
                radius: 6
                color: closeButton.down ? "#e8f0f6" : "#ffffff"
                border.color: "#235c87"
                border.width: 2
            }

            contentItem: Image {
                anchors.centerIn: parent
                width: 14
                height: 14
                source: "qrc:/qmlresources/icons/black/xmark_solid.svg"
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            onClicked: root.close()
        }

        Column {
            id: headerColumn
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: root.outerPadding
            anchors.leftMargin: root.outerPadding
            anchors.rightMargin: root.outerPadding
            spacing: 8

            Item {
                width: parent.width
                height: closeButton.height
            }

            Image {
                width: 40
                height: 40
                anchors.horizontalCenter: parent.horizontalCenter
                source: "qrc:/qmlresources/icons/color/circle_question.svg"
                fillMode: Image.PreserveAspectFit
                smooth: true
                opacity: 0.85
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Información")
                color: "#1e4d70"
                font.pixelSize: 28
                font.bold: true
            }
        }

        Text {
            id: bodyText
            anchors.top: headerColumn.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: 16
            anchors.leftMargin: root.outerPadding
            anchors.rightMargin: root.outerPadding
            anchors.bottomMargin: root.outerPadding
            width: parent.width - root.outerPadding * 2
            text: root.message
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            color: "#1e4d70"
            font.pixelSize: 15
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }

    Component.onCompleted: reposition()
}
