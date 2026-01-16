import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    property alias canvas: canvas
    property alias area: area
    property string map_currentState: map_currentState
    property alias imageDisplay: imageDisplay
    property bool checkPath: false

    ErrorRectangle {
        id: errorPopup
        anchors.centerIn: parent
        errorRectangleTextError.text: "Error: Has puesto el robot en una posicion donde esta prohibido."
    }

    // RowLayout {
    //     z: canvas.z + 1
    //     id: botones
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     Button {
    //         text: 'Clear'
    //         onClicked: canvas.clear()
    //     }
    //     Button {
    //         text: 'Paint'
    //         onClicked: {
    //             mapInfo.setScreenSize(imageDisplay.width, imageDisplay.height)
    //             canvas.enablePainting = true
    //         }
    //     }
    //     Button {
    //         text: 'Exit'
    //     }
    // }

    Rectangle {
        id: mapa
        width: 2 * parent.width / 3
        height: parent.height
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        color: "#518bb7"
        Image {
            id: imageDisplay
            width: Math.min(parent.width,
                            parent.height * imageDisplay.sourceSize.width
                            / imageDisplay.sourceSize.height)
            height: Math.min(parent.height,
                             parent.width * imageDisplay.sourceSize.height
                             / imageDisplay.sourceSize.width)
            anchors.centerIn: parent
            visible: true
            fillMode: Image.PreserveAspectCrop
            source: mapInfo.imgSource
        }
    }

    Canvas {
        id: canvas

        width: imageDisplay.width
        height: imageDisplay.height
        anchors.centerIn: mapa

        property var points: []
        property bool startPointExecuted: false

        property int lastX: -1
        property int lastY: -1
        property bool circleDrawn: false
        property bool enablePainting: false
        property real scale: 0.2

        property var imageRobotWithArrow: Image {
            source: "../../images/robot/robot_with_arrow.png" // Ruta a tu imagen
        }

        property var imageRobotWithArrowGoal: Image {
            source: "../../images/robot/robot_with_arrow_goal.png" // Ruta a tu imagen
        }

        property var imageRobot: Image {
            source: "../../images/robot/robot.png" // Ruta a tu imagen
        }
        property var imageRobotGoal: Image {
            source: "../../images/robot/robot_goal.png" // Ruta a tu imagen
        }

        Connections {
            target: mapInfo
            function onOrientationChanged() {
                canvas.requestPaint();
            }
            function onFinalPathOrientationChanged() {
                canvas.requestPaint();
            }
            function onPositionScreenChanged(){
                canvas.requestPaint();

            }
            function onTrajectoryGoalPoseChanged(){
                canvas.requestPaint();
            }
        }

        function drawAndValidateImage(x, y, orientation, initialOrientation) {
            var ctx = getContext('2d');
            // ctx.clearRect(0, 0, width, height); // Limpiar el Canvas antes de dibujar

            var scaledWidth, scaledHeight;
            if (orientation === 0.0) {
                // scaledWidth = imageRobot.width * scale;
                // scaledHeight = imageRobot.height * scale;
                // ctx.drawImage(
                //             imageRobot,
                //             x - scaledWidth / 2,
                //             y - scaledHeight / 2,
                //             scaledWidth,
                //             scaledHeight
                //             );
                // console.log("Posición: X=" + x + " Y=" + y);

                if (map_currentState === "map_initialPosition")
                {
                                    scaledWidth = imageRobot.width * scale;
                scaledHeight = imageRobot.height * scale;
                ctx.drawImage(
                            imageRobot,
                            x - scaledWidth / 2,
                            y - scaledHeight / 2,
                            scaledWidth,
                            scaledHeight
                            );
                    mapInfo.setPositionScreen(x, y);
                    circleDrawn = true;

                    if (mapInfo.checkPixelBlack(mapInfo.originalPosition.x, mapInfo.originalPosition.y)) {
                        errorPopup.open();
                        console.log("Es negro");
                        clear();
                    } else {
                        console.log("Es blanco");
                    }
                }

                else if (map_currentState === "map_goalPosePosition")
                {
                                    scaledWidth = imageRobotGoal.width * scale;
                scaledHeight = imageRobotGoal.height * scale;
                ctx.drawImage(
                            imageRobotGoal,
                            x - scaledWidth / 2,
                            y - scaledHeight / 2,
                            scaledWidth,
                            scaledHeight
                            );
                    mapInfo.setFinalScreenPosition(x, y);
                    circleDrawn = true;
                    console.log(" lastX " + lastX + " lastY " + lastY  + " mapInfoOriginalX " + mapInfo.finalPathPosition.x+ " mapInfoOriginalY " + mapInfo.finalPathPosition.y)

                    if (mapInfo.checkPixelBlack(mapInfo.finalPathPosition.x, mapInfo.finalPathPosition.y)) {
                        errorPopup.open();
                        console.log("Es negro");
                        clear();
                    } else {
                        console.log("Es blanco");
                    }
                }

            } else {
                var sourceImage;
                if ((map_currentState === "map_goalPoseOrientation" && initialOrientation === 0) || initialOrientation === 0)
                    sourceImage = imageRobotWithArrowGoal;
                else 
                {
                    sourceImage = imageRobotWithArrow;
                }
                scaledWidth = sourceImage.width * scale;
                scaledHeight = sourceImage.height * scale;
                ctx.save(); // Guardar el estado actual del contexto
                ctx.translate(x, y); // Trasladar el origen al punto (x, y)
                ctx.rotate((360 - orientation) * Math.PI / 180); // Rotar el contexto en radianes
                // console.log("Orientacion en MAP.QML " + orientation);
                ctx.drawImage(
                            sourceImage,
                            -scaledWidth / 2, // Ajustar la posición para centrar la imagen
                            -scaledHeight / 2,
                            scaledWidth,
                            scaledHeight
                            );
                ctx.restore();
            }
        }

        function drawPath(ctx) {
            if (points.length > 1) {
                ctx.beginPath();
                ctx.strokeStyle = "black";
                ctx.lineWidth = 2;

                ctx.moveTo(points[0].x, points[0].y);
                for (var i = 1; i < points.length; i++) {
                    ctx.lineTo(points[i].x, points[i].y);
                }
                ctx.stroke();
            }
        }

        function updatePath() {
            if (checkPath === true) {
                points = mapInfo.getPixels();
                requestPaint();  // Forzar repintado después de actualizar puntos
                checkPath = false;
            }
        }

        function drawPointsAndLines(ctx) {
            if (mapInfo.trajectoryGoalPose.length === 0) return;
            ctx.strokeStyle = "blue";
            ctx.lineWidth = 2;
            ctx.beginPath();

            // Comienza desde el primer punto
            var startPoint = mapInfo.trajectoryGoalPose[0];
            ctx.moveTo(startPoint.x, startPoint.y);

            for (var i = 1; i < mapInfo.trajectoryGoalPose.length; i++) {
                var point = mapInfo.trajectoryGoalPose[i];

                // Dibujar línea hacia el siguiente punto
                ctx.lineTo(point.x, point.y);
            }

            ctx.stroke();
        }



        onPaint: {
            var ctx = getContext('2d');
            switch (map_currentState) {
                // Set iniial pose x and y
            case "map_initialPosition":
                console.log("map_initialPosition")
                if(mapInfo.positionScreen.x !== 0 && mapInfo.positionScreen.y !== 0)
                {
                    ctx.clearRect(0, 0, width, height);
                    drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, 0.0, 1);
                }
                else
                {
                    if (!enablePainting || lastX === -1 || lastY === -1 || circleDrawn === true) {
                        return;
                    }
                    ctx.clearRect(0, 0, width, height);
                    drawAndValidateImage(lastX, lastY, 0.0, 1);
                }
                break;
                // Set initial pose yaw
            case "map_initialOrientation":
                // console.log("map_initialOrientation")
                ctx.clearRect(0, 0, width, height);
                // console.log("Orientation: " + mapInfo.orientation);
                mp_map.canvas.enablePainting = false
                drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation, 1);
                break;
                // Select if you want to draw the path or goal pose
            case "map_selectAction":
                console.log("map_selectAction")
                ctx.clearRect(0, 0, width, height);
                drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation, 1);
                break;
                // Set goal pose x and y
            case "map_goalPosePosition":
                //if(mapInfo.finalScreenPosition.x !== 0 && mapInfo.finalScreenPosition.y !== 0 && mapInfo.finalScreenPosition.x !== mapInfo.positionScreen.x && mapInfo.finalScreenPosition.y !== mapInfo.positionScreen.y)
                if(mapInfo.finalScreenPosition.x !== 0 && mapInfo.finalScreenPosition.y !== 0)
                {
                    ctx.clearRect(0, 0, width, height);
                    console.log("finalScreenPosition.x: " + mapInfo.finalScreenPosition.x+ " mapInfo.finalScreenPosition.y " + mapInfo.finalScreenPosition.y + " mapInfo.positionScreen.x " + mapInfo.positionScreen.x  + " mapInfo.positionScreen.y " + mapInfo.positionScreen.y)
                    drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation, 1);
                    drawAndValidateImage(mapInfo.finalScreenPosition.x, mapInfo.finalScreenPosition.y, 0.0, 1);
                }
                else {
                    console.log("map_goalPosePosition")
                    console.log("enablePainting: " + enablePainting + " lastX " + lastX + " lastY " + lastY  + " circleDrawn " + circleDrawn)
                    ctx.clearRect(0, 0, width, height);
                    drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation, 1);
                    circleDrawn = false
                    if (!enablePainting || lastX === -1 || lastY === -1 || circleDrawn === true) {
                        return;
                    }
                    drawAndValidateImage(lastX, lastY, 0.0, 0);
                }

                break;
                // set goal pose yaw
            case "map_goalPoseOrientation":
                // console.log("map_goalPoseOrientation")
                ctx.clearRect(0, 0, width, height);
                mp_map.canvas.enablePainting = false
                drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation, 1); // redibujo el robot inicial
                drawAndValidateImage(mapInfo.finalScreenPosition.x, mapInfo.finalScreenPosition.y, mapInfo.finalPathOrientation, 0); // dibujo el robot final y la orientacion
                break;
                // when you are waiting for the path of goal pose
            case "map_PathGoalPose":
                console.log("map_PathGoalPose")
                ctx.clearRect(0, 0, width, height);
                drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation, 1);
                drawAndValidateImage(mapInfo.finalScreenPosition.x, mapInfo.finalScreenPosition.y, mapInfo.finalPathOrientation, 0); // dibujo el robot final y la orientacion
                drawPointsAndLines(ctx)
                break;
                // Show the robot moving
            case "map_GoalPoseMove":
                console.log("map_GoalPoseMove")
                ctx.clearRect(0, 0, width, height);
                drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation, 1);
                drawAndValidateImage(mapInfo.finalScreenPosition.x, mapInfo.finalScreenPosition.y, mapInfo.finalPathOrientation, 0); // dibujo el robot final y la orientacion
                drawPointsAndLines(ctx)
                break;
                // Draw the trayectory
            case "map_drawPath":
                // console.log("map_drawPath")
                ctx.clearRect(0, 0, width, height);
                drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation, 1);
                drawPath(ctx);
                break;
                // Show the robot moving
            case "map_drawPathMove":
                // console.log("map_drawPathMove")
                ctx.clearRect(0, 0, width, height);
                drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation, 1);
                drawPath(ctx);
                break;
            case "map_selectNextStep":
                ctx.clearRect(0, 0, width, height);
                drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation, 1);
                drawPath(ctx);

                break;
            default:
                console.log("Estado no reconocido: ", map_currentState);
                break;
            }
        }

        MouseArea {
            id: area
            anchors.fill: parent
            onPressed: (mouse) =>  {
                           if(map_currentState === "map_drawPath" && canvas.enablePainting)
                           {
                               if (canvas.startPointExecuted === false) {
                                   // Dibuja línea desde el último punto guardado hasta el nuevo punto inicial
                                   canvas.points = [{x: mapInfo.positionScreen.x, y: mapInfo.positionScreen.y}]
                               } else {
                                   // Si no hay último punto, inicia con el punto actual
                                   canvas.points = [{x: mouseX, y: mouseY}]
                               }
                               canvas.requestPaint()
                           }
                           else{
                               if (canvas.enablePainting) {
                                   canvas.lastX = mouseX;
                                   canvas.lastY = mouseY;
                                   canvas.requestPaint();
                               }
                           }
                       }
            onPositionChanged: (mouse) => {
                                   if(map_currentState === "map_drawPath" && canvas.enablePainting)
                                   {
                                       canvas.points.push({x: mouseX, y: mouseY})
                                       mapInfo.addInfoImageOriginal(mouseX, mouseY)
                                       canvas.requestPaint()
                                   }
                               }
            onReleased: {
                canvas.enablePainting = false;  // Desactiva la pintura cuando se suelta el clic
            }
        }


        function clear() {
            var ctx = getContext('2d');
            ctx.clearRect(0, 0, width, height);
            lastX = -1;
            lastY = -1;
            circleDrawn = false;
            if (map_currentState === "map_initialPosition")
                mapInfo.setPositionScreen(0, 0);
            else if (map_currentState === "map_goalPosePosition")
            {
                mapInfo.setFinalScreenPosition(0, 0);
                // drawAndValidateImage(mapInfo.positionScreen.x, mapInfo.positionScreen.y, mapInfo.orientation);
            }
            else if(map_currentState === "map_GoalPoseMove")
            {
                mapInfo.setFinalScreenPosition(0, 0);
            }
            else if(map_currentState === "map_drawPathMove")
            {
                canvas.points = []
            }
            canvas.requestPaint();
        }

        function clear_internal() {
            var ctx = getContext('2d');
            ctx.clearRect(0, 0, width, height);
            lastX = -1;
            lastY = -1;
            circleDrawn = false;
            if (map_currentState === "map_drawPath")
            {
                canvas.points = []
            }
            canvas.requestPaint();
        }
    }
}
