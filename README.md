# GUI_ROBOGait

<!-- TABLA DE CONTENIDOS -->
<details open>
    <summary><h2>Tabla de contenidos</h2></summary>
    <ol>
        <li><a href="#descripción">Descripción</a></li>
        <li><a href="#organización-del-repositorio">Organización del repositorio</a></li>
        <li><a href="#instalador-principal">Instalador principal</a></li>
        <li><a href="#compatibilidad">Compatibilidad</a></li>
        <li><a href="#instalación">Instalación</a></li>
        <li><a href="#herramientas-de-desarrollo">Herramientas de desarrollo</a></li>
    </ol>
</details>

<!-- DESCRIPCIÓN -->
## Descripción

GUI_ROBOGait_V2 es un workspace ROS 2 que contiene la aplicación gráfica utilizada para gestionar pacientes y experimentos, visualizar y crear mapas, y supervisar y controlar el robot. El repositorio incluye además el nodo que ejecuta comandos en el robot, las interfaces ROS compartidas y los recursos necesarios para instalar o desplegar la aplicación.

<!-- ORGANIZACIÓN DEL REPOSITORIO -->
## Organización del repositorio

### `src/`

Contiene los paquetes ROS 2 y el código fuente del proyecto:

* [`robogait_gui`](src/robogait_gui/) &rarr; Aplicación Qt Quick que proporciona la interfaz de usuario, la persistencia SQLite y la comunicación ROS 2 con el robot. Consulte su documentacion técnica en &rarr; [src/robogait_gui/README.md](src/robogait_gui/README.md).

* [`command_executor`](src/command_executor/command_executor/) &rarr; Nodo ejecutado en el robot para validar y gestionar comandos de navegación, mapeado y mapas.

* [`command_executor_msgs`](src/command_executor/command_executor_msgs/) &rarr; Mensajes y servicios que comunican la GUI con `command_executor`.

  Los dos paquetes de `command_executor` se documentan conjuntamente en &rarr; [src/command_executor/README.md](src/command_executor/README.md).

* [`navigation_pkg`](src/navigation_pkg/) &rarr; Interfaces ROS compartidas por compatibilidad con RoboMesh e interfaz experimental para detecciones de cámara. Consulte su documentación técnica en &rarr; [src/navigation_pkg/README.md](src/navigation_pkg/README.md).

### `scripts/`

Contiene las funciones de instalación. Los scripts auxiliares son cargados por `install.sh` y no están diseñados para ejecutarse individualmente.

Consulte los flujos, cambios del sistema y responsabilidad de cada archivo en [scripts/README.md](scripts/README.md).

### `docker/`

Contiene los archivos y configuracion necesaria para ejecutar la aplicacion en un contenedor.

La construcción, configuración, ejecución y resolución de problemas se detallan en  [docker/README.md](docker/README.md).

### Archivos principales

* [`install.sh`](install.sh) &rarr; Punto de entrada interactivo para instalar el entorno de desarrollo o desplegar la aplicación con Docker.

* [`README.md`](README.md) &rarr; Documentación general y navegación hacia cada módulo.

<!-- INSTALADOR PRINCIPAL -->
## Instalador principal

[`install.sh`](install.sh) carga las funciones de `scripts/`, activa el modo estricto de Bash y guía al usuario mediante la selección de modalidad y distribución ROS 2. Las operaciones que necesitan privilegios solicitan la contraseña de `sudo` una vez y la reutilizan durante esa ejecución.

|         Modalidad         |                              Instala y configura                             |                             Resultado                             |
|---------------------------|:----------------------------------------------------------------------------:|-------------------------------------------------------------------|
|  Ordenador de desarrollo  |        ROS 2, Qt 6, herramientas C++, dependencias y utilidades SQLite       |      Workspace compilado y entornos ROS añadidos a `.bashrc`      |
| Dispositivo de aplicación | Docker Engine, grupos gráficos, entorno Docker, imagen y servicio de usuario |   GUI preparada para iniciarse mediante systemd y Docker Compose  |
|        Solo Docker        |            Docker Engine, grupos gráficos, entorno Docker e imagen           | Imagen disponible para ejecución manual, sin instalar el servicio |

El flujo de desarrollo compila mediante `colcon` los paquetes `command_executor_msgs`, `navigation_pkg`, `command_executor` y `robogait_gui`. Los flujos Docker construyen la imagen `robogait_gui:${ROS_DISTRO}`; únicamente la modalidad **Dispositivo de aplicación** crea `robogait-gui-docker.service`.

> [!WARNING]
>
>El instalador añade repositorios APT, instala paquetes y modifica la configuración del sistema. La instalación de Docker puede eliminar paquetes incompatibles y añade el usuario actual al grupo `docker`. Revise [Cambios realizados en el sistema](scripts/README.md#cambios-realizados-en-el-sistema) de *scripts/README.md*  para conocer todos los cambios.

<!-- COMPATIBILIDAD -->
## Compatibilidad

Para el entorno de desarrollo, el instalador valida estas combinaciones:

| Ubuntu |  ROS 2 |
|--------|--------|
| 22.04  | Humble |
| 24.04  | Jazzy  |

La aplicación Docker también se construye seleccionando Humble o Jazzy mediante `ROS_DISTRO`.

<!-- INSTALACIÓN -->
## Instalación

Clone el repositorio y ejecute el instalador desde su raíz:

```bash
git clone https://github.com/victor-villamar/GUI_ROBOGait_V2.git
cd GUI_ROBOGait_V2
chmod +x install.sh
./install.sh
```

Seleccione una de las tres modalidades mostradas por el instalador. Después de instalar Docker, cierre la sesión y vuelva a entrar, o reinicie el equipo, para aplicar la pertenencia al grupo `docker`.

Para instrucciones específicas consulte:

* [Instalación y scripts](scripts/README.md#uso).
* [Construcción y ejecución con Docker](docker/README.md#construcción).
* [Compilación y ejecución de la GUI](src/robogait_gui/README.md#compilación).

<!-- HERRAMIENTAS DE DESARROLLO -->
## Herramientas de desarrollo

El modo **Ordenador de desarrollo** instala `sqlite3` y `sqlitebrowser` para inspeccionar la base de datos. Estas herramientas no son necesarias para ejecutar la GUI: el acceso de la aplicación a SQLite se realiza mediante `libqt6sql6-sqlite`, instalado tanto en desarrollo como en Docker.

El instalador no instala el IDE Qt Creator, que continúa siendo opcional.