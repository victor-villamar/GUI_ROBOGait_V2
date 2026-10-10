# Nav2 messages compatibility

<!-- TABLA DE CONTENIDOS -->
<details open>
    <summary><h2>Tabla de contenidos</h2></summary>
    <ol>
        <li><a href="#descripción">Descripción</a></li>
        <li><a href="#motivo-del-paquete">Motivo del paquete</a></li>
        <li><a href="#estructura-del-paquete">Estructura del paquete</a></li>
        <li><a href="#interfaces-ros-2">Interfaces ROS 2</a></li>
        <li><a href="#compatibilidad-entre-distribuciones">Compatibilidad entre distribuciones</a></li>
        <li><a href="#dependencias">Dependencias</a></li>
        <li><a href="#compilación-y-validación">Compilación y validación</a></li>
    </ol>
</details>

<!-- DESCRIPCIÓN -->
## Descripción

`nav2_msgs` es un paquete local de compatibilidad que contiene el subconjunto de interfaces de Navigation2 utilizado por `robogait_gui`. Las definiciones proceden de `nav2_msgs` 1.3.12 de ROS 2 Jazzy y se generan con las herramientas ROSIDL de Lyrical.

Su finalidad es que la GUI ejecutada sobre ROS 2 Lyrical conserve el contrato DDS de Jazzy al comunicarse con el robot que ejecuta Nav2 Jazzy. No contiene nodos ni bibliotecas de Navigation2 y no recompila el stack Nav2 completo.

<!-- MOTIVO DEL PAQUETE -->
## Motivo del paquete

Aunque los nombres de las acciones se mantienen entre Jazzy y Lyrical, algunas de sus definiciones han cambiado:

|        Acción       |                  Cambio introducido en Lyrical                  |
| ------------------- | --------------------------------------------------------------- |
| `ComputePathToPose` |                    Añade `viapoints` al goal                    |
|   `NavigateToPose`  |   Añade campos de error de posición y orientación al feedback   |
|     `FollowPath`    | Añade `path_handler_id` al goal y utiliza un feedback diferente |

Una GUI compilada directamente contra `nav2_msgs` de Lyrical puede descubrir los servidores de acción Jazzy, pero no serializa ni deserializa correctamente esos mensajes. El paquete local conserva el nombre ROS `nav2_msgs` y las definiciones Jazzy para mantener la misma identidad y estructura de tipos en ambos extremos.

<!-- ESTRUCTURA DEL PAQUETE -->
## Estructura del paquete

* [`action/`](action/) &rarr; Definiciones de las acciones Nav2 utilizadas por la GUI.
* [`msg/`](msg/) &rarr; Mensajes necesarios para visualizar la nube de partículas.
* [`CMakeLists.txt`](CMakeLists.txt) &rarr; Generación de interfaces y validación de que el entorno activo sea Lyrical.
* [`package.xml`](package.xml) &rarr; Metadatos y dependencias ROS 2.
* [`README.md`](README.md) &rarr; Documentación del paquete de compatibilidad.

<!-- INTERFACES ROS 2 -->
## Interfaces ROS 2

El paquete genera únicamente las interfaces de Jazzy consumidas por `robogait_gui`:

|                            Interfaz                           |                  Uso en la GUI                |
| ------------------------------------------------------------- | --------------------------------------------- |
| [`ComputePathToPose.action`](action/ComputePathToPose.action) |  Solicitud y recepción de una ruta calculada  |
|    [`NavigateToPose.action`](action/NavigateToPose.action)    |       Navegación directa hasta una pose       |
|        [`FollowPath.action`](action/FollowPath.action)        |     Seguimiento de una trayectoria manual     |
|               [`Particle.msg`](msg/Particle.msg)              |          Pose y peso de una partícula         |
|          [`ParticleCloud.msg`](msg/ParticleCloud.msg)         | Conjunto de partículas mostrado sobre el mapa |

`Particle` y `ParticleCloud` se incluyen porque el override sustituye el paquete `nav2_msgs` completo, no archivos individuales. Omitirlos mezclaría headers y bibliotecas de tipos del paquete local y del instalado en `/opt/ros`.

<!-- COMPATIBILIDAD ENTRE DISTRIBUCIONES -->
## Compatibilidad entre distribuciones

| Distribución de la GUI |               `nav2_msgs` utilizado               |
| ---------------------- | ------------------------------------------------- |
|         Humble         |      Paquete del sistema en `/opt/ros/humble`     |
|          Jazzy         |       Paquete del sistema en `/opt/ros/jazzy`     |
|         Lyrical        | Paquete local generado con las definiciones Jazzy |

Los scripts de ROBOGait seleccionan este paquete solo cuando `ROS_DISTRO=lyrical`. En Humble y Jazzy lo excluyen explícitamente para conservar el paquete oficial de la distribución.

> [!WARNING]
>
>El overlay Lyrical resultante está destinado a una GUI que se comunica con un robot Jazzy. No debe utilizarse para ejecutar nodos Nav2 Lyrical, porque el paquete local sustituye deliberadamente sus interfaces por las de Jazzy.

<!-- DEPENDENCIAS -->
## Dependencias

La generación utiliza:

* `ament_cmake`
* `rosidl_default_generators`
* `rosidl_default_runtime`
* `action_msgs`
* `builtin_interfaces`
* `geometry_msgs`
* `nav_msgs`
* `std_msgs`

<!-- COMPILACIÓN Y VALIDACIÓN -->
## Compilación y validación

La instalación normal mediante [`install.sh`](../../install.sh) y la construcción Docker seleccionan automáticamente el paquete en Lyrical. Para compilarlo manualmente:

```bash
source /opt/ros/lyrical/setup.bash
colcon build --packages-select nav2_msgs
source install/setup.bash
```

El prefijo activo y las definiciones generadas pueden verificarse con:

```bash
ros2 pkg prefix nav2_msgs
ros2 interface show nav2_msgs/action/ComputePathToPose
ros2 interface show nav2_msgs/action/NavigateToPose
ros2 interface show nav2_msgs/action/FollowPath
ros2 interface show nav2_msgs/msg/ParticleCloud
```

En la imagen Docker Lyrical, `ros2 pkg prefix nav2_msgs` debe devolver `/opt/ros/robogait`.

Las interfaces proceden de Navigation2 Jazzy
