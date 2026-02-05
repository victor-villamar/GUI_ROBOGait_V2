import QtQuick 2.15
import "qrc:/Dialogs"

SettingsBadgeForm {
    id: root
    
    visible: userSession && userSession.isAuthenticated && userSession.role === "manager" && 
             applicationFlow && applicationFlow.state === "robot_connection"
    enabled: visible
 
    settingsButton.onClicked: {

        if (developerSettingsDialog.visible) {
            developerSettingsDialog.close()
        }
        else {
            developerSettingsDialog.open()
        }
    }
    
    DeveloperSettingsDialog {
        id: developerSettingsDialog
    }
}
