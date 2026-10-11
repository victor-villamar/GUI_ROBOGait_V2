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

    title: qsTr("Configuración del robot")
    modal: true
    dim: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay

    width: parent ? Math.min(760, parent.width * 0.94) : 760
    height: parent ? Math.min(760, parent.height * 0.9) : 760
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    function normalizedNumber(textValue) {
        return Number(String(textValue).trim().replace(",", "."))
    }

    function isPositiveNumber(textValue) {
        const value = normalizedNumber(textValue)
        return isFinite(value) && value > 0
    }

    function applyNumber(field, propertyName) {
        if (robotSettings && isPositiveNumber(field.text)) {
            robotSettings[propertyName] = normalizedNumber(field.text)
        }

        if (robotSettings) {
            field.text = String(robotSettings[propertyName])
        }
    }

    function applyText(field, propertyName) {
        if (robotSettings && field.acceptableInput && field.text.trim().length > 0) {
            robotSettings[propertyName] = field.text.trim()
        }

        if (robotSettings) {
            field.text = robotSettings[propertyName]
        }
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

    function commitPendingEdits() {
        if (!isAuthorized || !robotSettings) {
            return
        }

        applyNumber(radiusField, "robotRadius")
        applyNumber(linearVelocityField, "maxLinearVelocity")
        applyNumber(angularVelocityField, "maxAngularVelocity")

        if (isAdmin) {
            applyText(cmdVelTopicField, "cmdVelTopic")
            applyText(scanTopicField, "scanTopic")
            applyText(cameraTopicField, "cameraDetectionTopic")
            applyText(mapFrameField, "mapFrame")
            applyText(robotFrameField, "robotFrame")
        }
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
    onAboutToHide: commitPendingEdits()

    Overlay.modal: KeyboardCompatibleOverlay {
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
                            text: qsTr("Estos parámetros se aplican inmediatamente al robot seleccionado.")
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
                                onEditingFinished: root.applyNumber(radiusField, "robotRadius")

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
                                onEditingFinished: root.applyNumber(linearVelocityField, "maxLinearVelocity")

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
                                onEditingFinished: root.applyNumber(angularVelocityField, "maxAngularVelocity")

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
                                onEditingFinished: root.applyText(cmdVelTopicField, "cmdVelTopic")

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
                                onEditingFinished: root.applyText(scanTopicField, "scanTopic")

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
                                onEditingFinished: root.applyText(cameraTopicField, "cameraDetectionTopic")

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
                                onEditingFinished: root.applyText(mapFrameField, "mapFrame")

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
                                onEditingFinished: root.applyText(robotFrameField, "robotFrame")

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
                            text: qsTr("Información de solo lectura. Estas rutas dependen del montaje configurado al iniciar Docker.")
                            color: AppTheme.settings.groupDescription
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
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
                            text: qsTr("Los cambios solo se conservan durante la sesión actual y no modifican config.yaml.")
                            color: AppTheme.settings.white
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                Button {
                    id: doneButton
                    Layout.alignment: Qt.AlignHCenter
                    Layout.bottomMargin: 8
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: root.buttonHeightPx
                    text: qsTr("Cerrar")

                    background: Rectangle {
                        color: doneButton.down ? AppTheme.settings.primary : AppTheme.settings.white
                        radius: 8
                        border.color: AppTheme.settings.primaryDark
                        border.width: 2
                    }

                    contentItem: Label {
                        text: doneButton.text
                        color: AppTheme.settings.primaryDark
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: root.close()
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
