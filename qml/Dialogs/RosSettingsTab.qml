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
            if (userSession.rosManager && developerSettings) {
                userSession.rosManager.useNamespaceDiscovery = developerSettings.useNamespaceDiscovery
            }

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
            text: qsTr("Configuracion del uso del Namespaces")
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
            id: namespaceGroup
            title: qsTr("Descubrimiento con Namespaces")
            label: Text {
                text: namespaceGroup.title
                font: namespaceGroup.font
                color: "#000000"
            }

            width: parent.width

            contentItem: Column {
                spacing: 10
                width: parent.width

                Text {
                    text: qsTr("Habilitar descubrimiento de robots basado en namespaces")
                    font.pixelSize: 12
                    color: "#666666"
                    width: parent.width
                    wrapMode: Text.WordWrap
                }
                
                CheckBox {
                    id: namespaceCheckbox
                    text: qsTr("Usar descubrimiento por namespace")
                    width: parent.width
                    
                    enabled: root.enabled
                    checked: developerSettings ? developerSettings.useNamespaceDiscovery : true

                    indicator: Rectangle {
                        implicitWidth: 20
                        implicitHeight: 20
                        x: namespaceCheckbox.leftPadding
                        y: parent.height / 2 - height / 2
                        radius: 4
                        border.color: "#045671"
                        border.width: 2
                        color: "#ffffff"

                        Text {
                            anchors.centerIn: parent
                            text: "✓"
                            color: "#000000"
                            font.pixelSize: 14
                            font.bold: true
                            visible: namespaceCheckbox.checked
                        }
                    }

                    contentItem: Text {
                        text: namespaceCheckbox.text
                        font: namespaceCheckbox.font
                        color: "#045671"
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: namespaceCheckbox.indicator.width + namespaceCheckbox.spacing
                    }

                    onCheckedChanged: {
                        if (!userSession || !userSession.isAuthenticated) {
                            return
                        }

                        if (userSession.role === "manager" && developerSettings) {
                            developerSettings.useNamespaceDiscovery = checked
                        }

                        else {
                            checked = developerSettings ? developerSettings.useNamespaceDiscovery : true
                        }
                    }
                }

                Text {
                    text: checked ? 
                          qsTr("El descubrimiento de robots se realiza utilizando sus namespaces de ROS") :
                          qsTr("El descubrimiento por namespaces está deshabilitado")
                    font.pixelSize: 11
                    color: checked ? "#2E7D32" : "#E65100"
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.bold: true

                    property bool checked: namespaceCheckbox.checked
                }
            }
        }  
    }
}
