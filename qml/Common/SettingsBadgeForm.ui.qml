import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    
    width: 56
    height: 56
    
    property alias settingsButton: settingsButton
    property bool enabled: true

    signal settingsClicked()

    Button {
        id: settingsButton
        anchors.fill: parent
        
        enabled: root.enabled
        
        icon.source: "qrc:/qmlresources/icons/config.svg"
        icon.width: 32
        icon.height: 28
        
        background: Rectangle {
            color: settingsButton.pressed ? "#a9cfe8" : settingsButton.hovered ? "#a9cfe8" : "transparent"
            radius: 28

            Behavior on color {
                ColorAnimation {
                    duration: 150
                    easing.type: Easing.InOutQuad
                }
            }
        }

        onClicked: {
            root.settingsClicked()
        }
        
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Ajustes de desarrollador")
        ToolTip.delay: 500
    }
}
