import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Register"

Item {
    id: root

    property alias loginPage: loginPage
    property alias signInPage: signInPage
    property alias guestPage: guestPage
    property int contentCenterOffset: 0
    property real keyboardSafeArea: 0
    signal appExitRequested()

    readonly property real iconButtonSizePx: uiSizingSettings ? uiSizingSettings.interactivePx(uiSizingSettings.iconButtonSize, 0) : 50
    readonly property real iconGlyphSizePx: uiSizingSettings ? uiSizingSettings.px(uiSizingSettings.iconGlyphSize, 0) : 24

    Button {
        id: shutdownButton
        width: root.iconButtonSizePx
        height: root.iconButtonSizePx
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 12
        anchors.rightMargin: 16
        z: 2

        background: null

        contentItem: Image {
            source: "qrc:/qmlresources/icons/shutdown.svg"
            width: root.iconGlyphSizePx
            height: root.iconGlyphSizePx
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        onClicked: root.appExitRequested()
    }

    Login {
        id: loginPage
        anchors.fill: parent
        contentCenterOffset: root.contentCenterOffset
        keyboardSafeArea: root.keyboardSafeArea
    }

    SignIn {
        id: signInPage
        anchors.fill: parent
        visible: false
        enabled: false
        contentCenterOffset: root.contentCenterOffset
        keyboardSafeArea: root.keyboardSafeArea
    }

    Guest {
        id: guestPage
        anchors.fill: parent
        visible: false
        enabled: false
        contentCenterOffset: root.contentCenterOffset
        keyboardSafeArea: root.keyboardSafeArea
    }
}
