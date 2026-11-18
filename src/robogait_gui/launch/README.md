# Launchers de compatibilidad con RoboMesh

<!-- TABLA DE CONTENIDOS -->
<details open>
    <summary><h2>Tabla de contenidos</h2></summary>
    <ol>
        <li><a href="#descripción">Descripción</a></li>
        <li><a href="#contrato-ros-2-y-tf-de-robomesh">Contrato ROS 2 y TF de RoboMesh</a></li>
        <li><a href="#launchers">Launchers</a></li>
        <li><a href="#flujo-de-ejecución">Flujo de ejecución</a></li>
        <li><a href="#argumentos">Argumentos</a></li>
        <li><a href="#validación-y-diagnóstico">Validación y diagnóstico</a></li>
        <li><a href="#limitaciones">Limitaciones</a></li>
    </ol>
</details>

<!-- DESCRIPCIÓN -->
## Descripción

Esta carpeta contiene launchers auxiliares para ejecutar TurtleBot 4 y Navigation2 en ROS 2 Jazzy utilizando la organización de namespaces, tópicos y frames esperada por RoboMesh.

El bringup original de TurtleBot 4 puede publicar `/tf` y `/tf_static` dentro del namespace del robot mientras conserva identificadores de frame sin prefijo. RoboMesh utiliza el contrato contrario: los tópicos TF son globales y los frames propios de cada robot incluyen su namespace. Estos launchers adaptan la simulación en origen, sin crear un bridge ni duplicar transformaciones.

Son utilidades de desarrollo y no se instalan en `share/robogait_gui`. Los comandos deben apuntar directamente a esta carpeta del repositorio.

<!-- CONTRATO ROS 2 Y TF -->
## Contrato ROS 2 y TF de RoboMesh

Para un robot cuyo namespace sea `robot1`, la disposición resultante es:

|                  Elemento                  |       Nombre       |
|--------------------------------------------|--------------------|
|             Tópico TF dinámico             |        `/tf`       |
|             Tópico TF estático             |    `/tf_static`    |
|                Frame global                |        `map`       |
|             Frame de odometría             |    `robot1/odom`   |
|                  Frame base                | `robot1/base_link` |
|             Frames de sensores             |     `robot1/...`   |
| Nodos, servicios, acciones y otros tópicos |    `/robot1/...`   |

Los frame IDs nunca comienzan por `/`. Los launchers aceptan tanto `robot1` como `/robot1` y normalizan el valor antes de construir los parámetros. Esto evita valores inválidos como `/robot1/base_link`, que TF2 rechaza.

La GUI utiliza siempre esta disposición. No existe una opción de compilación específica para simulación ni es necesario mantener una variante binaria para TurtleBot 4.

<!-- LAUNCHERS -->
## Launchers

|                                 Archivo                                |                                                                              Responsabilidad                                                                              |
|------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`robogait_turtlebot4_gz.launch.py`](robogait_turtlebot4_gz.launch.py) |                  Inicia Gazebo y TurtleBot 4, prefija los frames del modelo, remapea TF a los tópicos globales y carga los nodos y bridges de simulación                  |
| [`robogait_create3_nodes.launch.py`](robogait_create3_nodes.launch.py) | Adapta los nodos y controladores de Create 3 para que el controlador diferencial publique sobre los tópicos TF globales. Es un helper interno del launcher de TurtleBot 4 |
|          [`robogait_slam.launch.py`](robogait_slam.launch.py)          |                                     Inicia SLAM Toolbox síncrono o asíncrono con `map` global, frames prefijados y tópicos TF globales                                    |
|  [`robogait_nav2_bringup.launch.py`](robogait_nav2_bringup.launch.py)  |    Inicia localización y navegación Nav2 dentro del namespace del robot, sustituye `<robot_namespace>` en el YAML y conserva `/tf` y `/tf_static` como tópicos globales   |

El launcher de TurtleBot 4 obliga a mantener `nav2`, `slam` y `localization` en `false`. ROBOGait inicia SLAM o Nav2 de forma separada cuando la GUI solicita el comando correspondiente.

<!-- FLUJO DE EJECUCIÓN -->
## Flujo de ejecución

### 1. Iniciar TurtleBot 4 en Gazebo

```bash
ros2 launch $HOME/GUI_ROBOGait_V2/src/robogait_gui/launch/robogait_turtlebot4_gz.launch.py \
  model:=standard namespace:=robot1 world:=maze \
  x:=-1.0 y:=0.0 yaw:=0.0 \
  nav2:=false slam:=false localization:=false rviz:=false
```

### 2. Crear un mapa

La GUI inicia este launcher mediante el comando `cartographer` del preset de
simulación. También puede ejecutarse manualmente:

```bash
ros2 launch $HOME/GUI_ROBOGait_V2/src/robogait_gui/launch/robogait_slam.launch.py \
  namespace:=robot1 use_sim_time:=true sync:=true
```

### 3. Localizar y navegar sobre un mapa

```bash
ros2 launch $HOME/GUI_ROBOGait_V2/src/robogait_gui/launch/robogait_nav2_bringup.launch.py \
  namespace:=robot1 use_sim_time:=true \
  use_localization:=true autostart:=true \
  map:=$HOME/maps/maze.yaml \
  params_file:=$HOME/.local/robogait/params/nav2_params.yaml
```

El preset
[`commands_jazzy_multirobot.yaml`](../params/simulation/jazzy/commands_jazzy_multirobot.yaml)
realiza estas llamadas automáticamente. Presupone que el repositorio se encuentra
en `$HOME/GUI_ROBOGait_V2`.

<!-- ARGUMENTOS -->
## Argumentos

### TurtleBot 4 y Gazebo

|            Argumento           |     Valor predeterminado     |                            Descripción                            |
|--------------------------------|------------------------------|-------------------------------------------------------------------|
|           `namespace`          |           `robot1`           |              Namespace y prefijo de frames del robot              |
|             `model`            |            `standard`        |                     Modelo `standard` o `lite`                    |
|             `world`            |          `warehouse`         |                      Mundo cargado por Gazebo                     |
|      `x`, `y`, `z`, `yaw`      |              `0.0`           |                       Pose inicial del robot                      |
|          `use_sim_time`        |             `true`           |                Utiliza el reloj publicado por Gazebo              |
|          `param_file`          | Configuración de TurtleBot 4 |                   Parámetros del nodo del robot                   |
|             `rviz`             |            `false`           |          Inicia RViz con la configuración de TurtleBot 4          |
| `nav2`, `slam`, `localization` |            `false`           | Se aceptan por compatibilidad, pero deben permanecer desactivados |

### SLAM Toolbox

|        Argumento        |           Valor predeterminado           |                            Descripción                             |
|-------------------------|------------------------------------------|--------------------------------------------------------------------|
|       `namespace`       |                 `robot1`                 |               Namespace y prefijo de frames del robot              |
|     `use_sim_time`      |                  `true`                  |                   Utiliza el reloj de simulación                   |
|         `sync`          |                  `true`                  | Selecciona el launcher síncrono; con `false` utiliza el asíncrono  |
|       `autostart`       |                  `true`                  |             Activa automáticamente los nodos lifecycle             |
| `use_lifecycle_manager` |                 `false`                  |          Solicita al launcher de SLAM su gestor lifecycle          |
|        `params`         | `turtlebot4_navigation/config/slam.yaml` | Archivo base que se reescribe con los frames y tópicos de ROBOGait |

### Navigation2

|     Argumento      |          Valor predeterminado          |                                 Descripción                                 |
|--------------------|----------------------------------------|-----------------------------------------------------------------------------|
|    `namespace`     |                `robot1`                |                  Namespace normalizado y prefijo de frames                  |
|   `use_sim_time`   |                 `true`                 |                        Utiliza el reloj de simulación                       |
|    `autostart`     |                 `true`                 |            Configura y activa automáticamente los nodos lifecycle           |
|   `params_file`    | `nav2_bringup/params/nav2_params.yaml` | YAML de Nav2; el preset multirobot utiliza placeholders `<robot_namespace>` |
|       `map`        |                  Vacío                 |                     Mapa YAML cargado por `map_server`                      |
| `use_localization` |                 `true`                 |              Inicia `map_server`, AMCL y su lifecycle manager               |
|   `use_respawn`    |                `False`                 |             Reinicia los procesos que terminan inesperadamente              |
|    `log_level`     |                 `info`                 |                       Nivel de log de los nodos Nav2                        |
|  `use_namespace`   |                 `true`                 |    Argumento de compatibilidad; este launcher siempre utiliza namespace     |
|       `slam`       |                `False`                 |    Argumento de compatibilidad; SLAM se inicia con su launcher dedicado     |
| `use_composition`  |                `False`                 | Argumento de compatibilidad; los nodos se ejecutan como procesos separados  |

<!-- VALIDACIÓN Y DIAGNÓSTICO -->
## Validación y diagnóstico

Después de iniciar la simulación, la cadena de odometría debe estar disponible:

```bash
ros2 run tf2_ros tf2_echo robot1/odom robot1/base_link
```

Con SLAM o AMCL activo también debe existir la transformación desde el mapa:

```bash
ros2 run tf2_ros tf2_echo map robot1/base_link
```

Los tópicos TF utilizados por todos los robots pueden comprobarse con:

```bash
ros2 topic info /tf
ros2 topic info /tf_static
```

Para navegación, AMCL y los servidores Nav2 deben estar activos:

```bash
ros2 lifecycle get /robot1/amcl
ros2 lifecycle get /robot1/map_server
ros2 lifecycle get /robot1/controller_server
ros2 lifecycle get /robot1/planner_server
```

Si TF2 muestra `frame_ids cannot start with a '/'`, algún parámetro se generó con
un namespace sin normalizar. Detenga Nav2 y vuelva a iniciarlo con este launcher;
los parámetros se generan durante cada arranque y un proceso ya iniciado no los
actualiza automáticamente.

<!-- LIMITACIONES -->
## Limitaciones

* Estos launchers están destinados a integrar la simulación TurtleBot 4 de ROS 2 Jazzy con RoboMesh.
* No modifican RoboMesh ni sustituyen los launchers utilizados por los robots físicos.
* No crean bridges TF ni republican transformaciones; adaptan los productores originales.
* Cada robot debe utilizar un namespace único.
* Nav2 y SLAM no deben iniciarse simultáneamente para el mismo robot.
* Al ejecutarse desde el código fuente, el repositorio debe conservar la ruta utilizada por los presets de comandos.
