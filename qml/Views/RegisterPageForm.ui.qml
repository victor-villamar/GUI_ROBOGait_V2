import QtQuick 2.15

import "qrc:/Register"

Item {
    id: root

    property alias loginPage: loginPage
    property alias signInPage: signInPage
    property alias guestPage: guestPage

    Login {
        id: loginPage
        anchors.fill: parent
    }

    SignIn {
        id: signInPage
        anchors.fill: parent
        visible: false
        enabled: false
    }

    Guest {
        id: guestPage
        anchors.fill: parent
        visible: false
        enabled: false
    }
}
