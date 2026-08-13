import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import AppTheme 1.0
import "qrc:/Components"

Dialog {
    id: root
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose
    parent: Overlay.overlay

    property string doctorName: ""
    property string patientName: ""
    property string locationName: ""
    property string dateText: ""
    property string annotations: ""

    readonly property real inputHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.inputHeight, 0) : 40
    readonly property real buttonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 44
    readonly property real titleFontPx: Math.max(28, Math.min(40, Math.round(buttonHeightPx * 0.70)))
    readonly property real bodyFontPx: Math.max(16, Math.min(22, Math.round(buttonHeightPx * 0.38)))
    readonly property real annotationsHeightPx: Math.max(100, Math.round(inputHeightPx * 2.6))
    readonly property bool hasVerticalScroll: flick.contentHeight > flick.height + 1
    readonly property real scrollTrackWidth: 10
    readonly property real scrollTrackGap: 6
    readonly property real scrollTrackEdgeMargin: 2
    readonly property real scrollTrackReserve: scrollTrackWidth + scrollTrackGap + scrollTrackEdgeMargin

    signal saveRequested(string annotations)

    width: parent ? Math.min(760, parent.width * 0.90) : 760
    height: keyboardHelper.computedHeight
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? keyboardHelper.computedY : 0

    KeyboardAwareHelper {
        id: keyboardHelper
        target: root
        maxDialogHeight: parent ? Math.min(520, parent.height * 0.88) : 520
    }

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: AppTheme.selection.overlayDim
    }

    background: Rectangle {
        color: AppTheme.selection.primary
        border.color: AppTheme.selection.white
        border.width: 2
        radius: 12
    }

    contentItem: Item {
        anchors.fill: parent

        Flickable {
            id: flick
            anchors.fill: parent
            anchors.margins: 16
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            interactive: root.hasVerticalScroll
            contentWidth: width
            contentHeight: content.implicitHeight

            onContentHeightChanged: {
                if (!root.hasVerticalScroll) {
                    contentY = 0
                }
            }
            onHeightChanged: {
                if (!root.hasVerticalScroll) {
                    contentY = 0
                }
            }

            Item {
                id: content
                width: flick.width
                implicitHeight: Math.max(formColumn.implicitHeight, flick.height)

                Column {
                    id: formColumn
                    width: Math.min(680, Math.max(0, content.width - 24 - root.scrollTrackReserve))
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: Math.max(10, Math.round((flick.height - formColumn.implicitHeight) / 2))
                    spacing: 10

                    Text {
                        width: parent.width
                        text: qsTr("Guardar Experimento")
                        color: AppTheme.selection.white
                        font.pixelSize: root.titleFontPx
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: AppTheme.selection.white
                        opacity: 0.55
                    }

                    GridLayout {
                        width: parent.width
                        columns: 2
                        rowSpacing: 6
                        columnSpacing: 12

                        Text {
                            text: qsTr("Doctor:")
                            color: AppTheme.selection.white
                            font.pixelSize: root.bodyFontPx
                            font.bold: true
                        }

                        Text {
                            text: root.doctorName
                            color: AppTheme.selection.white
                            font.pixelSize: root.bodyFontPx
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        Text {
                            text: qsTr("Paciente:")
                            color: AppTheme.selection.white
                            font.pixelSize: root.bodyFontPx
                            font.bold: true
                        }

                        Text {
                            text: root.patientName
                            color: AppTheme.selection.white
                            font.pixelSize: root.bodyFontPx
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        Text {
                            text: qsTr("Ubicación:")
                            color: AppTheme.selection.white
                            font.pixelSize: root.bodyFontPx
                            font.bold: true
                        }

                        Text {
                            text: root.locationName
                            color: AppTheme.selection.white
                            font.pixelSize: root.bodyFontPx
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        Text {
                            text: qsTr("Fecha:")
                            color: AppTheme.selection.white
                            font.pixelSize: root.bodyFontPx
                            font.bold: true
                        }

                        Text {
                            text: root.dateText
                            color: AppTheme.selection.white
                            font.pixelSize: root.bodyFontPx
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }

                    Text {
                        width: parent.width
                        text: qsTr("Anotaciones")
                        color: AppTheme.selection.white
                        font.pixelSize: root.bodyFontPx
                        font.bold: true
                    }

                    TextArea {
                        id: annotationsText
                        width: parent.width
                        height: root.annotationsHeightPx
                        wrapMode: TextEdit.Wrap
                        selectByMouse: true
                        text: root.annotations
                        placeholderTextColor: AppTheme.selection.placeholderText
                        placeholderText: qsTr("Escriba observaciones del experimento...")
                        color: AppTheme.selection.black
                        font.pixelSize: root.bodyFontPx

                        background: Rectangle {
                            radius: 10
                            color: AppTheme.selection.white
                            border.color: AppTheme.selection.primaryDark
                            border.width: 1
                        }
                    }

                    Item {
                        width: 1
                        height: 1
                    }

                    Row {
                        width: parent.width
                        spacing: 14

                        Button {
                            id: cancelButton
                            width: (parent.width - parent.spacing) / 2
                            height: root.buttonHeightPx
                            padding: 0

                            background: Rectangle {
                                color: cancelButton.down ? AppTheme.selection.primary : AppTheme.selection.white
                                radius: 8
                                border.color: AppTheme.selection.primaryDark
                                border.width: 2
                            }

                            contentItem: Label {
                                text: qsTr("CANCELAR")
                                color: AppTheme.selection.primaryDark
                                font.pixelSize: Math.round(root.bodyFontPx * 1.05)
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: root.reject()
                        }

                        Button {
                            id: saveButton
                            width: (parent.width - parent.spacing) / 2
                            height: root.buttonHeightPx
                            padding: 0

                            background: Rectangle {
                                color: saveButton.down ? AppTheme.selection.primary : AppTheme.selection.white
                                radius: 8
                                border.color: AppTheme.selection.primaryDark
                                border.width: 1
                            }

                            contentItem: Label {
                                text: qsTr("GUARDAR")
                                color: AppTheme.selection.primaryDark
                                font.pixelSize: Math.round(root.bodyFontPx * 1.05)
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                root.annotations = annotationsText.text
                                root.saveRequested(root.annotations)
                            }
                        }
                    }
                }
            }
        }

        CustomScrollTrack {
            flickable: flick
            formColumn: formColumn
            startItem: annotationsText
            trackWidth: root.scrollTrackWidth
            gapFromForm: root.scrollTrackGap
            edgeMargin: root.scrollTrackEdgeMargin
            trackVisible: root.hasVerticalScroll
        }
    }
}
