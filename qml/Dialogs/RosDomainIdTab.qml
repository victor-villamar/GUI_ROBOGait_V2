import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0

Item {
    id: root

    enabled: userSession && userSession.role === "manager" && userSession.isAuthenticated

    implicitWidth: contentLayout.childrenRect.width + 40
    implicitHeight: contentLayout.childrenRect.height + 40

    function applyChanges() {
        if (!userSession || !userSession.isAuthenticated) {
            return false
        }

        if (userSession.role === "manager") {
            return developerSettings.applyChanges()
        }
        else {
            return false
        }
    }

    function resetChanges() {
        developerSettings.resetChanges()
    }

    Column {
        id: contentLayout
        x: 20
        y: 20
        width: parent.width - 40
        height: implicitHeight
        spacing: 15

        Text {
            text: qsTr("Configuracion del Ros Domain Id")
            font.pixelSize: 18
            font.bold: true
            color: root.enabled ? "#045671" : "#999999"
            width: parent.width
        }

        Rectangle {
            height: 2
            color: "#ffffff"
            width: parent.width
        }

        GroupBox {
            id: domainGroup
            title: qsTr("Ros Domain Id")
            label: Text {
                text: domainGroup.title
                font: domainGroup.font
                color: "#000000"
            }

            width: parent.width

            contentItem: Column {
                spacing: 10
                width: parent.width

                Text {
                    text: qsTr("Selecciona el dominio de ROS")
                    font.pixelSize: 10
                    color: "#666666"
                    width: parent.width
                    wrapMode: Text.WordWrap
                }

                SpinBox {
                    id: domainSpinBox
                    from: 0
                    to: 232
                    value: developerSettings ? developerSettings.rosDomainId : 0
                    editable: true
                    enabled: root.enabled
                    width: 130

                    onValueChanged: {
                        if (!userSession || !userSession.isAuthenticated) {
                            return
                        }

                        if (userSession.role === "manager" && developerSettings) {
                            developerSettings.rosDomainId = value
                        }
                    }

                    contentItem: TextInput {
                        text: domainSpinBox.textFromValue(domainSpinBox.value, domainSpinBox.locale)
                        font: domainSpinBox.font
                        color: "#045671"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        readOnly: !domainSpinBox.editable
                        validator: IntValidator { bottom: domainSpinBox.from; top: domainSpinBox.to }
                        anchors.fill: parent
                        anchors.rightMargin: 24
                    }

                    up.indicator: Rectangle {
                        implicitWidth: 16
                        implicitHeight: 14
                        color: "#a9cfe8"
                        border.color: "#045671"
                        border.width: 1
                        radius: 3
                        anchors.right: parent.right
                        anchors.rightMargin: 3
                        anchors.top: parent.top
                        anchors.topMargin: 3
                        Text {
                            anchors.centerIn: parent
                            text: "▲"
                            color: "#045671"
                            font.pixelSize: 10
                        }
                    }

                    down.indicator: Rectangle {
                        implicitWidth: 16
                        implicitHeight: 14
                        color: "#a9cfe8"
                        border.color: "#045671"
                        border.width: 1
                        radius: 3
                        anchors.right: parent.right
                        anchors.rightMargin: 3
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 3
                        Text {
                            anchors.centerIn: parent
                            text: "▼"
                            color: "#045671"
                            font.pixelSize: 10
                        }
                    }

                    background: Rectangle {
                        implicitWidth: 130
                        implicitHeight: 36
                        radius: 6
                        color: "#ffffff"
                        border.color: "#045671"
                        border.width: 1
                    }
                }

                Text {
                    text: qsTr("El Domain Id define el dominio DDS para el descubrimiento")
                    font.pixelSize: 11
                    color: "#2E7D32"
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.bold: true
                }
            }
        }
    }
}
