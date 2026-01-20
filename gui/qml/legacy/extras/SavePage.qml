import QtQuick 2.15

SavePageForm {

    property string currentState: "sp_save_with_not_save_button"
    state: currentState

    states: [
        State {
            name: "sp_save_with_not_save_button"
            // StateChangeScript {
            //     script: {
            //         console.log("estoy en saliendo por eso me pregunta que hacer si guardar sin cambios")
            //     }
            // }
            PropertyChanges {
                target: text_header
                text: "¿Guardar los cambios en este archivo?"
            }
            PropertyChanges {
                target: no_save
                visible: true
                opacity: 1

            }
        },
        State {
            name: "sp_save_without_not_save_button"
            // StateChangeScript {
            //     script: {
            //         console.log("estoy en guardar")
            //     }
            // }
            PropertyChanges {
                target: text_header
                text: "Guardar este mapa"
            }
            PropertyChanges {
                target: no_save
                visible: false
                opacity: 0

            }
            PropertyChanges {
                target: save
                anchors.right: cancel.left
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 28
                anchors.rightMargin: 10
            }
        }
    ]
}
