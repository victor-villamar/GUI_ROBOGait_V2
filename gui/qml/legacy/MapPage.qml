import QtQuick 2.15
import "extras"

MapPageForm {

    id:mapPage
    state: "mp_initialPosition"
    // signal mapPage_clear_pressed()
    // signal mapPage_draw_path_pressed()
    // signal mapPage_check_pressed()
    // signal mapPage_edit_pressed()

    ErrorRectangle {
        id: errorPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: "Error: Has puesto el robot en una posicion donde esta prohibido."
    }

    mapPageForm_buttonStop.onClicked: {
        applicationFlow.popToHome()
        mapInfo.sendStopProcesses();
    }

    infoButton.onClicked: {
        infoForm.visible = true
        infoForm.enabled = true
    }

    mapPageForm_buttonNext.onClicked: {
        if(state === "mp_initialPosition")
        {
            // mapInfo.checkInitInitialPose = false;
            if(mapInfo.positionScreen.x === 0 || mapInfo.positionScreen.y === 0)
            {
                errorPopup.errorRectangleTextError.text = "Error: Has intentado seguir adelante sin poner el robot en el mapa"
                errorPopup.open()
                return;
            }
            mapInfo.initBringUp();
        }
        if(state === "mp_goalPosePosition")
        {
            // mapInfo.checkInitInitialPose = false;
            if(mapInfo.finalPathPosition.x === 0 || mapInfo.finalPathPosition.y === 0)
            {
                errorPopup.errorRectangleTextError.text = "Error: Has intentado seguir adelante sin poner el robot en el mapa para indicar la poscion final"
                errorPopup.open()
                return;
            }
        }

        if(state === "mp_initialOrientation")
        {
            if(mapInfo.orientation === 0.00)
            {
                errorPopup.errorRectangleTextError.text = "Error: Has intentado seguir adelante sin poner la orientacion del robot"
                errorPopup.open()
                return;
            }
            // if(!mapInfo.checkInitInitialPose)
            mapInfo.sendInitialPose();
        }
        if(state === "mp_goalPoseOrientation")
        {
            if(mapInfo.finalPathOrientation === 0.00)
            {
                errorPopup.errorRectangleTextError.text = "Error: Has intentado seguir adelante sin poner la orientacion del robot en la poscion final"
                errorPopup.open()
                return;
            }
            // mapInfo.checkInitInitialPose = true;
            mapInfo.sendAllInformationPose()

        }
        if(state === "mp_PathGoalPose")
        {
            // if(mapInfo.finalPathOrientation === 0.00)
            // {
            //     errorPopup.errorRectangleTextError.text = "Error: Has intentado seguir adelante sin poner la orientacion del robot en la poscion final"
            //     errorPopup.open()
            //     return;
            // }
            // mapInfo.checkInitInitialPose = true;
            mapInfo.sendGoalPose()
        }
        if(state === "mp_drawPath")
        {
            if(mapInfo.pixels.length === 0)
            {
                errorPopup.errorRectangleTextError.text = qsTr("Error: Has intentado seguir adelante sin dibujar la trayectoria")
                errorPopup.open()
                return;
            }

            mp_map.checkPath = true
            mp_map.canvas.updatePath()
            if(mapInfo.checkPathBlack())
            {
                errorPopup.errorRectangleTextError.text = qsTr("Error: Estas cerca o encima de una zona negra intenta dibujar de nuevo la trayectoria")
                errorPopup.open()
                return;
            }
            // mapInfo.checkInitInitialPose = true;
            mapInfo.sendWaypointFollower()
        }

        if(state === "mp_drawPathMove" || state === "mp_GoalPoseMove")
        {
            console.log("_______________________ HE pasado por mp_drawPathMove o mp_GoalPoseMove")
            mapInfo.clearNextPath()
            mp_map.canvas.clear_internal()
            mp_map.canvas.clear()
            mp_map.checkPath = true
            mp_map.canvas.updatePath()
        }

        if(state === "mp_initialOrientation" || state === "mp_goalPosePosition" || state === "mp_initialPosition" || state === "mp_goalPoseOrientation" || state === "mp_GoalPoseMove" || state === "mp_PathGoalPose" || state === "mp_drawPath" || state === "mp_drawPathMove")
        {
            mapPage.state = mapPage.mapPageForm_nextState
        }
    }

    mapPageForm_buttonPrevious.onClicked: {
        if (state === "mp_PathGoalPose")
        {
            console.log("Se ha limpiado");
            mapInfo.clearTrajectoryGoalPose();
        }

        mapPage.state = mapPage.mapPageForm_previousState
    }

    mapPageForm_boxImages.onBif_check_pressed: {
        if (state === "mp_drawPath")
        {
            console.log("Le he dado a check pressed")
            mp_map.checkPath = true
            mp_map.canvas.updatePath()
        }
    }
    
    mapPageForm_boxImages.onBif_draw_path_pressed: {
        if (state === "selectAction"){
            mapPage.state = "mp_drawPath"
            mapInfo.sendInitialPose();
        }
        if (state === "mp_selectNextStep")
        {
            applicationFlow.popToManualControl() ///////// TODO
        }

        else if (state === "mp_drawPath")
        {
            console.log("Le he dado a check pressed")
            mp_map.checkPath = true
            mp_map.canvas.updatePath()
        }
    }

    mapPageForm_boxImages.onBif_check_black_pressed: {
        if (state === "mp_drawPath")
        {
            if(mapInfo.checkPathBlack())
            {
                errorPopup.errorRectangleTextError.text = qsTr("Error: Has intentado seguir adelante sin dibujar la trayectoria")
                errorPopup.open()
            }
            else
            {
                console.log("TODO BIEN")
            }
        }

    }



    // mapPageForm_boxImages.bif_edit_pressed: {

    // }

    mapPageForm_boxImages.onBif_clear_pressed: {
        if (state === "mp_initialPosition" || state === "mp_goalPosePosition")
        {    mp_map.canvas.clear()
            mp_map.canvas.enablePainting = true
        }
        else if (state === "mp_drawPath")
        {
            mp_map.canvas.clear_internal()
            mapInfo.clearListPixels()
            mp_map.canvas.enablePainting = true
            mp_map.checkPath = false
        }
        else if (state === "selectAction")
            mapPage.state = "mp_goalPosePosition"
        else if (state === "mp_selectNextStep")
            applicationFlow. popToMainMenu() ////////////////TODO

    }

    mapPageForm_boxImages.onBif_edit_pressed: {
        if (state === "mp_initialPosition")
        {
            mapInfo.setScreenSize(mp_map.imageDisplay.width, mp_map.imageDisplay.height)
            mp_map.canvas.enablePainting = true
        }
        if (state === "mp_goalPosePosition")
        {
            mp_map.canvas.clear_internal()
            mp_map.canvas.enablePainting = true
        }
        if (state === "mp_drawPath")
        {
            mp_map.canvas.enablePainting = true
        }
    }





    states: [
        State {
            name: "mp_initialPosition"
            StateChangeScript { script: console.log("estoy en mp_initialPosition") }
            StateChangeScript { script: mp_map.canvas.requestPaint() }
            PropertyChanges { target: mapPage; mapPageForm_previousState: ""; mapPageForm_nextState: "mp_initialOrientation" }
            PropertyChanges { target: mapPageForm_text; text: qsTr("Al hacer clic en el mapa, se coloca el robot sobre él") }
            PropertyChanges { target: mapPageForm_orientationCircleForm; state: "nothing" }
            PropertyChanges { target: mapPageForm_buttonNext; enabled: true; opacity: 1; text: qsTr("Siguiente") }
            PropertyChanges { target: mapPageForm_buttonPrevious; enabled: false; opacity: 0 }
            PropertyChanges { target: mp_map; map_currentState: "map_initialPosition" }
            PropertyChanges { target: mapPageForm_boxImages; state: "bi_draw_robot" }
            PropertyChanges {
                target: infoForm
                textInfo.text:     "<h2>Información de la pantalla</h2>
                                   <p>En esta pantalla debes establecer la posición inicial del robot en el mapa.
                                   Es importante que sea lo más precisa posible para evitar errores durante la prueba.</p>

                                   <h2>Botones</h2>
                                   <ul>
                                       <li><b>Insertar imagen:</b> Al pulsar da permiso para colocar la imagen del robot en el mapa.</li>
                                       <li><b>Borrar:</b> Al pulsar elimina la imagen del robot del mapa.</li>
                                   </ul>

                                   <h2>¿Necesita ayuda?</h2>
                                   <p>Para asistencia técnica, contacte con ETSIDI en:
                                   <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
            }
        },
        State {
            name: "mp_initialOrientation"
            StateChangeScript { script: console.log("estoy en mp_initialOrientation") }
            StateChangeScript { script: mp_map.canvas.requestPaint() }
            PropertyChanges { target: mapPage; mapPageForm_previousState: "mp_initialPosition"; mapPageForm_nextState: "selectAction" }
            PropertyChanges { target: mapPageForm_text; text: qsTr("Al girar la rueda situada debajo, se indica la orientación del robot") }
            PropertyChanges { target: mapPageForm_orientationCircleForm; state: "orientacion_inicial" }
            PropertyChanges { target: mapPageForm_buttonNext; enabled: true; opacity: 1; text: qsTr("Siguiente") }
            PropertyChanges { target: mapPageForm_buttonPrevious; enabled: true; opacity: 1; text: qsTr("Anterior") }
            PropertyChanges { target: mp_map; map_currentState: "map_initialOrientation" }
            PropertyChanges { target: mapPageForm_boxImages; state: "bi_nothing" }
            PropertyChanges {
                target: infoForm
                textInfo.text:     "<h2>Información de la pantalla</h2>
                                   <p>En esta pantalla debes establecer la orientación inicial del robot en el mapa. <br>
                                   Es importante que sea lo más precisa posible para evitar errores durante la prueba.</p>

                                   <h2>Botones</h2>
                                   <ul>
                                       <li><b>Rueda:</b> Gire la rueda para indicar la orientación, presión la zona más alejada del centro, dentro del circulo, para comenzar.
                                   </ul>

                                   <h2>¿Necesita ayuda?</h2>
                                   <p>Para asistencia técnica, contacte con ETSIDI en:
                                   <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
            }
        },
        State {
            name: "selectAction"
            StateChangeScript { script: console.log("estoy en selectAction") }
            StateChangeScript { script: mp_map.canvas.requestPaint() }
            PropertyChanges { target: mapPage; mapPageForm_previousState: "mp_initialOrientation"; mapPageForm_nextState: "" }
            PropertyChanges { target: mapPageForm_text; text: qsTr("Seleccione la acción que quiere realizar a continuación:") }
            PropertyChanges { target: mapPageForm_orientationCircleForm; state: "nothing" }
            PropertyChanges { target: mapPageForm_buttonNext; enabled: false; opacity: 0 }
            PropertyChanges { target: mapPageForm_buttonPrevious; enabled: true; opacity: 1; text: qsTr("Anterior") }
            PropertyChanges { target: mp_map; map_currentState: "map_selectAction" }
            PropertyChanges { target: mapPageForm_boxImages; state: "bi_select_action" }
            PropertyChanges {
                target: infoForm
                textInfo.text:     "<h2>Información de la pantalla</h2>
                                   <p>En esta pantalla debe elegir como quiere hacer el camino del robot.</p>

                                   <h2>Botones</h2>
                                   <ul>
                                       <li><b>Dibujar trayectoria:</b> A mano alzada dibuja la trajectoria que el robot segirá.</li>
                                       <li><b>Posición Final:</b> Selecciona la posición final pero no la trayectoria que sigue.</li>
                                   </ul>

                                   <h2>¿Necesita ayuda?</h2>
                                   <p>Para asistencia técnica, contacte con ETSIDI en:
                                   <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
            }
        },
        State {
            name: "mp_goalPosePosition"
            StateChangeScript { script: console.log("estoy en mp_goalPosePosition") }
            StateChangeScript { script: mp_map.canvas.requestPaint() }
            StateChangeScript { script: mp_map.canvas.clear_internal() }
            PropertyChanges { target: mapPage; mapPageForm_previousState: "selectAction"; mapPageForm_nextState: "mp_goalPoseOrientation" }
            PropertyChanges { target: mapPageForm_text; text: qsTr("Al hacer clic en el mapa, se coloca una pin para indicar la posición final") }
            PropertyChanges { target: mapPageForm_orientationCircleForm; state: "nothing" }
            PropertyChanges { target: mapPageForm_buttonNext; enabled: true; opacity: 1; text: qsTr("Siguiente") }
            PropertyChanges { target: mapPageForm_buttonPrevious; enabled: true; opacity: 1; text: qsTr("Anterior") }
            PropertyChanges { target: mp_map; map_currentState: "map_goalPosePosition" }
            PropertyChanges { target: mapPageForm_boxImages; state: "bi_draw_robot" }
            PropertyChanges {
                target: infoForm
                textInfo.text:     "<h2>Información de la pantalla</h2>
                                   <p>En esta pantalla debes establecer la posición objetivo del robot en el mapa.
                                   Es importante que sea lo más precisa posible para evitar errores durante la prueba.</p>

                                   <h2>Botones</h2>
                                   <ul>
                                       <li><b>Insertar imagen:</b> Al pulsar da permiso para colocar la imagen del robot en el mapa.</li>
                                       <li><b>Borrar:</b> Al pulsar elimina la imagen del robot del mapa.</li>
                                   </ul>

                                   <h2>¿Necesita ayuda?</h2>
                                   <p>Para asistencia técnica, contacte con ETSIDI en:
                                   <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
            }
        },
        State {
            name: "mp_goalPoseOrientation"
            StateChangeScript { script: console.log("estoy en mp_goalPoseOrientation") }
            StateChangeScript { script: mp_map.canvas.requestPaint() }
            PropertyChanges { target: mapPage; mapPageForm_previousState: "mp_goalPosePosition"; mapPageForm_nextState: "mp_PathGoalPose" }
            PropertyChanges { target: mapPageForm_text; text: qsTr("Al girar la rueda situada debajo, se indica la orientación del robot al final") }
            PropertyChanges { target: mapPageForm_orientationCircleForm; state: "orientacion_final" }
            PropertyChanges { target: mapPageForm_buttonNext; enabled: true; opacity: 1 }
            PropertyChanges { target: mapPage; mapPageForm_nextState_text: qsTr("Enviar") }
            PropertyChanges { target: mapPageForm_buttonPrevious; enabled: true; opacity: 1; text: qsTr("Anterior") }
            PropertyChanges { target: mp_map; map_currentState: "map_goalPoseOrientation" }
            PropertyChanges { target: mapPageForm_boxImages; state: "bi_nothing" }
            PropertyChanges {
                target: infoForm
                textInfo.text:     "<h2>Información de la pantalla</h2>
                                   <p>En esta pantalla debes establecer la orientación objetivo del robot en el mapa. <br>
                                   Es importante que sea lo más precisa posible para evitar errores durante la prueba.</p>

                                   <h2>Botones</h2>
                                   <ul>
                                       <li><b>Rueda:</b> Gire la rueda para indicar la orientación, presión la zona más alejada del centro, dentro del circulo, para comenzar.
                                   </ul>

                                   <h2>¿Necesita ayuda?</h2>
                                   <p>Para asistencia técnica, contacte con ETSIDI en:
                                   <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
            }
        },
        State {
            name: "mp_PathGoalPose"
            StateChangeScript { script: console.log("estoy en mp_PathGoalPose") }
            StateChangeScript { script: mp_map.canvas.requestPaint() }
            PropertyChanges { target: mapPage; mapPageForm_previousState: "mp_goalPoseOrientation"; mapPageForm_nextState: "mp_GoalPoseMove" }
            PropertyChanges { target: mapPageForm_text; text: qsTr("Trayectoria del robot") }
            PropertyChanges { target: mapPageForm_orientationCircleForm; state: "nothing" }
            PropertyChanges { target: mapPageForm_buttonNext; enabled: mapInfo.trajectoryGoalPose.length === 0 ? 0 : 1; opacity: mapInfo.trajectoryGoalPose.length === 0 ? 0 : 1 }
            PropertyChanges { target: mapPage; mapPageForm_nextState_text: qsTr("Start") }
            PropertyChanges { target: mapPageForm_buttonPrevious; enabled: true; opacity: 1; text: qsTr("Anterior") }
            PropertyChanges { target: mp_map; map_currentState: "map_PathGoalPose" }
            PropertyChanges { target: mapPageForm_boxImages; state: "bi_nothing" }
            PropertyChanges {
                target: infoForm
                textInfo.text:     "<h2>Información de la pantalla</h2>
                                   <p>Tras selecionar la posición objetivo se esta creando la trayectoria hacia el objetivo. Cuando la trayectoria se pinta en pantalla podrá seguir con el comienzo del movimiento</p>

                                   <h2>¿Necesita ayuda?</h2>
                                   <p>Para asistencia técnica, contacte con ETSIDI en:
                                   <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
            }
        },
        State {
            name: "mp_GoalPoseMove"
            StateChangeScript { script: console.log("estoy en mp_GoalPoseMove") }
            StateChangeScript { script: mp_map.canvas.requestPaint() }
            PropertyChanges { target: mapPage; mapPageForm_previousState: "mp_PathGoalPose"; mapPageForm_nextState: "mp_selectNextStep" }
            PropertyChanges { target: mapPageForm_text; text: qsTr("Moviendo") }
            PropertyChanges { target: mapPageForm_orientationCircleForm; state: "nothing" }
            PropertyChanges { target: mapPageForm_buttonNext; enabled: true; opacity: 1 }
            PropertyChanges { target: mapPage; mapPageForm_nextState_text: qsTr("Siguiente+") }
            PropertyChanges { target: mapPageForm_buttonPrevious; enabled: false; opacity: 0 }
            PropertyChanges { target: mp_map; map_currentState: "map_GoalPoseMove" }
            PropertyChanges { target: mapPageForm_boxImages; state: "bi_nothing" }
            PropertyChanges {
                target: infoForm
                textInfo.text:     "<h2>Información de la pantalla</h2>
                                   <p>El robot se esta moviendo hacia la posición objetivo</p>

                                   <h2>¿Necesita ayuda?</h2>
                                   <p>Para asistencia técnica, contacte con ETSIDI en:
                                   <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
            }
        },
        State {
            name: "mp_drawPath"
            StateChangeScript { script: console.log("estoy en mp_drawPath") }
            StateChangeScript { script: mp_map.canvas.requestPaint() }
            PropertyChanges { target: mapPage; mapPageForm_previousState: "selectAction"; mapPageForm_nextState: "mp_drawPathMove" }
            PropertyChanges { target: mapPageForm_text; text: qsTr("Dibuja la trayectoria que quieras realizar") }
            PropertyChanges { target: mapPageForm_orientationCircleForm; state: "nothing" }
            PropertyChanges { target: mapPageForm_buttonNext; enabled: true; opacity: 1 }
            PropertyChanges { target: mapPage; mapPageForm_nextState_text: qsTr("Start") }
            PropertyChanges { target: mapPageForm_buttonPrevious; enabled: true; opacity: 1; text: qsTr("Anterior") }
            PropertyChanges { target: mp_map; map_currentState: "map_drawPath" }
            PropertyChanges { target: mapPageForm_boxImages; state: "bi_draw_path" }
            PropertyChanges {
                target: infoForm
                textInfo.text:     "<h2>Información de la pantalla</h2>
                                   <p>En esta pantalla dibujara la trayectoria que quiere que siga el robot.</p>

                                   <h2>Botones</h2>
                                   <ul>
                                        <li><b>Dibujar:</b> A mano alzada dibuja la trajectoria que el robot segirá.</li>
                                        <li><b>Borrar:</b> Borrará la trayectoria dibujada.</li>
                                        <li><b>Trayectoria final:</b> Se hace un filtrado de la trajectoria.</li>
                                        <li><b>Verificar:</b> Verificas que la trayectoria no pasa cerca de una zona de color negro.</li>
                                   </ul>

                                   <h2>¿Necesita ayuda?</h2>
                                   <p>Para asistencia técnica, contacte con ETSIDI en:
                                   <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
            }

        },
        State {
            name: "mp_drawPathMove"
            StateChangeScript { script: console.log("estoy en mp_drawPathMove") }
            StateChangeScript { script: mp_map.canvas.requestPaint() }
            PropertyChanges { target: mapPage; mapPageForm_previousState: "mp_drawPath"; mapPageForm_nextState: "mp_selectNextStep" }
            PropertyChanges { target: mapPageForm_text; text: qsTr("Moviendo") }
            PropertyChanges { target: mapPageForm_orientationCircleForm; state: "nothing" }
            PropertyChanges { target: mapPageForm_buttonNext; enabled: true; opacity: 1 }
            PropertyChanges { target: mapPage; mapPageForm_nextState_text: qsTr("Siguiente+") }
            PropertyChanges { target: mapPageForm_buttonPrevious; enabled: false; opacity: 0}
            PropertyChanges { target: mp_map; map_currentState: "map_drawPathMove" }
            PropertyChanges { target: mapPageForm_boxImages; state: "bi_nothing" }
            PropertyChanges {
                target: infoForm
                textInfo.text:     "<h2>Información de la pantalla</h2>
                                   <p>El robot se esta moviendo siguiendo la trajectoria dibujada</p>

                                   <h2>¿Necesita ayuda?</h2>
                                   <p>Para asistencia técnica, contacte con ETSIDI en:
                                   <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
            }
        },
        State {
            name: "mp_selectNextStep"
            StateChangeScript { script: console.log("estoy en selectAction") }
            StateChangeScript { script: mp_map.canvas.requestPaint() }
            PropertyChanges { target: mapPage; mapPageForm_previousState: ""; mapPageForm_nextState: "" }
            PropertyChanges { target: mapPageForm_text; text: qsTr("Seleccione la acción que quiere realizar a continuación:") }
            PropertyChanges { target: mapPageForm_orientationCircleForm; state: "nothing" }
            PropertyChanges { target: mapPageForm_buttonNext; enabled: false; opacity: 0 }
            PropertyChanges { target: mapPageForm_buttonPrevious; enabled: false; opacity: 0;}
            PropertyChanges { target: mp_map; map_currentState: "map_selectNextStep" }
            PropertyChanges { target: mapPageForm_boxImages; state: "bi_select_action_next_step" }
            PropertyChanges {
                target: infoForm
                textInfo.text:     "<h2>Información de la pantalla</h2>
                                   <p>En esta pantalla debe elegir que haces a continuación.</p>

                                   <h2>Botones</h2>
                                   <ul>
                                       <li><b>Menú principal:</b> Te dirigiras al menú principal de la apliación.</li>
                                       <li><b>Control Manual:</b> Movieras el robot de forma manual en la pantalla control manual.</li>
                                   </ul>

                                   <h2>¿Necesita ayuda?</h2>
                                   <p>Para asistencia técnica, contacte con ETSIDI en:
                                   <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p><br><br><br>"
            }
        }
    ]
}
