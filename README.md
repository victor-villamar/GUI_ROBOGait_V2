
# Proyecto ROBOGait – GUI

<!-- TABLA DE CONTENIDOS -->
<details>
    <summary><h2>Tabla de contenidos</h2></summary>
    <ol>
        <li>
            <a href="#instalación">Instalación</a>
               <ul>
                  <li><a href="#requisitos-previos">Requisitos previos</a></li>
               </ul>
               <ul>
                  <li><a href="#pasos-para-instalar-qt-creator">Pasos para instalar Qt Creator</a></li>
               </ul>
               <ul>
                  <li><a href="#clonación-del-repositorio">Clonación del repositorio</a></li>
               </ul>
               <ul>
                  <li><a href="#instalación-de-dependencias">Instalación de dependencias</a></li>
               </ul>
               <ul>
                  <li><a href="#instalación-de-la-base-de-datos">Instalación de la base de datos</a></li>
               </ul>
               <ul>
                  <li><a href="#configuración-del-proyecto-en-qt-creator">Configuración del proyecto en Qt Creator</a></li>
               </ul>
        </li>
    </ol>
</details>

<!-- INSTALACION -->
## Instalación

Para poder ejecutar este proyecto hay que seguir los siguientes pasos

<!-- REQUISITOS PREVIOS -->
### Requisitos previos

- Ubuntu 22.04
- Conexión a Internet
- Cuenta UPM para licencia educativa de Qt

<!-- CLONACION DEL REPOSITORIO -->
## Clonación del repositorio

1. Creamos un workspace para el proyecto

   ```terminal
   cd
   mkdir /home/$USER/gui_ws
   cd gui_ws
   mkdir src
   cd src
   ```

2. Clonamos el repositorio &rarr; [GUI_ROBOGait_V2](https://github.com/victor-villamar/GUI_ROBOGait_V2#)

   ```terminal
   git clone https://github.com/victor-villamar/GUI_ROBOGait_V2.git
   ```

3. Nos cambiamos a la rama de desarrollo (actualmente develop)

   ```terminal
   cd GUI_ROBOGait_v2
   git checkout develop
   ```

<!-- PASOS PARA INSTALAR QT CREATOR -->
### Pasos para instalar Qt Creator

1. Descarga Qt Creator con licencia educativa desde este enlace:  
   👉 [Solicitar licencia educativa de Qt](https://www.qt.io/qt-educational-license#application)

2. En la instalación instalar la versión de Qt **6.10.2**

   ![Qt_installation](/images/Qt_installation.png)

   2.1. Instalar Virtual Keyboard de *Additional Libraries*

      ![Qt_installation_virtual_keyboard](/images/Qt_installation_virtual_keyboard.png)

   2.2. Instalar *Plugin Development* en la sección Qt Creator

      ![Qt_Creator_installation](/images/Qt_Creator_installation.png)

<!-- INSTALACION DE DEPENDENCIAS -->
### Instalación de dependencias

>[!WARNING]
>
>La instalación de las dependencias se realiza dando por sentado que se ha realizado la instalacion de Qt con la versión **6.10.2** anteriormente mencionada. No se asegura compatibilidad de las dependencias con versiones anteriores.

1. **Instalar ROS 2 Humble**  
   👉 [Guía oficial](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debs.html)

2. **Instalar dependencias para la base de datos SQlite**

      ```terminal
      sudo apt install sqlite3 sqlitebrowser
      ```

3. **Instalar dependencias del proyecto**

      ```terminal
      sudo apt install libyaml-cpp-dev
      ```

4. **Plugin de ROS**

   Para la instalación del plugin de ROS para Qt Creator hay que seguir los siguientes pasos:

   1. *Obtener nuestra versión de Qt Creator*

      Para obtener la versión de Qt debemos dirigirnos desde la aplicación a *Help* &rarr; *About Qt Creator*

      ![qt_version](/images/qt_version.png)

   2. *Instalación de dependencias*

      ```terminal
      sudo apt install libgl1-mesa-dev ninja-build libutf8proc-dev libcups2-dev
      ```

   3. *Descargar el plugin*

      Debemos descargarlo desde el repositorio oficial &rarr; [ros_qtc_plugin](https://github.com/ros-industrial/ros_qtc_plugin)

      Con la versión obtenida en el paso anterior nos dirigimos a *Tags* &rarr; *18.0*

      ![Tag](/images/Tag.png)
      ![Tag_2](/images/Tags_2.png)

      Y nos descargamos el siguiente *asset*

      ![plugin_asset](/images/plugin_asset.png)

      En el directorio donde hayamos descargado el *asset* realizamos

      ```terminal
      zip_file="qtcreator-plugin-ros-18.0-Linux-x86_64.zip"
      unzip "$zip_file" -d "${zip_file%.zip}"
      cd qtcreator-plugin-ros-18.0-Linux-x86_64
      ```

      Dentro visualizaremos dos carpetas *lib* y *share*. Debemos copiar el contenido de *lib* a nuestro *lib* de Qt Creator

      ```terminal
      cp -r $HOME/Downloads/qtcreator-plugin-ros-18.0-Linux-x86_64/lib/qtcreator/plugins/* $HOME/Qt/Tools/QtCreator/lib/qtcreator/plugins
      ```

      Para confirmar que se ha instalado correctamente en la aplicación nos dirigimos a *Help* &rarr; *About Plugins*

      ![about_plugins](/images/about_plugins.png)

      Y buscamos ***ROS Project Manager***

      ![plugin_verification](/images/plugin_verfication.png)

<!-- INSTALACION DE LA BASE DE DATOS -->
## Instalación de la base de datos

La base de datos del proyecto debe ser instalada en el directorio ```/opt/robogait/``` . Para la instalación es necesario ejecutar el script ```install_db.sh``` que se encuentra en el directoria *scripts* del proyecto. Lo debemos ejecutar como ```sudo```.

```terminal
sudo $HOME/workspace/src/GUI_ROBOGait_V2/scripts/install_db.sh
```

Seguimos la indicaciones del script y al finalizar reiniciamos el dispositivo para que los cambios se apliquen correctamente.

<!-- CONFIGURACION DEL PROYECTO EN QT CREATOR -->
## Configuración del proyecto en Qt Creator

   1. *Creación del proyecto*

      - En la Qt Creator nos dirigimos a *File* &rarr; *New Project*.

         ![new_project](/images/new_project.png)

      - Establecemos en *Project* &rarr; *Other Project* y elejimos *ROS Wokspace*

         ![create_project_1](/images/create_project_1.png)

      - En el siguiente paso establecemos *Name* &rarr; el nombre que queramos, en *Build System* &rarr; *Colcon* y en *Wokspace Path* el path donde se encuentra nuestro workspace.

         ![project_name_and_location](/images/project_name_and_location.png)

   2. *Configuracion de build y run*

      Nos dirigimos al apartado *Project* de la barra lateral.

      ![Projects](/images/Projects.png)

      En el apartado ***Build & Run*** seleccionamos el kit que hemos instalado.

      ![Build&Run](/images/Build&Run.png)

      - ***Configuración del Build***

         ![build_configuration](/images/build_configuration.png)

         - En el apartado *ROS Manager* establecemos la siguiente configuración

            ![ros_manager_configuration](/images/ros_manager_configuration.png)

         - En el apartado *Build Steps* establecemos la siguiente configuracion

            ![build_steps](/images/build_steps.png)

         - En el apartado *System Enviroment* agregamos la siguiente línea

            ![system_enviroment](/images/system_enviroment.png)

            ```terminal
            QT6_DIR=/home/$USER/Qt/6.10.2/gcc_64/lib/cmake/Qt6
            ```

      - ***Configuración del Run***

         Una vez compilado el proyecto nos dirigimos a la sección *Run Settings*

         ![run_configuration](/images/run_configuration.png)

         - En *executable* buscamos el ejecutable generado en la compilación (/install/robogait_gui/bin) con el Browser

         - En el apartado *Enviroment*

            ![enviroment](/images/enviroment.png)

            Agregamos al final de *LD_LIBRARY_PATH*

            ```terminal
            :/home/$USER/Qt/6.10.2/gcc_64/lib
            ```
