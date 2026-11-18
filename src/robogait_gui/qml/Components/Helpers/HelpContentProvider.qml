import QtQuick 2.15
import AppTheme 1.0

QtObject {
    function mainMenuMessage() {
        return qsTr(
            "<h2>Objetivo de la aplicación</h2>"
            + "<p>Esta aplicación permite controlar el robot <b>ROBOGait</b> y realizar pruebas de la marcha humana para la evaluación de la movilidad.<br>"
            + "Su uso facilita la detección de posibles alteraciones en la marcha y contribuye al diagnóstico clínico.</p>"
            + "<h2>Cómo empezar</h2>"
            + "<ol>"
            + "<li><b>Seleccionar mapa</b> – Defina el entorno en el que se realizará la evaluación del paciente.</li>"
            + "<li><b>Seleccionar paciente</b> – Asigne un paciente para registrar y analizar su patrón de marcha.</li>"
            + "<li><b>Control manual</b> – Permite un manejo directo del robot para ajustes o pruebas preliminares.</li>"
            + "<li><b>Prueba</b> – Inicia el análisis de la marcha del paciente con el robot.</li>"
            + "</ol>"
            + "<p>La selección del mapa y el paciente es obligatoria antes de iniciar una prueba.<br>"
            + "Los nombres seleccionados aparecerán en la parte inferior de la pantalla.</p>"
            + "<h2>¿Necesita ayuda?</h2>"
            + "<p>Para asistencia técnica, contacte con la ETSIDI en: "
            + "<a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p>"
        )
    }

    function mapViewMessage() {
        return qsTr(
            "<h2>Visualización del Mapa</h2>"
            + "<p>Esta pantalla muestra el mapa generado por SLAM y permite controlar el robot manualmente.</p>"
            + "<h2>Requisitos</h2>"
            + "<ul>"
            + "<li><b>Robot seleccionado:</b> Debe haber un robot activo en el sistema.</li>"
            + "<li><b>SLAM activo:</b> El robot debe estar ejecutando un nodo de SLAM (cartographer, slam_toolbox, etc.).</li>"
            + "<li><b>Topic /map:</b> El robot debe publicar el mapa en el topic estándar.</li>"
            + "</ul>"
            + "<h2>Desbloqueo del joystick</h2>"
            + "<p>Pulse el candado para desbloquear el joystick.</p>"
            + "<h2>Movimiento del joystick</h2>"
            + "<p>Arrastre el joystick para mover el robot (arriba=adelante, abajo=atrás, laterales=giro). "
            + "La distancia desde el centro determina la velocidad de movimiento.</p>"
            + "<h2>Bloqueo del joystick</h2>"
            + "<p>Pulse nuevamente el candado para bloquear el joystick.</p>"
            + "<h2>Estados del mapa</h2>"
            + "<ul>"
            + "<li><b>Ocupado (negro):</b> Obstáculo detectado.</li>"
            + "<li><b>Libre (blanco):</b> Espacio navegable.</li>"
            + "<li><b>Desconocido (gris):</b> Área no explorada.</li>"
            + "</ul>"
            + "<h2><span style='color:" + AppTheme.map.warningTitle + "'>¡¡ADVERTENCIA!!</span></h2>"
            + "<p>Este robot no cuenta con sistema de gestión de colisiones. Supervise el desplazamiento en todo momento "
            + "y evite obstáculos.</p>"
            + "<h2>¿Necesita ayuda?</h2>"
            + "<p>Para asistencia técnica, contacte con ETSIDI en: "
            + "<a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p>"
        )
    }

    function manualControlMessage() {
        return qsTr(
            "<h2>Control Manual del Robot</h2>"
            + "<p>Esta pantalla permite controlar el robot de forma manual utilizando el joystick virtual.</p>"
            + "<h2>Desbloqueo del joystick</h2>"
            + "<p>Pulse el candado a la derecha del joystick para desbloquearlo.</p>"
            + "<h2>Movimiento del joystick</h2>"
            + "<p>Una vez desbloqueado, desplace el círculo pequeño para comenzar a moverse. La velocidad de movimiento aumentará cuanto más se aleje del centro.</p>"
            + "<ul>"
            + "<li><b>Hacia adelante:</b> Avanza en línea recta.</li>"
            + "<li><b>Hacia atrás:</b> Retrocede en línea recta.</li>"
            + "<li><b>Hacia la izquierda:</b> Gira en sentido horario (hacia la derecha).</li>"
            + "<li><b>Hacia la derecha:</b> Gira en sentido antihorario (hacia la izquierda).</li>"
            + "<li><b>Posiciones intermedias:</b> El robot se moverá en una combinación de traslación y giro, ajustando su dirección según la inclinación del joystick.</li>"
            + "</ul>"
            + "<h2>Bloqueo del joystick</h2>"
            + "<p>Pulse el candado a la derecha del joystick para bloquearlo.</p>"
            + "<h2><span style='color:" + AppTheme.manualControl.warningTitle + "'>¡¡ADVERTENCIA!!</span></h2>"
            + "<p>Este robot no cuenta con un sistema de gestión de colisiones. Asegúrese de evitar obstáculos y supervisar su desplazamiento en todo momento.</p>"
            + "<h2>¿Necesita ayuda?</h2>"
            + "<p>Para asistencia técnica, contacte con ETSIDI en: "
            + "<a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p>"
        )
    }

    function testMapPositionMessage() {
        return qsTr("<h2>Resumen</h2>")
                + qsTr("<p>Colocación de la posición inicial del robot en el mapa seleccionado para el experimento.</p>")
                + qsTr("<h2>Formas de posicionar el robot</h2>")
                + qsTr("<ul>"
                       + "<li><b>Manual:</b> pulse <b>HABILITAR COLOCACIÓN</b> y toque el punto del mapa donde está el robot.</li>"
                       + "<li><b>Autolocalización:</b> pulse <b>AUTOLOCALIZAR</b>. El robot se moverá para localizarse automáticamente en el mapa usando el algoritmo <b>AMCL</b>.</li>"
                       + "</ul>")
                + qsTr("<h2>Botones</h2>")
                + qsTr("<ul>"
                       + "<li><b>HABILITAR COLOCACIÓN:</b> activa la colocación manual en el mapa.</li>"
                       + "<li><b>BORRAR:</b> elimina la posición actual para volver a colocarla.</li>"
                       + "<li><b>AUTOLOCALIZAR:</b> inicia la localización automática.</li>"
                       + "</ul>")
                + qsTr("<h2>Seguridad</h2>")
                + qsTr("<p>En cualquier momento puede pulsar <b>Emergency Stop</b> para realizar una parada de emergencia si es necesario.</p>")
                + qsTr("<h2>¿Necesita ayuda?</h2>")
                + qsTr("<p>Para asistencia técnica, contacte con ETSIDI en: <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p>")
    }

    function testMapOrientationMessage() {
        return qsTr("<h2>Resumen</h2>")
                + qsTr("<p>Ajuste de la orientación del robot para que coincida con la orientación real en el entorno.</p>")
                + qsTr("<h2>Cómo orientar el robot</h2>")
                + qsTr("<p>Use la <b>rueda de orientación</b> para girar la dirección del robot en el mapa hasta que coincida con la realidad.</p>")
                + qsTr("<p>Si llega desde la autolocalización, normalmente el robot ya queda orientado. Si el robot se ha girado manualmente, puede volver a orientarlo con la rueda.</p>")
                + qsTr("<h2>Controles</h2>")
                + qsTr("<ul>"
                       + "<li><b>HABILITAR ORIENTACIÓN:</b> activa el ajuste de orientación.</li>"
                       + "<li><b>RUEDA DE ORIENTACIÓN:</b> permite definir el ángulo final del robot.</li>"
                       + "<li><b>ATRÁS:</b> vuelve a la fase de posición.</li>"
                       + "</ul>")
                + qsTr("<h2>Seguridad</h2>")
                + qsTr("<p>En cualquier momento puede pulsar <b>Emergency Stop</b> para realizar una parada de emergencia si es necesario.</p>")
                + qsTr("<h2>¿Necesita ayuda?</h2>")
                + qsTr("<p>Para asistencia técnica, contacte con ETSIDI en: <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p>")
    }

    function testMapTrajectoryMessage() {
        return qsTr("<h2>Resumen</h2>")
                + qsTr("<p>Esta fase sirve para definir la trayectoria que seguirá el robot durante el experimento.</p>")
                + qsTr("<h2>Tipos de trayectoria</h2>")
                + qsTr("<ul>"
                       + "<li><b>Ir a un punto:</b> seleccione el botón de objetivo, coloque el punto final y ajuste su orientación.</li>"
                       + "<li><b>Describir una ruta:</b> seleccione el botón de ruta y dibuje el recorrido sobre el mapa.</li>"
                       + "</ul>")
                + qsTr("<h2>Modo punto</h2>")
                + qsTr("<ul>"
                       + "<li><b>BORRAR:</b> elimina el punto seleccionado para definir otro.</li>"
                       + "<li><b>CALCULAR:</b> calcula la trayectoria de navegación hasta el objetivo.</li>"
                       + "</ul>")
                + qsTr("<h2>Modo ruta</h2>")
                + qsTr("<ul>"
                       + "<li><b>SUAVIZAR:</b> segmenta y suaviza la ruta dibujada para preparar el cálculo.</li>"
                       + "<li><b>CALCULAR:</b> calcula la trayectoria de navegación a partir de la ruta suavizada.</li>"
                       + "<li><b>BORRAR:</b> elimina la ruta dibujada para volver a crearla.</li>"
                       + "</ul>")
                + qsTr("<h2>Siguiente paso</h2>")
                + qsTr("<p>Tras calcular correctamente la trayectoria, se habilita el botón <b>INICIAR TEST</b> para avanzar a la siguiente fase.</p>")
                + qsTr("<h2>Seguridad</h2>")
                + qsTr("<p>En cualquier momento puede pulsar <b>Emergency Stop</b> para realizar una parada de emergencia si es necesario.</p>")
                + qsTr("<h2>¿Necesita ayuda?</h2>")
                + qsTr("<p>Para asistencia técnica, contacte con ETSIDI en: <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p>")
    }

    function testMapExperimentMessage() {
        return qsTr("<h2>Resumen</h2>")
                + qsTr("<p>Esta fase ejecuta el experimento con la trayectoria ya calculada y permite supervisar la navegación del robot.</p>")
                + qsTr("<h2>Controles de vista</h2>")
                + qsTr("<ul>"
                       + "<li><b>- / +:</b> reduce o aumenta el zoom del mapa.</li>"
                       + "<li><b>Centrar robot:</b> activa o desactiva el seguimiento del robot en pantalla.</li>"
                       + "<li><b>Ajustar vista:</b> encuadra el mapa completo en la pantalla.</li>"
                       + "</ul>")
                + qsTr("<h2>Botones de experimento</h2>")
                + qsTr("<ul>"
                       + "<li><b>TERMINAR EXPERIMENTO:</b> cancela la navegación y abre el flujo de cierre del experimento.</li>"
                       + "<li><b>REPETIR EXPERIMENTO:</b> aparece al finalizar/cancelar para volver a la fase de cálculo de trayectoria.</li>"
                       + "<li><b>HOME:</b> aparece cuando el robot completa la trayectoria para volver al punto inicial.</li>"
                       + "</ul>")
                + qsTr("<h2>Seguridad</h2>")
                + qsTr("<p>En cualquier momento puede pulsar <b>Emergency Stop</b> para realizar una parada de emergencia si es necesario.</p>")
                + qsTr("<h2>¿Necesita ayuda?</h2>")
                + qsTr("<p>Para asistencia técnica, contacte con ETSIDI en: <a href='mailto:correo.soporte@upm.es'>correo.soporte@upm.es</a></p>")
    }

    function defaultStepMessage() {
        return qsTr("<h2>Información</h2><p>Seleccione una fase para ver su ayuda detallada.</p>")
    }
}
