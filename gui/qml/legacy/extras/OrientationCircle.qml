import QtQuick 2.15

OrientationCircleForm {
    id: orientationCircle
    state: "nothing"


    mouseAreaCircle.onReleased: {
        if (state === "orientacion_inicial") {
            mapInfo.setOrientation(orientation);
            // console.log("Círculo orientación inicial liberado: " + mapInfo.orientation);
        } else if (state === "orientacion_final") {
            mapInfo.setFinalPathOrientation(orientation);
            // console.log("Círculo orientación final liberado: " + mapInfo.finalPathOrientation);
        }
    }
    mouseAreaCircle.onClicked: {
        if (state === "orientacion_inicial") {
            mapInfo.setOrientation(orientation);
            // console.log("Círculo orientación inicial liberado: " + mapInfo.orientation);
        } else if (state === "orientacion_final") {
            mapInfo.setFinalPathOrientation(orientation);
            // console.log("Círculo orientación final liberado: " + mapInfo.finalPathOrientation);
        }
    }
    mouseAreaCircle.onPositionChanged: {
        if (state === "orientacion_inicial") {
            mapInfo.setOrientation(orientation);
            // console.log("Círculo orientación inicial liberado: " + mapInfo.orientation);
        } else if (state === "orientacion_final") {
            mapInfo.setFinalPathOrientation(orientation);
            // console.log("Círculo orientación final liberado: " + mapInfo.finalPathOrientation);
        }
    }

    states: [
        State {
            name: "nothing"
            PropertyChanges {
                target: orientationCircle_rectangle
                visible: false
            }
        },
        State {
            name: "orientacion_inicial"
            PropertyChanges {
                target: orientationCircle_rectangle
                visible: true
            }
        },
        State {
            name: "orientacion_final"
            PropertyChanges {
                target: orientationCircle_rectangle
                visible: true
            }
        }
    ]
}
