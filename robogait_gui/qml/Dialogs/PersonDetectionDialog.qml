import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import AppTheme 1.0

Popup {
    id: root
    modal: false
    focus: false
    closePolicy: Popup.CloseOnEscape

    parent: Overlay.overlay

    readonly property real dialogMarginPx: 16
    readonly property real contentMarginPx: 16
    readonly property real buttonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 44
    property int acceptHoldMs: 900

    width: parent ? Math.min(560, parent.width - dialogMarginPx * 2) : 560
    height: parent ? Math.min(620, parent.height - dialogMarginPx * 2) : 620
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    property string message: ""
    property string imageSource: ""

    signal redetectRequested()
    signal startTestRequested()

    function reposition()
    {
        if (!parent) {
            return
        }

        x = Math.round((parent.width - width) / 2)
        y = Math.round((parent.height - height) / 2)
    }

    function openWithDetection(messageText, imageSourceText)
    {
        message = messageText
        imageSource = imageSourceText
        reposition()
        open()
    }

    onOpened: reposition()
    onWidthChanged: reposition()
    onHeightChanged: reposition()
    onParentChanged: reposition()

    background: Rectangle {
        color: AppTheme.dialogsCommon.panelBackground
        border.color: AppTheme.dialogsCommon.light
        border.width: 2
        radius: 8
    }

    contentItem: Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: root.contentMarginPx
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: root.message
                color: AppTheme.dialogsCommon.titleText
                font.pixelSize: 16
                font.bold: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Rectangle {
                visible: root.imageSource.length > 0
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 120
                color: AppTheme.dialogsCommon.black
                radius: 6
                clip: true

                Image {
                    anchors.fill: parent
                    anchors.margins: 8
                    source: root.imageSource
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    cache: false
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: root.buttonHeightPx
                spacing: 12

                Button {
                    id: redetectButton
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    Layout.preferredWidth: 1
                    Layout.preferredHeight: root.buttonHeightPx
                    text: qsTr("Volver a detectar")

                    background: Rectangle {
                        color: redetectButton.down ? AppTheme.dialogsCommon.closeButtonPressed : AppTheme.dialogsCommon.light
                        radius: 8
                        border.color: AppTheme.dialogsCommon.primary
                        border.width: 2
                    }

                    contentItem: Label {
                        text: redetectButton.text
                        color: AppTheme.dialogsCommon.primary
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        startHoldAnimation.stop()
                        startButton.holdProgress = 0
                        root.close()
                        root.redetectRequested()
                    }
                }

                Button {
                    id: startButton
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    Layout.preferredWidth: 1
                    Layout.preferredHeight: root.buttonHeightPx
                    text: qsTr("Iniciar test")

                    property real holdProgress: 0

                    NumberAnimation {
                        id: startHoldAnimation
                        target: startButton
                        property: "holdProgress"
                        from: 0
                        to: 1
                        duration: root.acceptHoldMs
                        onFinished: {
                            root.close()
                            root.startTestRequested()
                        }
                    }

                    background: Rectangle {
                        id: startButtonBackground
                        color: AppTheme.dialogsCommon.light
                        radius: 8
                        border.color: AppTheme.dialogsCommon.primary
                        border.width: 2
                        clip: true

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            color: AppTheme.dialogsCommon.acceptProgress
                            width: parent.width * startButton.holdProgress
                            radius: startButtonBackground.radius
                            antialiasing: true
                        }
                    }

                    contentItem: Label {
                        text: startButton.text
                        color: AppTheme.dialogsCommon.primary
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onPressed: {
                        startHoldAnimation.stop()
                        startButton.holdProgress = 0
                        startHoldAnimation.start()
                    }

                    onReleased: {
                        startHoldAnimation.stop()
                        startButton.holdProgress = 0
                    }

                    onCanceled: {
                        startHoldAnimation.stop()
                        startButton.holdProgress = 0
                    }
                }
            }
        }
    }
}
