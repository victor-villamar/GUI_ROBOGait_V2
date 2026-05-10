import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.0
import QtQuick.Window 2.15
import AppTheme 1.0

Dialog {
    id: root

    title: qsTr("Ajustes de desarrollador")
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay

    Overlay.modal: Rectangle {
        anchors.fill: parent
        color: AppTheme.settings.overlayDim
    }

    width: Math.min(500, parent ? parent.width * 0.9 : 500)
    implicitHeight: header.height + contentItem.implicitHeight
    height: Math.min(implicitHeight, parent ? parent.height * 0.8 : implicitHeight)
    

    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0
    
    enabled: userSession && userSession.role === "manager"
    readonly property real buttonHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.buttonHeight, 0) : 52
    readonly property real tabHeightPx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.tabHeight, 0) : 44
    
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
        color: AppTheme.settings.lightBlue
        border.color: AppTheme.settings.white
        border.width: 2
        radius: 12
    }
    
    header: Rectangle {
        color: AppTheme.settings.primaryDark
        height: 50
        radius: 12
        
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 12
            color: AppTheme.settings.primaryDark
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
                color: AppTheme.settings.white
                font.pixelSize: 18
                font.bold: true
            }
        }

        Text {
            anchors.right: closeButton.left
            anchors.rightMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            text: userSession ? qsTr("User: %1").arg(userSession.username) : ""
            color: AppTheme.settings.lightBlue
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
                color: closeButton.down ? AppTheme.settings.closeButtonPressedBg : AppTheme.settings.white
                radius: 8
                border.color: AppTheme.settings.primaryDark
                border.width: 2
            }
            
            contentItem: Label {
                text: closeButton.text
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
        id: contentRoot
        implicitWidth: contentColumn.implicitWidth + contentColumn.anchors.margins * 2
        implicitHeight: contentColumn.implicitHeight + contentColumn.anchors.margins * 2

        ColumnLayout {
            id: contentColumn
            anchors.fill: parent
            anchors.margins: 20
            spacing: 14

            Rectangle {
                Layout.fillWidth: true
                height: 10
                color: "transparent"
            }

            TabBar {
                id: settingsTabBar
                Layout.fillWidth: true
                height: root.tabHeightPx
                spacing: 6
                background: Item {}

                TabButton {
                    id: rosTabButton
                    text: qsTr("Descubrimiento de robots")
                    height: root.tabHeightPx
                    padding: 0
                    leftPadding: 12
                    rightPadding: 12

                    background: Rectangle {
                        color: rosTabButton.checked ? AppTheme.settings.primaryDark : AppTheme.settings.tabUnselectedBg
                        border.color: AppTheme.settings.white
                        border.width: 2
                        radius: 8
                    }

                    contentItem: Text {
                        text: rosTabButton.text
                        color: rosTabButton.checked ? AppTheme.settings.white : AppTheme.settings.primaryDark
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                TabButton {
                    id: rosDomainTabButton
                    text: qsTr("Ros Domain Id")
                    height: root.tabHeightPx
                    padding: 0
                    leftPadding: 12
                    rightPadding: 12

                    background: Rectangle {
                        color: rosDomainTabButton.checked ? AppTheme.settings.primaryDark : AppTheme.settings.tabUnselectedBg
                        border.color: AppTheme.settings.white
                        border.width: 2
                        radius: 8
                    }

                    contentItem: Text {
                        text: rosDomainTabButton.text
                        color: rosDomainTabButton.checked ? AppTheme.settings.white : AppTheme.settings.primaryDark
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 10
                color: "transparent"
            }

            StackLayout {
                id: settingsTabStack
                Layout.fillWidth: true
                currentIndex: settingsTabBar.currentIndex
                Layout.preferredHeight: Math.max(rosSettingsTab.implicitHeight, rosDomainIdTab.implicitHeight)
                Layout.minimumHeight: Layout.preferredHeight

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
                Layout.preferredHeight: root.buttonHeightPx
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
                    color: AppTheme.settings.white
                    radius: 8
                    border.color: AppTheme.settings.primaryDark
                    border.width: 2
                    clip: true

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        color: AppTheme.settings.applyProgressFill
                        width: parent.width * applyButton.holdProgress
                        radius: applyButtonBackground.radius
                        antialiasing: true
                    }
                }

                contentItem: Label {
                    text: applyButton.text
                    color: AppTheme.settings.primaryDark
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
                color: AppTheme.settings.white
                radius: 8
                border.color: AppTheme.settings.primaryDark
                border.width: 1
            
                Text {
                    id: infoText
                    anchors.fill: parent
                    anchors.margins: 10
                    text: qsTr("ℹ️ Los cambios se aplican inmediatamente. El descubrimiento de namespaces ayuda a organizar robots en entornos multi-robot.")
                    font.pixelSize: 11
                    color: AppTheme.settings.primaryDark
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                }
            }
        }
    }
}
