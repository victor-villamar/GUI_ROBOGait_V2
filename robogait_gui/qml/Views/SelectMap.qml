import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"
import "qrc:/Views"
import CommandExecutorBridge 1.0

SelectMapForm {
    id: root

    ListModel {
        id: mapsModel
    }

    property string pendingMapName: ""
    property string pendingMapLocation: ""
    property string pendingMapDescription: ""
    property string pendingDeleteMapName: ""
    property bool waitingForMappingStart: false

    readonly property var commandExecutorBridge : (userSession && userSession.rosManager && userSession.rosManager.robotManager)
                                                  ? userSession.rosManager.robotManager.commandExecutorBridge
                                                  : null

    mapsListView.model: mapsModel
    selectedMapIndex: -1

    function loadMaps() {
        mapsModel.clear()
        selectedMapIndex = -1

        if (!dbManager) {
            return
        }

        var maps = dbManager.listMaps()
        for (var i = 0; i < maps.length; ++i) {
            var m = maps[i]
            var name = m.map_name || m.name || m.display || m
            mapsModel.append({
                "map_name": name
            })
        }
    }

    StackView.onActivated: loadMaps()

    onAddMapRequested: mapRegisterDialog.open()

    onMapClicked: function(mapIndex, mapNameValue) {
        selectedMapIndex = mapIndex
        mapsListView.currentIndex = mapIndex

        if (!dbManager) {
            return
        }

        var details = dbManager.getMapDetails(mapNameValue)

        var previewPath = ""
        if (userSession && userSession.rosManager && userSession.rosManager.robotManager
            && userSession.rosManager.robotManager.mapVisualizationManager) {
            previewPath = userSession.rosManager.robotManager.mapVisualizationManager.getMapPreviewPath(mapNameValue)
        }

        mapDetailsDialog.openForMap(mapNameValue, details, previewPath)
    }

    onMapDeleteRequested: function(mapIndex, mapNameValue) {
        pendingDeleteMapName = mapNameValue
        deleteConfirmDialog.message = qsTr("¿Borrar el mapa %1?").arg(mapNameValue)
        deleteConfirmDialog.openWithMessage(deleteConfirmDialog.message)
    }

    MapRegisterDialog {
        id: mapRegisterDialog

        onCreateMapRequested: function(name, location, description) {
            pendingMapName = name
            pendingMapLocation = location
            pendingMapDescription = description

            waitingForMappingStart = true
            busyDialog.openWithMessage(qsTr("Iniciando creación del mapa..."))

            var ok = commandExecutorBridge.startMapping()
            if(!ok)
            {
                waitingForMappingStart = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la creación del mapa")
                errorPopup.open()
                return
            }
        }
    }

    Component {
        id: mapViewPage

        MapView {
            confirmBackNavigation: true
            mapName: root.pendingMapName
            mapLocation: root.pendingMapLocation
            mapDescription: root.pendingMapDescription
        }
    }

    MapDetailsDialog {
        id: mapDetailsDialog

        onAcceptedSelection: function(mapNameValue) {
            if (userSession) {
                userSession.assignMap(mapNameValue)
            }
            if (root.StackView.view) {
                root.StackView.view.pop()
            }
        }
    }

    ConfirmationDialog {
        id: deleteConfirmDialog
        holdToAccept: true
        acceptText: qsTr("Borrar")

        onAccepted: {
            if (!dbManager) {
                return
            }

            var okRemote = commandExecutorBridge.deleteMap(pendingDeleteMapName)
            if(!okRemote)
            {
                errorPopup.errorRectangleTextError.text = qsTr("Advertencia: No se pudo borrar el mapa en el robot")
                errorPopup.open()
                return
            }

            var ok = dbManager.deleteMap(pendingDeleteMapName)
            if (!ok) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: %1").arg(dbManager.lastError)
                errorPopup.open()
                return
            }

            var previewDeleted = true
            if (userSession && userSession.rosManager && userSession.rosManager.robotManager
                && userSession.rosManager.robotManager.mapVisualizationManager) {
                previewDeleted = userSession.rosManager.robotManager.mapVisualizationManager.deleteMapPreview(pendingDeleteMapName)
            }

            if (!previewDeleted) {
                errorPopup.errorRectangleTextError.text = qsTr("Advertencia: No se pudo borrar la vista previa del mapa")
                errorPopup.open()
                return
            }

            if (mapDetailsDialog.visible && mapDetailsDialog.mapName === pendingDeleteMapName) {
                mapDetailsDialog.close()
                selectedMapIndex = -1
                mapsListView.currentIndex = -1
            }

            loadMaps()
        }
    }

    BusyDialog {
        id: busyDialog
    }

    ErrorRectangle {
        id: errorPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: ""
    }

    Connections {
        target: commandExecutorBridge

        function onStatusChanged() {

            if(!waitingForMappingStart) {
                return
            }

            if(commandExecutorBridge.status === CommandExecutorBridge.RUNNING) {
                waitingForMappingStart = false
                busyDialog.close()
                if(root.StackView.view) {
                    root.StackView.view.push(mapViewPage)
                }
            }
            else if (commandExecutorBridge.status === CommandExecutorBridge.ERROR) {
                waitingForMappingStart = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la creación del mapa")
                errorPopup.open()
            }
        }
    }
}
