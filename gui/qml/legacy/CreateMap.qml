import QtQuick 2.15
import "extras"

CreateMapForm {
    id: root

    property string currentState: "cm_nothing"
    state: currentState

    Connections {
        target: joystick
        function onDirChanged(linear, angular) {
            stringHandler.setCurrentVelocity(angular, linear)
        }
    }
    mapPageForm_buttonStop.onClicked: {
        applicationFlow.popToHome()
        mapInfo.sendStopProcesses();
    }

    Connections {
        target: stringHandler
        function onAngularVelocityChanged(){
            angular_value = stringHandler.angularVelocity
        }
        function onLinealVelocityChanged(){
            linear_value = stringHandler.linealVelocity
        }
        function onImageSourceChanged() {
            if (stringHandler.imageSource !== "") {
                imageDisplay.source = stringHandler.imageSource
                imageDisplay.visible = true
            } else {
                imageDisplay.visible = false
            }
        }
        function onTypeSaveMapChanged(){

            if(stringHandler.typeSaveMap === 0){
                currentState = "cm_back_button_pressed"
                applicationFlow.type_save = -1
                stringHandler.setTypeSaveMap(-1)
            }
        }

    }

    joystick.onPressed: {
        console.log("Joystick presionado");
    }

    joystick.onReleased: {
        stringHandler.setCurrentVelocity(0.0, 0.0)
        stringHandler.setSaveMap(0)
        console.log("Joystick liberado");
    }

    property Timer resetTimer: Timer {
        interval: 2000 // 2 seconds
        repeat: false
        onTriggered: stringHandler.startSLAM()
    }

    resetMapButton.onClicked: {
        stringHandler.stopSLAM()
        resetTimer.running = true
    }

    saveMapButton.onClicked: {
        currentState = "cm_save_map"
    }

    save_page.save.onClicked: {
        console.log("Guardar presionado")
        stringHandler.setSaveMap(1)
        ddbb.mapCreateSave(1) // guardar que hay mapa creado
        stringHandler.setNameMap(ddbb.mapNameTemporal)
        ddbb.selectAllMap()
        stringHandler.stopSLAM()
        applicationFlow.popFunction()
        applicationFlow.popFunction()
    }

    save_page.cancel.onClicked: {
        console.log("Presionado cancelar")
        currentState = "cm_nothing"
    }

    save_page.no_save.onClicked: {
        console.log("Presionado cancelar")
        stringHandler.setSaveMap(1)
        currentState = "cm_nothing"
        ddbb.mapCreateSave(0) // borrar la información que haya del mapa si no se guarda
        stringHandler.stopSLAM()
        applicationFlow.popFunction()
        applicationFlow.popFunction()
    }

    states: [
        State {
            name: "cm_nothing"
            StateChangeScript {
                script: console.log("estoy en CREATE_MAP cm_nothing")
            }
            PropertyChanges {
                target: save_page
                visible: false
                opacity: 0
            }
        },
        State {
            name: "cm_save_map"
            StateChangeScript {
                script: console.log("estoy en CREATE_MAP cm_save_map")
            }
            PropertyChanges {
                target: save_page
                visible: true
                opacity: 1 //save_again
                currentState: "sp_save_without_not_save_button"
            }
        },
        State {
            name: "cm_back_button_pressed"
            StateChangeScript {
                script: console.log("estoy en CREATE_MAP cm_back_button_pressed")
            }
            PropertyChanges {
                target: save_page
                visible: true
                opacity: 1 //save_again
                currentState: "sp_save_with_not_save_button"
            }
        }
    ]

}

// CreateMapForm {
//     property bool visible_value: visible_value
//     // property bool savePageVisible: savePageVisible
//     property bool visible_save: visible_save
//     property int type_save: type_save
//     // property bool mapping: mapping

//     states: [
//         State {
//             name: "mapping"
//             when: customSwitch.checked === false
//             StateChangeScript {
//                 script: {
//                     console.log("estoy en mapping")
//                     clearImageSource()
//                     stringHandler.setMapping(0)
//                     stringHandler.sendStateRemoteControlledHandler(0,1)
//                 }
//             }
//             PropertyChanges {
//                 target: imageDisplay
//                 visible: false
//             }
//             PropertyChanges {
//                 target: save_page
//                 visible: false
//                 opacity: 0
//             }
//             // PropertyChanges {
//             //     target: applicationFlow.toolbar
//             //     saveButton.opacity: 0
//             //     saveButton.enabled: false
//             // }
//         },
//         State {
//             name: "hidden"
//             when: visible_value === false
//             StateChangeScript {
//                 script: {
//                     console.log("estoy en hidden")
//                     clearImageSource();
//                     stringHandler.setMapping(1)
//                 }
//             }
//             PropertyChanges {
//                 target: imageDisplay
//                 visible: false
//             }
//             PropertyChanges {
//                 target: save_page
//                 visible: false
//                 opacity: 0
//             }
//         },
//         State {
//             name: "nothing"
//             when: (type_save === 0 || type_save === -1) && visible_value === true
//             StateChangeScript {
//                 script: {
//                     console.log("estoy en nothing")

//                     stringHandler.setMapping(1)
//                     stringHandler.sendStateRemoteControlledHandler(1,1)
//                 }
//             }
//             PropertyChanges {
//                 target: imageDisplay
//                 visible: true
//             }
//             PropertyChanges {
//                 target: save_page
//                 visible: false
//                 opacity: 0
//             }
//             PropertyChanges {
//                 target: joystick
//                 enabled: true // Deshabilitar joystick
//             }
//             PropertyChanges {
//                 target: switchRow
//                 enabled: true
//             }
//             // PropertyChanges {
//             //     target: applicationFlow.toolbar
//             //     saveButton.opacity: 1
//             //     saveButton.enabled: true
//             // }
//         },
//         State {
//             name: "no_save"
//             when: type_save === 1 && visible_value === true
//             StateChangeScript {
//                 script: {
//                     console.log("estoy en no_save")

//                     stringHandler.setMapping(1)
//                 }
//             }
//             PropertyChanges {
//                 target: imageDisplay
//                 visible: true
//             }
//             PropertyChanges {
//                 target: save_page
//                 visible: true
//                 state_save_page: 1
//                 opacity: 1
//             }
//             PropertyChanges {
//                 target: joystick
//                 enabled: false // Deshabilitar joystick
//             }
//             PropertyChanges {
//                 target: switchRow
//                 enabled: false
//             }
//         },
//         State {
//             name: "save"
//             when: type_save === 2 && visible_value === true
//             StateChangeScript {
//                 script: {
//                     console.log("estoy en save")

//                     stringHandler.setMapping(1)
//                 }
//             }
//             PropertyChanges {
//                 target: imageDisplay
//                 visible: true
//             }
//             PropertyChanges {
//                 target: save_page
//                 visible: true
//                 state_save_page: 2
//                 opacity: 1
//             }
//             PropertyChanges {
//                 target: joystick
//                 enabled: false // Deshabilitar joystick
//             }
//             PropertyChanges {
//                 target: switchRow
//                 enabled: false
//             }
//         },
//         State {
//             name: "save_again"
//             when: type_save === 3 && visible_value === true
//             StateChangeScript {
//                 script: {
//                     console.log("estoy en save_again")

//                     stringHandler.setMapping(1)
//                 }
//             }
//             PropertyChanges {
//                 target: imageDisplay
//                 visible: true
//             }
//             PropertyChanges {
//                 target: save_page
//                 visible: true
//                 state_save_page: 3
//                 opacity: 1
//             }
//             PropertyChanges {
//                 target: joystick
//                 enabled: false // Deshabilitar joystick
//             }
//             PropertyChanges {
//                 target: switchRow
//                 enabled: false
//             }
//         }
//     ]


//     // Definir los estados
//     function clearImageSource() {
//         stringHandler.setImageSource("") // Borra la fuente de la imagen
//     }



//     joystick.onDirChanged: {
//         stringHandler.setCurrentMove(linear, angular)
//         // console.log("Velocidad lineal: "+ linear + " Velocidad angular: " + angular);
//     }

//     // Conectando el signal pressed al manejador de presionado
//     joystick.onPressed: {
//         console.log("Joystick presionado");
//     }

//     // Conectando el signal released al manejador de liberado
//     joystick.onReleased: {
//         stringHandler.setCurrentMove("0.0", "0.0")
//         stringHandler.setSaveMap(0)
//         console.log("Joystick liberado");
//     }

//     save_page.cancel.onClicked: {
//         console.log("Presionado Cancelar")

//         applicationFlow.type_save = -1
//         stringHandler.setSaveMap(0)
//         stringHandler.setTypeSaveMap(-1)
//         console.log(type_save)
//     }

//     save_page.no_save.onClicked: {
//         console.log("Presionado no guardar")
//         stringHandler.setSaveMap(1)
//         applicationFlow.backButton()
//     }

//     save_page.save.onClicked: {
//         console.log("Presionado Guardar")
//         applicationFlow.type_save = 0
//         stringHandler.setSaveMap(1)
//         stringHandler.setTypeSaveMap(1)
//         stringHandler.setNameMap(save_page.text_name.text)
//     }

// }
