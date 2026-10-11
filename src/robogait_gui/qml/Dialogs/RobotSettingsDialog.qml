import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0
import AppTheme 1.0

import "qrc:/Components"

Dialog {
    id: root

    readonly property bool isAdmin: userSession && userSession.role === "manager"
    readonly property bool isDoctor: userSession && userSession.role === "doctor"
    readonly property bool isAuthorized: userSession
                                                 && userSession.isAuthenticated
                                                 && (isDoctor || isAdmin)
    readonly property real buttonHeightPx: uiSizingSettings
                                                ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0)
                                                : 52
    readonly property real inputHeightPx: uiSizingSettings
                                               ? uiSizingSettings.interactivePx(uiSizingSettings.inputHeight, 0)
                                               : 52
    readonly property real contentPadding: 20
    readonly property bool hasVerticalScroll: contentFlick.contentHeight > contentFlick.height + 1
    readonly property real scrollTrackWidth: 10
    readonly property real scrollTrackGap: 6
    readonly property real scrollTrackEdgeMargin: 2
    readonly property real scrollTrackReserve: scrollTrackWidth + scrollTrackGap + scrollTrackEdgeMargin
    readonly property real desiredHeight: parent ? Math.min(760, parent.height * 0.9) : 760
    readonly property bool generalValuesValid: isPositiveNumber(radiusField.text)
                                                        && isPositiveNumber(linearVelocityField.text)
                                                        && isPositiveNumber(angularVelocityField.text)
    readonly property bool administratorValuesValid: !isAdmin
                                                              || (cmdVelTopicField.acceptableInput
                                                                  && scanTopicField.acceptableInput
                                                                  && cameraTopicField.acceptableInput
                                                                  && mapFrameField.acceptableInput
                                                                  && robotFrameField.acceptableInput)
    readonly property bool canSave: isAuthorized && generalValuesValid && administratorValuesValid
    property var focusedField: null

    title: qsTr("Configuración del robot")
    modal: false
    dim: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay

    width: parent ? Math.min(760, parent.width * 0.94) : 760
    height: keyboardHelper.computedHeight
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? keyboardHelper.computedY : 0

    KeyboardAwareHelper {
        id: keyboardHelper
        target: root
        maxDialogHeight: root.desiredHeight
        topPadding: 20
        keyboardPadding: 12
        margin: topPadding + keyboardPadding
    }

    function normalizedNumber(textValue) {
        return Number(String(textValue).trim().replace(",", "."))
    }

    function isPositiveNumber(textValue) {
        const value = normalizedNumber(textValue)
        return isFinite(value) && value > 0
    }

    function ensureFocusedFieldVisible() {
        if (!focusedField || !focusedField.activeFocus || !contentFlick) {
            return
        }

        const fieldPosition = focusedField.mapToItem(contentFlick.contentItem, 0, 0)
        const margin = 12
        const fieldTop = fieldPosition.y - margin
        const fieldBottom = fieldPosition.y + focusedField.height + margin
        const viewportTop = contentFlick.contentY
        const viewportBottom = viewportTop + contentFlick.height
        const maximumContentY = Math.max(0, contentFlick.contentHeight - contentFlick.height)

        if (fieldTop < viewportTop) {
            contentFlick.contentY = Math.max(0, fieldTop)
        }
        else if (fieldBottom > viewportBottom) {
            contentFlick.contentY = Math.min(maximumContentY,
                                            fieldBottom - contentFlick.height)
        }
    }

    function followFieldFocus(field) {
        if (!field.activeFocus) {
            return
        }

        focusedField = field
        Qt.callLater(ensureFocusedFieldVisible)
    }

    function syncFields() {
        if (!robotSettings) {
            return
        }

        radiusField.text = String(robotSettings.robotRadius)
        linearVelocityField.text = String(robotSettings.maxLinearVelocity)
        angularVelocityField.text = String(robotSettings.maxAngularVelocity)
        cmdVelTopicField.text = robotSettings.cmdVelTopic
        scanTopicField.text = robotSettings.scanTopic
        cameraTopicField.text = robotSettings.cameraDetectionTopic
        mapFrameField.text = robotSettings.mapFrame
        robotFrameField.text = robotSettings.robotFrame
        mapImagePathField.text = robotSettings.mapImagePath
        robotMapPathField.text = robotSettings.robotMapPath
    }

    function saveChanges() {
        if (!canSave || !robotSettings) {
            return false
        }

        robotSettings.robotRadius = normalizedNumber(radiusField.text)
        robotSettings.maxLinearVelocity = normalizedNumber(linearVelocityField.text)
        robotSettings.maxAngularVelocity = normalizedNumber(angularVelocityField.text)

        if (isAdmin) {
            robotSettings.cmdVelTopic = cmdVelTopicField.text.trim()
            robotSettings.scanTopic = scanTopicField.text.trim()
            robotSettings.cameraDetectionTopic = cameraTopicField.text.trim()
            robotSettings.mapFrame = mapFrameField.text.trim()
            robotSettings.robotFrame = robotFrameField.text.trim()
        }

        close()
        return true
    }

    function openForCurrentSettings() {
        if (!isAuthorized || !robotSettings) {
            return
        }

        syncFields()
        open()
    }

    onIsAuthorizedChanged: {
        if (!isAuthorized && visible) {
            close()
        }
    }

    onOpened: syncFields()
    onHeightChanged: Qt.callLater(ensureFocusedFieldVisible)
    onClosed: {
        focusedField = null
        Qt.inputMethod.hide()
    }

    Overlay.modeless: KeyboardCompatibleOverlay {
        anchors.fill: parent
        color: AppTheme.settings.overlayDim
        onBackgroundPressed: root.close()
    }

    background: Rectangle {
        color: AppTheme.settings.lightBlue
        border.color: AppTheme.settings.white
        border.width: 2
        radius: 12
    }

    header: Rectangle {
        height: 54
        color: AppTheme.settings.primaryDark
        radius: 12

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 12
            color: AppTheme.settings.primaryDark
        }

        Text {
            anchors.left: parent.left
            anchors.right: roleText.left
            anchors.leftMargin: 20
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: root.title
            color: AppTheme.settings.white
            font.pixelSize: 18
            font.bold: true
            elide: Text.ElideRight
        }

        Text {
            id: roleText
            anchors.right: closeButton.left
            anchors.rightMargin: 14
            anchors.verticalCenter: parent.verticalCenter
            text: root.isAdmin ? qsTr("Administrador") : qsTr("Doctor")
            color: AppTheme.settings.lightBlue
            font.pixelSize: 13
            font.bold: true
        }

        Button {
            id: closeButton
            width: 40
            height: 34
            anchors.right: parent.right
            anchors.rightMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            padding: 0

            background: Rectangle {
                color: closeButton.down ? AppTheme.settings.closeButtonPressedBg : AppTheme.settings.white
                radius: 8
                border.color: AppTheme.settings.primaryDark
                border.width: 2
            }

            contentItem: Label {
                text: "✕"
                color: AppTheme.settings.primaryDark
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.close()
        }
    }

    contentItem: Item {
        Flickable {
            id: contentFlick
            anchors.fill: parent
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            interactive: root.hasVerticalScroll
            contentWidth: width
            contentHeight: contentColumn.implicitHeight + root.contentPadding * 2

            ColumnLayout {
                id: contentColumn
                x: root.contentPadding
                y: root.contentPadding
                width: Math.max(0, contentFlick.width - root.contentPadding * 2 - root.scrollTrackReserve)
                spacing: 16

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: generalLayout.implicitHeight + 28
                    color: AppTheme.settings.white
                    radius: 10
                    border.color: AppTheme.settings.primary
                    border.width: 1

                    ColumnLayout {
                        id: generalLayout
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Movimiento y tamaño")
                            color: AppTheme.settings.primaryDark
                            font.pixelSize: 18
                            font.bold: true
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Los cambios se aplicarán al robot seleccionado al pulsar Guardar.")
                            color: AppTheme.settings.groupDescription
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 10

                            Text {
                                text: qsTr("Tamaño del robot (radio)")
                                color: AppTheme.settings.primaryDark
                                font.pixelSize: 14
                            }

                            TextField {
                                id: radiusField
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.inputHeightPx
                                placeholderText: qsTr("Metros")
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                selectByMouse: true
                                color: AppTheme.settings.primaryDark
                                onActiveFocusChanged: root.followFieldFocus(radiusField)

                                background: Rectangle {
                                    color: AppTheme.settings.white
                                    radius: 6
                                    border.color: root.isPositiveNumber(radiusField.text)
                                                  ? AppTheme.settings.primaryDark
                                                  : AppTheme.settings.statusWarning
                                    border.width: 1
                                }
                            }

                            Text {
                                text: qsTr("Velocidad máxima de movimiento")
                                color: AppTheme.settings.primaryDark
                                font.pixelSize: 14
                            }

                            TextField {
                                id: linearVelocityField
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.inputHeightPx
                                placeholderText: qsTr("Metros por segundo")
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                selectByMouse: true
                                color: AppTheme.settings.primaryDark
                                onActiveFocusChanged: root.followFieldFocus(linearVelocityField)

                                background: Rectangle {
                                    color: AppTheme.settings.white
                                    radius: 6
                                    border.color: root.isPositiveNumber(linearVelocityField.text)
                                                  ? AppTheme.settings.primaryDark
                                                  : AppTheme.settings.statusWarning
                                    border.width: 1
                                }
                            }

                            Text {
                                text: qsTr("Velocidad máxima de giro")
                                color: AppTheme.settings.primaryDark
                                font.pixelSize: 14
                            }

                            TextField {
                                id: angularVelocityField
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.inputHeightPx
                                placeholderText: qsTr("Radianes por segundo")
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                selectByMouse: true
                                color: AppTheme.settings.primaryDark
                                onActiveFocusChanged: root.followFieldFocus(angularVelocityField)

                                background: Rectangle {
                                    color: AppTheme.settings.white
                                    radius: 6
                                    border.color: root.isPositiveNumber(angularVelocityField.text)
                                                  ? AppTheme.settings.primaryDark
                                                  : AppTheme.settings.statusWarning
                                    border.width: 1
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: administratorLayout.implicitHeight + 28
                    visible: root.isAdmin
                    color: AppTheme.settings.white
                    radius: 10
                    border.color: AppTheme.settings.primary
                    border.width: 1

                    ColumnLayout {
                        id: administratorLayout
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Comunicación ROS")
                            color: AppTheme.settings.primaryDark
                            font.pixelSize: 18
                            font.bold: true
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Los tópicos mantienen automáticamente el namespace del robot seleccionado.")
                            color: AppTheme.settings.groupDescription
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 10

                            Text {
                                text: qsTr("Tópico de movimiento")
                                color: AppTheme.settings.primaryDark
                                font.pixelSize: 14
                            }

                            TextField {
                                id: cmdVelTopicField
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.inputHeightPx
                                selectByMouse: true
                                color: AppTheme.settings.primaryDark
                                validator: RegularExpressionValidator {
                                    regularExpression: /^\/?[A-Za-z_][A-Za-z0-9_]*(\/[A-Za-z_][A-Za-z0-9_]*)*$/
                                }
                                onActiveFocusChanged: root.followFieldFocus(cmdVelTopicField)

                                background: Rectangle {
                                    color: AppTheme.settings.white
                                    radius: 6
                                    border.color: cmdVelTopicField.acceptableInput
                                                  ? AppTheme.settings.primaryDark
                                                  : AppTheme.settings.statusWarning
                                    border.width: 1
                                }
                            }

                            Text {
                                text: qsTr("Tópico del láser")
                                color: AppTheme.settings.primaryDark
                                font.pixelSize: 14
                            }

                            TextField {
                                id: scanTopicField
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.inputHeightPx
                                selectByMouse: true
                                color: AppTheme.settings.primaryDark
                                validator: RegularExpressionValidator {
                                    regularExpression: /^\/?[A-Za-z_][A-Za-z0-9_]*(\/[A-Za-z_][A-Za-z0-9_]*)*$/
                                }
                                onActiveFocusChanged: root.followFieldFocus(scanTopicField)

                                background: Rectangle {
                                    color: AppTheme.settings.white
                                    radius: 6
                                    border.color: scanTopicField.acceptableInput
                                                  ? AppTheme.settings.primaryDark
                                                  : AppTheme.settings.statusWarning
                                    border.width: 1
                                }
                            }

                            Text {
                                text: qsTr("Tópico de detección de personas")
                                color: AppTheme.settings.primaryDark
                                font.pixelSize: 14
                            }

                            TextField {
                                id: cameraTopicField
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.inputHeightPx
                                selectByMouse: true
                                color: AppTheme.settings.primaryDark
                                validator: RegularExpressionValidator {
                                    regularExpression: /^\/?[A-Za-z_][A-Za-z0-9_]*(\/[A-Za-z_][A-Za-z0-9_]*)*$/
                                }
                                onActiveFocusChanged: root.followFieldFocus(cameraTopicField)

                                background: Rectangle {
                                    color: AppTheme.settings.white
                                    radius: 6
                                    border.color: cameraTopicField.acceptableInput
                                                  ? AppTheme.settings.primaryDark
                                                  : AppTheme.settings.statusWarning
                                    border.width: 1
                                }
                            }

                            Text {
                                text: qsTr("Frame global del mapa")
                                color: AppTheme.settings.primaryDark
                                font.pixelSize: 14
                            }

                            TextField {
                                id: mapFrameField
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.inputHeightPx
                                selectByMouse: true
                                color: AppTheme.settings.primaryDark
                                validator: RegularExpressionValidator {
                                    regularExpression: /^[A-Za-z_][A-Za-z0-9_]*(\/[A-Za-z_][A-Za-z0-9_]*)*$/
                                }
                                onActiveFocusChanged: root.followFieldFocus(mapFrameField)

                                background: Rectangle {
                                    color: AppTheme.settings.white
                                    radius: 6
                                    border.color: mapFrameField.acceptableInput
                                                  ? AppTheme.settings.primaryDark
                                                  : AppTheme.settings.statusWarning
                                    border.width: 1
                                }
                            }

                            Text {
                                text: qsTr("Frame del robot")
                                color: AppTheme.settings.primaryDark
                                font.pixelSize: 14
                            }

                            TextField {
                                id: robotFrameField
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.inputHeightPx
                                selectByMouse: true
                                color: AppTheme.settings.primaryDark
                                validator: RegularExpressionValidator {
                                    regularExpression: /^[A-Za-z_][A-Za-z0-9_]*(\/[A-Za-z_][A-Za-z0-9_]*)*$/
                                }
                                onActiveFocusChanged: root.followFieldFocus(robotFrameField)

                                background: Rectangle {
                                    color: AppTheme.settings.white
                                    radius: 6
                                    border.color: robotFrameField.acceptableInput
                                                  ? AppTheme.settings.primaryDark
                                                  : AppTheme.settings.statusWarning
                                    border.width: 1
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: mapLayout.implicitHeight + 28
                    color: AppTheme.settings.white
                    radius: 10
                    border.color: AppTheme.settings.primary
                    border.width: 1

                    ColumnLayout {
                        id: mapLayout
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Rutas del mapa")
                            color: AppTheme.settings.primaryDark
                            font.pixelSize: 18
                            font.bold: true
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Ruta local de imágenes")
                            color: AppTheme.settings.primaryDark
                            font.pixelSize: 14
                        }

                        TextField {
                            id: mapImagePathField
                            Layout.fillWidth: true
                            Layout.preferredHeight: root.inputHeightPx
                            readOnly: true
                            selectByMouse: true
                            color: AppTheme.settings.groupDescription

                            background: Rectangle {
                                color: AppTheme.settings.tabUnselectedBg
                                radius: 6
                                border.color: AppTheme.settings.primary
                                border.width: 1
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Ruta del mapa en el robot")
                            color: AppTheme.settings.primaryDark
                            font.pixelSize: 14
                        }

                        TextField {
                            id: robotMapPathField
                            Layout.fillWidth: true
                            Layout.preferredHeight: root.inputHeightPx
                            readOnly: true
                            selectByMouse: true
                            color: AppTheme.settings.groupDescription

                            background: Rectangle {
                                color: AppTheme.settings.tabUnselectedBg
                                radius: 6
                                border.color: AppTheme.settings.primary
                                border.width: 1
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(60, noticeRow.implicitHeight + 20)
                    color: AppTheme.settings.primaryDark
                    radius: 8
                    border.color: AppTheme.settings.white
                    border.width: 1

                    RowLayout {
                        id: noticeRow
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Image {
                            Layout.preferredWidth: uiSizingSettings
                                                   ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0)
                                                   : 18
                            Layout.preferredHeight: width
                            source: "qrc:/qmlresources/icons/white/circle_info_solid.svg"
                            fillMode: Image.PreserveAspectFit
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Los cambios guardados solo se conservan durante la sesión actual.")
                            color: AppTheme.settings.white
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                Button {
                    id: saveButton
                    Layout.alignment: Qt.AlignHCenter
                    Layout.bottomMargin: 8
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: root.buttonHeightPx
                    text: qsTr("Guardar")
                    enabled: root.canSave
                    opacity: enabled ? 1.0 : 0.5

                    background: Rectangle {
                        color: saveButton.down ? AppTheme.settings.primary : AppTheme.settings.white
                        radius: 8
                        border.color: AppTheme.settings.primaryDark
                        border.width: 2
                    }

                    contentItem: Label {
                        text: saveButton.text
                        color: AppTheme.settings.primaryDark
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: root.saveChanges()
                }
            }
        }

        CustomScrollTrack {
            flickable: contentFlick
            formColumn: contentColumn
            startItem: contentColumn.children.length > 0 ? contentColumn.children[0] : null
            trackWidth: root.scrollTrackWidth
            gapFromForm: root.scrollTrackGap
            edgeMargin: root.scrollTrackEdgeMargin
            trackVisible: root.hasVerticalScroll
        }
    }
}
