import QtQuick 2.15
import QtQuick.Controls 2.15

import "qrc:/Dialogs"
import "qrc:/Views"
import RobotServiceBridge 1.0

SelectMapForm {
    id: root

    ListModel {
        id: mapsModel
    }

    property string pendingMapName: ""
    property string pendingMapLocation: ""
    property string pendingMapDescription: ""
    property string pendingDeleteMapName: ""
    property bool pendingDeleteMapHasHistory: false
    property bool waitingForMappingStart: false
    property bool waitingForDeleteMap: false

    readonly property var robotServiceBridge : (userSession && userSession.rosManager && userSession.rosManager.robotManager)
                                                  ? userSession.rosManager.robotManager.robotServiceBridge
                                                  : null

    mapsListView.model: mapsModel
    selectedMapName: (userSession && userSession.currentMapName) ? userSession.currentMapName : ""

    function loadMaps() {
        mapsModel.clear()

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

        if (selectedMapName && selectedMapName !== "")
        {
            var idx = -1
            for (var j = 0; j < mapsModel.count; ++j) {
                if (mapsModel.get(j).map_name === selectedMapName)
                {
                    idx = j
                    break
                }
            }
            mapsListView.currentIndex = idx
        }
        else
        {
            mapsListView.currentIndex = -1
        }
    }

    function completeMapDeletion() {
        var databaseUpdated = dbManager.deleteMap(pendingDeleteMapName)
        if (!databaseUpdated) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: %1").arg(dbManager.lastError)
            errorPopup.open()
            return
        }

        if (!pendingDeleteMapHasHistory
                                        && userSession && userSession.rosManager
                                        && userSession.rosManager.robotManager) {
            var previewDeleted = userSession.rosManager.robotManager.deleteMapPreview(pendingDeleteMapName)
            if (!previewDeleted) {
                errorPopup.errorRectangleTextError.text = qsTr("Advertencia: No se pudo borrar la vista previa del mapa")
                errorPopup.open()
            }
        }

        if (mapDetailsDialog.visible && mapDetailsDialog.mapName === pendingDeleteMapName) {
            mapDetailsDialog.close()
        }

        if (userSession && userSession.currentMapName === pendingDeleteMapName) {
            userSession.clearMap()
        }

        selectedMapName = ""
        mapsListView.currentIndex = -1
        loadMaps()
    }

    StackView.onActivated: loadMaps()

    onAddMapRequested: mapRegisterDialog.open()

    onMapClicked: function(mapIndex, mapNameValue) {
        mapsListView.currentIndex = mapIndex

        if (!dbManager) {
            return
        }

        var details = dbManager.getMapDetails(mapNameValue)

        var previewPath = ""
        if (userSession && userSession.rosManager && userSession.rosManager.robotManager) {
            previewPath = userSession.rosManager.robotManager.getMapPreviewPath(mapNameValue)
        }

        mapDetailsDialog.openForMap(mapNameValue, details, previewPath)
    }

    onMapDeleteRequested: function(mapIndex, mapNameValue) {
        if (!dbManager) {
            return
        }

        var impact = dbManager.getMapDeletionImpact(mapNameValue)
        if (!impact.valid) {
            errorPopup.errorRectangleTextError.text = qsTr("Error: %1").arg(dbManager.lastError)
            errorPopup.open()
            return
        }

        pendingDeleteMapName = mapNameValue
        pendingDeleteMapHasHistory = impact.will_archive
        if (pendingDeleteMapHasHistory) {
            var testLabel = impact.experiment_count === 1 ? qsTr("prueba") : qsTr("pruebas")
            deleteConfirmDialog.message = qsTr("El mapa %1 está asociado a %2 %3. Se eliminará del robot, pero se conservará en el historial y dejará de estar disponible para nuevas sesiones.")
                                                  .arg(mapNameValue)
                                                  .arg(impact.experiment_count)
                                                  .arg(testLabel)
        }
        else {
            deleteConfirmDialog.message = qsTr("¿Borrar completamente el mapa %1?").arg(mapNameValue)
        }
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

            var ok = robotServiceBridge.startMapping()
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
            if (!dbManager || !robotServiceBridge) {
                errorPopup.errorRectangleTextError.text = qsTr("Error: No hay un robot disponible para borrar el mapa")
                errorPopup.open()
                return
            }

            waitingForDeleteMap = true
            busyDialog.openWithMessage(qsTr("Borrando mapa del robot..."))
            var okRemote = robotServiceBridge.deleteMap(pendingDeleteMapName)
            if(!okRemote)
            {
                waitingForDeleteMap = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = qsTr("Advertencia: No se pudo borrar el mapa en el robot")
                errorPopup.open()
                return
            }
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
        target: robotServiceBridge

        function onStatusChanged() {

            if(!waitingForMappingStart) {
                return
            }

            if(robotServiceBridge.status === RobotServiceBridge.RUNNING) {
                waitingForMappingStart = false
                busyDialog.close()
                if(root.StackView.view) {
                    root.StackView.view.push(mapViewPage)
                }
            }
            else if (robotServiceBridge.status === RobotServiceBridge.ERROR) {
                waitingForMappingStart = false
                busyDialog.close()
                errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la creación del mapa")
                errorPopup.open()
            }
        }

        function onRequestFinished(success) {
            if (waitingForMappingStart) {
                if (!success) {
                    waitingForMappingStart = false
                    busyDialog.close()
                    errorPopup.errorRectangleTextError.text = qsTr("Error: No se pudo iniciar la creación del mapa")
                    errorPopup.open()
                }
                return
            }

            if (waitingForDeleteMap) {
                waitingForDeleteMap = false
                busyDialog.close()
                if (!success) {
                    errorPopup.errorRectangleTextError.text = qsTr("Advertencia: No se pudo borrar el mapa en el robot")
                    errorPopup.open()
                    return
                }

                completeMapDeletion()
            }
        }
    }
}
