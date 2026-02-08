import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0

Dialog {
    id: root

    title: qsTr("Ajustes de desarrollador")
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: "#A0505050"
    }

    width: Math.min(500, parent ? parent.width * 0.9 : 500)
    implicitHeight: header.height + contentItem.implicitHeight
    height: Math.min(implicitHeight, parent ? parent.height * 0.8 : implicitHeight)
    

    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0
    
    enabled: userSession && userSession.role === "manager"
    
    function reposition()
    {
        if (!parent) {
            return
        }

        x = Math.round((parent.width - width) / 2)
        y = Math.round((parent.height - height) / 2)
    }

    onOpened: reposition()
    onWidthChanged: reposition()
    onHeightChanged: reposition()
    onParentChanged: reposition()

    background: Rectangle {
        color: "#a9cfe8"
        border.color: "#ffffff"
        border.width: 2
        radius: 12
    }
    
    header: Rectangle {
        color: "#045671"
        height: 50
        radius: 12
        
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 12
            color: "#045671"
        }
        
        Row {
            anchors.left: parent.left
            anchors.right: closeButton.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 20
            anchors.rightMargin: 10
            spacing: 16
            
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: root.title
                color: "white"
                font.pixelSize: 18
                font.bold: true
            }
        }

        Text {
            anchors.right: closeButton.left
            anchors.rightMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            text: userSession ? qsTr("User: %1").arg(userSession.username) : ""
            color: "#a9cfe8"
            font.pixelSize: 14
            font.bold: true
        }
        
        Button {
            id: closeButton
            width: 40
            height: 34
            text: "✕"
            anchors.right: parent.right
            anchors.rightMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            
            background: Rectangle {
                color: closeButton.down ? "#518bb7" : "#ffffff"
                radius: 8
                border.color: "#045671"
                border.width: 2
            }
            
            contentItem: Label {
                text: closeButton.text
                color: "#045671"
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            
            onClicked: root.close()
        }
    }
    
    contentItem: Item {
        id: contentRoot
        implicitWidth: contentColumn.implicitWidth + contentColumn.anchors.margins * 2
        implicitHeight: contentColumn.implicitHeight + contentColumn.anchors.margins * 2

        ColumnLayout {
            id: contentColumn
            anchors.fill: parent
            anchors.margins: 20
            spacing: 14

            TabBar {
                id: settingsTabBar
                Layout.fillWidth: true
                spacing: 6
                background: Item {}

                TabButton {
                    id: rosTabButton
                    text: qsTr("Ros Namespaces")
                    padding: 8

                    background: Rectangle {
                        color: rosTabButton.checked ? "#045671" : "#e6f2f8"
                        border.color: "#ffffff"
                        border.width: 2
                        radius: 8
                    }

                    contentItem: Text {
                        text: rosTabButton.text
                        color: rosTabButton.checked ? "#ffffff" : "#045671"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                TabButton {
                    id: rosDomainTabButton
                    text: qsTr("Ros Domain Id")
                    padding: 8

                    background: Rectangle {
                        color: rosDomainTabButton.checked ? "#045671" : "#e6f2f8"
                        border.color: "#ffffff"
                        border.width: 2
                        radius: 8
                    }

                    contentItem: Text {
                        text: rosDomainTabButton.text
                        color: rosDomainTabButton.checked ? "#ffffff" : "#045671"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            StackLayout {
                id: settingsTabStack
                Layout.fillWidth: true
                property int fixedHeight: 200
                Layout.preferredHeight: fixedHeight
                Layout.minimumHeight: fixedHeight
                Layout.maximumHeight: fixedHeight
                currentIndex: settingsTabBar.currentIndex

                RosSettingsTab {
                    id: rosSettingsTab
                    Layout.fillWidth: true
                }

                RosDomainIdTab {
                    id: rosDomainIdTab
                    Layout.fillWidth: true
                }
            }

            Button {
                id: applyButton
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 8
                Layout.preferredWidth: 220
                Layout.preferredHeight: 52
                text: qsTr("Aplicar")
                enabled: developerSettings ? developerSettings.hasPendingChanges : false
            
                property real holdProgress: 0
                property bool holdToApply: true
                property int applyHoldMs: 900

                NumberAnimation {
                    id: holdAnimation
                    target: applyButton
                    property: "holdProgress"
                    from: 0
                    to: 1
                    duration: applyButton.applyHoldMs
                    onFinished: {
                        if (rosSettingsTab.applyChanges() && rosDomainIdTab.applyChanges()) {
                            if(developerSettings && developerSettings.applyChanges()){
                                root.close();
                            }
                        }
                        else {
                            applyButton.holdProgress = 0
                        }
                    }
                }

                background: Rectangle {
                    id: applyButtonBackground
                    color: "#ffffff"
                    radius: 8
                    border.color: "#045671"
                    border.width: 2
                    clip: true

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        color: "#00C8FF"
                        width: parent.width * applyButton.holdProgress
                        radius: applyButtonBackground.radius
                        antialiasing: true
                    }
                }

                contentItem: Label {
                    text: applyButton.text
                    color: "#045671"
                    font.pixelSize: 16
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onPressed: {
                    if(!holdToApply) {
                        return
                    }

                    holdAnimation.stop()
                    applyButton.holdProgress = 0
                    holdAnimation.start()
                }

                onReleased: {
                    if(!holdToApply) {
                        return
                    }

                    holdAnimation.stop()
                    applyButton.holdProgress = 0
                }

                onCanceled: {
                    if(!holdToApply) {
                        return
                    }

                    holdAnimation.stop()
                    applyButton.holdProgress = 0
                }

                onClicked: {
                    if(!holdToApply) {
                        if (rosSettingsTab.applyChanges() && rosDomainIdTab.applyChanges()) {
                            if(developerSettings && developerSettings.applyChanges()){
                                root.close();
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 60
                color: "#ffffff"
                radius: 8
                border.color: "#045671"
                border.width: 1
            
                Text {
                    id: infoText
                    anchors.fill: parent
                    anchors.margins: 10
                    text: qsTr("ℹ️ Los cambios se aplican inmediatamente. El descubrimiento de namespaces ayuda a organizar robots en entornos multi-robot.")
                    font.pixelSize: 11
                    color: "#045671"
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                }
            }
        }
    }
}
