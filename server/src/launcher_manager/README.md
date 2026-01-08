
# Node Manager

El directorio `node_manager` es un componente del módulo del servidor `GUI_ROBOGait`. Se encarga de comunicarse con ros2 como como el servidor de la base de datos.

## Características

- **Gestión del ciclo de vida de los nodos**: Se encarga de la inicialización, monitoreo y terminación de los nodos, asegurando que cada nodo se gestione adecuadamente durante todo su ciclo de vida.
- **Comunicación entre nodos**: Facilita el intercambio de datos y la sincronización entre nodos para garantizar que el sistema opere de manera fluida.
- **Manejo de errores**: Implementa mecanismos para detectar y manejar errores relacionados con los nodos, garantizando que los fallos se gestionen de manera adecuada y que el sistema permanezca estable.
- **Flexibilidad de configuración**: Proporciona parámetros configurables para ajustar el comportamiento de los nodos mediante el directorio `param/`.

## Estructura del Directorio

La estructura del directorio `node_manager` es la siguiente:

```
node_manager/
├── include/            # Archivos de encabezado para las funcionalidades principales del gestor de nodos
├── launch_file/        # Archivos de configuración de lanzamiento
├── param/              # Parámetros de configuración
├── src/                # Código fuente de la implementación del gestor de nodos
├── CMakeLists.txt      # Configuración del sistema de construcción
├── package.xml         # Metadatos y dependencias del paquete
└── README.md           # Documentación del gestor de nodos
```

## Uso

1. **Directorio Include**: Agrega los archivos `.h` del directorio `include/` a tu proyecto para acceder a las funcionalidades principales proporcionadas por el gestor de nodos.
2. **Directorio Launch File**: Utiliza los archivos del directorio `launch_file/` para crear planes de trayectoria.
3. **Parámetros**: Modifica los parámetros del directorio `param/` para personalizar el comportamiento del gestor de nodos y ajustar el sistema.
4. **Código Fuente**: Extiende o modifica la funcionalidad del gestor de nodos editando los archivos de código fuente en el directorio `src/`.
5. **Sistema de Construcción**: Usa el archivo `CMakeLists.txt` para gestionar dependencias, configurar el sistema de construcción y compilar el módulo del gestor de nodos.
6. **Configuración del Paquete**: El archivo `package.xml` contiene metadatos sobre el paquete, incluidas las dependencias e información de versión. Asegúrate de actualizar este archivo cuando sea necesario.

## Include y Src

+ **header.h**: Se encuentran todos los `enum` y `struct` del proyecto y el path absoluto para llegar a la carpeta `param/` se recomienda cambiar el path si esta en una nueva dirección

+ **servidor.h servidor.cpp**: Donde se realiza la comunicacion con el cliente y el intercambio de mensajes.

+ **LauncherManager.hpp LauncherManager.cpp**: Se trata de las carpeta donde se hacen todas las sucripciones a nodos o servicios o topic del programa.

+ **ProcessController.hpp ProcessController.cpp**: En estos archivos se ejecutan todos lo programas en segundo plano con un `fork`.
