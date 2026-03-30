import QtQuick 2.15

import "qrc:/Register"

Item {
    id: root

    property alias loginPage: loginPage
    property alias signInPage: signInPage
    property alias guestPage: guestPage
    property int contentCenterOffset: 0
    property real keyboardSafeArea: 0

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
