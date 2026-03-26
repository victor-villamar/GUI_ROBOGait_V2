# Command Executor Node

<!-- TABLA DE CONTENIDOS -->
<details>
    <summary><h2>Tabla de contenidos</h2></summary>
    <ol>
        <li><a href="#descripción">Instalación</a></li>
        <li><a href="#organizacion-del-nodo">Organización del nodo</a>
        <li><a href="#command-executor">Command Executor</a></li>
        <li><a href="#command-executor-msgs">Command Executor msgs</a></li>
        <li><a href="#dependecias">Dependencias</a></li>
        <li><a href="#lanzamiento-del-nodo">Lanzamiento del nodo</a></li>
    </ol>
</details>

<!-- DESCRIPCION -->
## Descripción

Este nodo permite la ejecución de commandos dentro de una whitelist desde la GUI. De esta forma la GUI es capaz de poder lanzar o detener procesos del robot (ej: navegación, mapeo, etc).

<!-- ORGANIZACION DEL NODO -->
## Organización del nodo

El nodo esta organizado en dos módulos.

* ***command_executor*** &rarr; Módulo principal con toda la lógica de negocio y comunicación.

* ***command_executor_msgs*** &rarr; Módulo con los mensajes y servicios necesarios para la comunicación.

<!-- COMMAND EXECUTOR -->
## Command Executor

Este módulo tiene las siguientes responsabilidades.

* Lectura de los comandos de bash que esta permitido su ejecución en el sistema &rarr; **YamlLoader**

* Gestión del ciclo de vida de los procesos activos, incluyendo su registro, eliminación, activación y detención &rarr; **ProcessManager**

* Creación de los servicios de ROS expuestos a la interfaz y la publicación constante del status del robot como hearthbeat &rarr; **CommandExecutor**

>[!NOTE]
>
>Unicamente se permiten comando con `ros2`. Hay casos especiales donde se ha mapeado comandos especiales como `rm` para tener el control total de lo que se realiza y no permitir comportamientos no esperados

<!-- COMMAND EXECUTOR MSGS -->
## Command Executor msgs

Este módulo esta dividido en dos:

* ***msg*** &rarr; Contiene el mensaje de `RobotStatus`

    Este mensaje tiene la siguiente información:

    * id &rarr; Identificador único del robot

    * battery &rarr; Nivel de batería del robot

    * ns &rarr; Namespace del robot

    * version &rarr; version del software del robot

    * hardware_id &rarr; Identificador del hardware del robot

    * serial_number &rarr; Numero de serie del robot

* ***srv*** &rarr; Contiene los servicios expuesto a la interfaz

    Estos servicios son los siguientes:

    * Cmd &rarr; Permite ejecutar comandos en el sistema.

    * GetMapData &rarr; Permite obtener los datos raw del mapa creado durante el mape.

    >[!WARNING]
    >
    >Es necesario la creacion de la carpetas maps en el robot. Ya que es en ese directorio donde se guardaran los mapa creados durante el mapeo mediante el nodo `map_saver_cli`. Debe encontranse en `$HOME/maps/`

<!-- DEPENDENCIAS -->
## Dependencias

Las dependencias de este nodo son las siguientes:

* ament_cmake_ros

    ```bash
    sudo apt install -y ros-${ROS_DISTRO}-ament-cmake
    ```

* rclcpp

    ```bash
    sudo apt install -y ros-${ROS_DISTRO}-rclcpp 
    ```

* ament_index_cpp

    ```bash
    sudo apt install -y ros-${ROS_DISTRO}-ament-index-cpp
    ```

* rosidl_default_generators

    ```bash
    sudo apt install -y ros-${ROS_DISTRO}-rosidl-default-generators
    ```

* Boost

    ```bash
    sudo apt install -y libboost-all-dev
    ```

* yaml-cpp

    ```bash
    sudo apt install -y libyaml-cpp-dev
    ```

<!-- LANZAMIENTO DEL NODO -->
## Lanzamiento del nodo

El nodo puede lanzarse de la siguiente forma con o sin namespace

* Namespace

    ```bash
    ros2 run command_executor command_executor_node --ros_args --remap __ns:=/namespace-deseado
    ```

* Sin namespace

    ```bash
    ros2 run command_executor command_executor_node
    ```
