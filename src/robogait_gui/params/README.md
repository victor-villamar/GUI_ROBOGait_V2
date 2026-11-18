# Parámetros de `robogait_gui`

<!-- TABLA DE CONTENIDOS -->
<details open>
    <summary><h2>Tabla de contenidos</h2></summary>
    <ol>
        <li><a href="#descripción">Descripción</a></li>
        <li><a href="#estructura-de-directorios">Estructura de directorios</a></li>
        <li><a href="#archivos-instalados">Archivos instalados</a></li>
        <li><a href="#configuraciones-de-simulación">Configuraciones de simulación</a></li>
        <li><a href="#selección-de-la-configuración">Selección de la configuración</a></li>
        <li><a href="#consideraciones-importantes">Consideraciones importantes</a></li>
    </ol>
</details>

<!-- DESCRIPCIÓN -->
## Descripción

Este directorio contiene la configuración predeterminada de `robogait_gui` y las variantes de referencia utilizadas durante las simulaciones.

Los archivos situados directamente en `params/` representan la configuración predeterminada para trabajar con el robot real. Las variantes incluidas en `params/simulation/` no se instalan durante la compilación; se conservan en el repositorio para seleccionar manualmente el entorno de simulación necesario.

La aplicación utiliza nombres de archivo estables en tiempo de ejecución:

* `config.yaml`

* `commands.yaml`

* `nav2_params.yaml`, cuando Nav2 se ejecuta en el mismo equipo durante una simulación

Por tanto, al seleccionar una variante debe copiarse con uno de esos nombres, aunque el archivo de referencia incluya la distribución ROS 2 o el tipo de simulación en su nombre.

<!-- ESTRUCTURA DE DIRECTORIOS -->
## Estructura de directorios

```text
params/
├── README.md
├── bootstrap.yaml
├── commands.yaml
├── config.yaml
└── simulation/
    ├── humble/
    │   ├── commands.yaml
    │   └── nav2_params.yaml
    └── jazzy/
        ├── commands_jazzy_multirobot.yaml
        ├── config_jazzy_multirobot.yaml
        ├── nav2_params_jazzy.yaml
        └── nav2_params_jazzy_multirobot.yaml
```

<!-- ARCHIVOS INSTALADOS -->
## Archivos instalados

La compilación instala únicamente los siguientes archivos:

| Archivo | Finalidad |
|---------|-----------|
| [`bootstrap.yaml`](bootstrap.yaml) | Define el directorio persistente y los nombres de los archivos de configuración del usuario. |
| [`config.yaml`](config.yaml) | Configuración predeterminada de la interfaz, ROS 2, robot, navegación, mapas y renderizado. |
| [`commands.yaml`](commands.yaml) | Comandos predeterminados de mapeado, guardado de mapas y navegación. |

Los archivos quedan disponibles en el directorio `share` del paquete:

```text
<install-prefix>/share/robogait_gui/params
```

Durante el primer arranque, `bootstrap.yaml` indica que `config.yaml` y `commands.yaml` deben copiarse a:

```text
$HOME/.local/robogait/params
```

La copia se realiza solamente cuando el archivo de destino todavía no existe. Esto permite conservar los ajustes del usuario entre compilaciones e instalaciones.

> [!IMPORTANT]
>
>Una nueva compilación no sobrescribe `config.yaml` ni `commands.yaml` si ya existen en `$HOME/.local/robogait/params`. En ese caso deben actualizarse manualmente en el directorio persistente.

Los archivos `nav2_params*.yaml` no se instalan ni se copian automáticamente. En el robot real, el archivo de Nav2 normalmente reside en el propio robot. En simulación debe copiarse manualmente al equipo donde se ejecuta Nav2.

<!-- CONFIGURACIONES DE SIMULACIÓN -->
## Configuraciones de simulación

### ROS 2 Humble

|                                   Archivo                                  |                                     Finalidad                                     |
|----------------------------------------------------------------------------|-----------------------------------------------------------------------------------|
|    [`simulation/humble/commands.yaml`](simulation/humble/commands.yaml)    | Comandos de simulación de TurtleBot 3 para Cartographer, guardado de mapas y Nav2 |
| [`simulation/humble/nav2_params.yaml`](simulation/humble/nav2_params.yaml) |        Parámetros Nav2 utilizados por la simulación de TurtleBot 3 en Humble      |

### ROS 2 Jazzy

|                                                  Archivo                                                   |                                                               Finalidad                                                    |
|------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|
|       [`simulation/jazzy/config_jazzy_multirobot.yaml`](simulation/jazzy/config_jazzy_multirobot.yaml)     |           Configuración de la GUI para TurtleBot 4 multirobot, incluido el topic de velocidad sin sello temporal           |
|     [`simulation/jazzy/commands_jazzy_multirobot.yaml`](simulation/jazzy/commands_jazzy_multirobot.yaml)   |                           Comandos namespaced para SLAM, guardado de mapas y Nav2 en TurtleBot 4                           |
| [`simulation/jazzy/nav2_params_jazzy_multirobot.yaml`](simulation/jazzy/nav2_params_jazzy_multirobot.yaml) |                               Parámetros Nav2 de TurtleBot 4 reutilizables bajo distintos namespaces                       |
|            [`simulation/jazzy/nav2_params_jazzy.yaml`](simulation/jazzy/nav2_params_jazzy.yaml)            | Configuración Nav2 Jazzy para TurtleBot 3 conservada como referencia para entornos sin el preset multirobot de TurtleBot 4 |

Las configuraciones multirobot utilizan topics relativos y reciben el namespace mediante `ROBOT_NAMESPACE`. Cada proceso `command_executor` debe iniciarse con la variable y el remapeo correspondientes al mismo robot.

<!-- SELECCIÓN DE LA CONFIGURACIÓN -->
## Selección de la configuración

Existen dos formas de seleccionar los parámetros.

### Opción 1: preparar los valores predeterminados antes de compilar

Esta opción modifica qué `config.yaml` y `commands.yaml` se instalan en el directorio `share` del paquete.

Para Jazzy con TurtleBot 4 multirobot:

```bash
cp $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/simulation/jazzy/config_jazzy_multirobot.yaml \
   $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/config.yaml

cp $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/simulation/jazzy/commands_jazzy_multirobot.yaml \
   $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/commands.yaml

colcon build --packages-up-to robogait_gui
```

Para Humble con TurtleBot 3, `config.yaml` mantiene la configuración predeterminada y solamente es necesario sustituir `commands.yaml` antes de compilar:

```bash
cp $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/simulation/humble/commands.yaml \
   $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/commands.yaml

colcon build --packages-up-to robogait_gui
```

El archivo Nav2 sigue necesitando una copia manual porque CMake no instala las variantes `nav2_params*.yaml`. Unicamente es un archivo usado para simulación y no se instala en el robot real.

> [!WARNING]
>
>Esta opción solo inicializa una instalación nueva. Si `$HOME/.local/robogait/params/config.yaml` o `$HOME/.local/robogait/params/commands.yaml` ya existen, la aplicación conserva esos archivos y no utiliza las nuevas copias instaladas.

### Opción 2: cambiar los parámetros persistentes después de compilar

Esta es la opción más directa para alternar entre simuladores sin recompilar. Debe realizarse con la GUI detenida.

Para Humble con TurtleBot 3:

```bash
PARAMS_DIR="$HOME/.local/robogait/params"
mkdir -p "$PARAMS_DIR"

cp $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/config.yaml \
   "$PARAMS_DIR/config.yaml"

cp $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/simulation/humble/commands.yaml \
   "$PARAMS_DIR/commands.yaml"

cp $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/simulation/humble/nav2_params.yaml \
   "$PARAMS_DIR/nav2_params.yaml"
```

Para Jazzy con TurtleBot 4 multirobot:

```bash
PARAMS_DIR="$HOME/.local/robogait/params"
mkdir -p "$PARAMS_DIR"

cp $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/simulation/jazzy/config_jazzy_multirobot.yaml \
   "$PARAMS_DIR/config.yaml"

cp $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/simulation/jazzy/commands_jazzy_multirobot.yaml \
   "$PARAMS_DIR/commands.yaml"

cp $HOME/GUI_ROBOGait_V2/src/robogait_gui/params/simulation/jazzy/nav2_params_jazzy_multirobot.yaml \
   "$PARAMS_DIR/nav2_params.yaml"
```

Después de sustituir los archivos debe reiniciarse la GUI. El nombre y la ubicación elegidos para Nav2 deben coincidir con:

```yaml
navigation:
  nav2_params_path: "/.local/robogait/params/"
  nav2_params_file: "nav2_params.yaml"
```

<!-- CONSIDERACIONES IMPORTANTES -->
## Consideraciones importantes

* En simulación, la GUI, `command_executor`, Nav2 y los archivos de parámetros se encuentran normalmente en el mismo equipo.

* En el robot real, `navigation.nav2_params_path` y `navigation.nav2_params_file` se resuelven en el sistema de archivos del equipo que ejecuta el comando de Nav2.

* `$HOME/GUI_ROBOGait_V2/src/command_executor/command_executor/params/config.yaml` pertenece a `command_executor` y configura, entre otros elementos, su lista de comandos permitidos. No debe confundirse con `$HOME/GUI_ROBOGait_V2/src/robogait_gui/params/config.yaml`.

* Los archivos de `simulation/` son referencias versionadas y no deben depender de cambios generados dentro de `install/` o `$HOME/.local/robogait/params`.
