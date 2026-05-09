import QtQuick 2.15
import QtQuick.Controls 2.15
import AppTheme 1.0

import "qrc:/Dialogs"
import "qrc:/Components"

Dialog {
    id: root
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose

    parent: Overlay.overlay

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: AppTheme.selection.overlayDim
    }

    width: parent ? Math.min(580, parent.width * 0.90) : 580
    height: keyboardHelper.computedHeight

    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? keyboardHelper.computedY : 0

    KeyboardAwareHelper {
        id: keyboardHelper
        target: root
        maxDialogHeight: parent ? Math.min(820, parent.height * 0.90) : 820
    }

    signal patientRegistered()
    readonly property real inputHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.inputHeight, 0) : 40
    readonly property real buttonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 44
    readonly property real textAreaHeightPx: Math.round(inputHeightPx * 3)
    readonly property bool hasVerticalScroll: flick.contentHeight > flick.height + 1
    readonly property real scrollTrackWidth: 10
    readonly property real scrollTrackGap: 6
    readonly property real scrollTrackEdgeMargin: 2
    readonly property real scrollTrackReserve: scrollTrackWidth + scrollTrackGap + scrollTrackEdgeMargin

    function resetForm()
    {
        nameField.text = ""
        lastNameField.text = ""
        ageField.text = ""
        weightField.text = ""
        heightField.text = ""
        descriptionField.text = ""
    }

    onOpened: resetForm()

    background: Rectangle {
        color: AppTheme.selection.primary
        border.color: AppTheme.selection.white
        border.width: 2
        radius: 12
    }

    contentItem: Item {
        anchors.fill: parent

        Button {
            id: closeButton
            z: 2
            width: 28
            height: 28
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 12
            anchors.rightMargin: 12

            background: Rectangle {
                color: closeButton.down ? AppTheme.selection.closeButtonPressedBackground : AppTheme.selection.white
                radius: 8
                border.color: AppTheme.selection.primaryDark
                border.width: 2
            }

            contentItem: Image {
                source: "qrc:/qmlresources/icons/black/xmark_solid.svg"
                fillMode: Image.PreserveAspectFit
                sourceSize.width: 14
                sourceSize.height: 14
                anchors.centerIn: parent
            }

            onClicked: root.close()
        }

        Flickable {
            id: flick
            z: 1
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
                    width: Math.min(450, Math.max(0, content.width - 32 - root.scrollTrackReserve))
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 15
                    leftPadding: 20
                    rightPadding: 20
                    topPadding: 20
                    bottomPadding: 20
                    readonly property real innerWidth: Math.max(0, width - leftPadding - rightPadding)

                    Text {
                        text: qsTr("Registrar paciente")
                        color: AppTheme.selection.white
                        font.pixelSize: 40
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        padding: 10
                    }

                    TextField {
                        id: nameField
                        width: formColumn.innerWidth
                        height: root.inputHeightPx
                        color: AppTheme.selection.black
                        font.pointSize: 15
                        verticalAlignment: TextInput.AlignVCenter
                        leftPadding: 10
                        rightPadding: 10
                        placeholderText: qsTr("Nombre")

                        background: Rectangle {
                            radius: 10
                            color: AppTheme.selection.white
                        }
                    }

                    TextField {
                        id: lastNameField
                        width: formColumn.innerWidth
                        height: root.inputHeightPx
                        color: AppTheme.selection.black
                        font.pointSize: 15
                        verticalAlignment: TextInput.AlignVCenter
                        leftPadding: 10
                        rightPadding: 10
                        placeholderText: qsTr("Apellidos")

                        background: Rectangle {
                            radius: 10
                            color: AppTheme.selection.white
                        }
                    }

                    TextField {
                        id: ageField
                        width: formColumn.innerWidth
                        height: root.inputHeightPx
                        color: AppTheme.selection.black
                        font.pointSize: 15
                        verticalAlignment: TextInput.AlignVCenter
                        leftPadding: 10
                        rightPadding: 10
                        placeholderText: qsTr("Edad (años)")

                        background: Rectangle {
                            radius: 10
                            color: AppTheme.selection.white
                        }
                    }

                    TextField {
                        id: weightField
                        width: formColumn.innerWidth
                        height: root.inputHeightPx
                        color: AppTheme.selection.black
                        font.pointSize: 15
                        verticalAlignment: TextInput.AlignVCenter
                        leftPadding: 10
                        rightPadding: 10
                        placeholderText: qsTr("Peso (Kg)")

                        background: Rectangle {
                            radius: 10
                            color: AppTheme.selection.white
                        }
                    }

                    TextField {
                        id: heightField
                        width: formColumn.innerWidth
                        height: root.inputHeightPx
                        color: AppTheme.selection.black
                        font.pointSize: 15
                        verticalAlignment: TextInput.AlignVCenter
                        leftPadding: 10
                        rightPadding: 10
                        placeholderText: qsTr("Altura (cm)")

                        background: Rectangle {
                            radius: 10
                            color: AppTheme.selection.white
                        }
                    }

                    TextArea {
                        id: descriptionField
                        width: formColumn.innerWidth
                        height: root.textAreaHeightPx
                        color: AppTheme.selection.black
                        font.pointSize: 15
                        wrapMode: TextEdit.Wrap
                        placeholderText: qsTr("Descripción")

                        background: Rectangle {
                            radius: 10
                            color: AppTheme.selection.white
                        }
                    }

                    Button {
                        id: addButton
                        width: 240
                        height: root.buttonHeightPx
                        anchors.horizontalCenter: parent.horizontalCenter

                        background: Rectangle {
                            radius: 10
                            color: AppTheme.selection.lightBlue
                        }

                        contentItem: Label {
                            text: qsTr("AÑADIR")
                            color: AppTheme.selection.white
                            font.bold: true
                            font.pixelSize: 20
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (!dbManager) {
                                return
                            }

                            var age = parseInt(ageField.text)
                            var weight = parseFloat(weightField.text)
                            var height = parseFloat(heightField.text)

                            if (!nameField.text || !lastNameField.text) {
                                errorPopup.errorRectangleTextError.text = qsTr("Error: Has dejado campos vacíos")
                                errorPopup.open()
                                return
                            }

                            if (isNaN(age) || isNaN(weight) || isNaN(height)) {
                                errorPopup.errorRectangleTextError.text = qsTr("Error: Edad, peso y altura deben ser números válidos")
                                errorPopup.open()
                                return
                            }

                            var ok = dbManager.registerPatient(
                                        nameField.text,
                                        lastNameField.text,
                                        age,
                                        weight,
                                        height,
                                        descriptionField.text)

                            if (!ok) {
                                errorPopup.errorRectangleTextError.text = qsTr("Error: %1").arg(dbManager.lastError)
                                errorPopup.open()
                                return
                            }

                            root.close()
                            root.patientRegistered()
                        }
                    }
                }
            }
        }

        CustomScrollTrack {
            flickable: flick
            formColumn: formColumn
            startItem: nameField
            trackWidth: root.scrollTrackWidth
            gapFromForm: root.scrollTrackGap
            edgeMargin: root.scrollTrackEdgeMargin
            trackVisible: root.hasVerticalScroll
        }

        ErrorRectangle {
            id: errorPopup
            z: 3
            anchors.centerIn: parent
            errorRectangleTextError.text: ""
        }
    }
}
