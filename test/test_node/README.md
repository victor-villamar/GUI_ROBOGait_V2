# Test Node - Robot Discovery Testing

## Descripción

Nodo de prueba para verificar el funcionamiento del sistema de descubrimiento de robots mediante topics ROS2. Publica mensajes de estado simulando robots conectados al sistema.

## Propósito

Este nodo permite:
- Probar el descubrimiento automático de robots sin hardware físico
- Verificar la recepción de mensajes `std_msgs/String` en el topic de descubrimiento
- Simular múltiples robots publicando en diferentes namespaces
- Validar la lógica de detección de robots del sistema principal

## Ejecución

### Ejecutar el test node

**Paso 1**: Source del workspace y ejecutar el nodo

```bash
# Source del workspace
source /home/$USER/workspace/install/setup.bash

#Ejecutar el nodo
ros2 run robogait_gui test_node_node
```

**Paso 2**: El programa solicitará información interactiva

```terminal
===============================================
     TEST NODE - Robot Status Publisher       
===============================================

Ingrese el ROS_DOMAIN_ID (0-101): 
```

**Usuario debe introducir**: Por ejemplo, `42` y presionar Enter

```terminal
Ingrese el ROS_DOMAIN_ID (0-101): 42

ROS_DOMAIN_ID configurado a: 42

Quiere lanzar el nodo con namespace? (s/n): 
```

**Usuario debe introducir**:

- `n` (sin namespace) + Enter, **o**
- `s` (con namespace) + Enter

**Si elige `n`**:

```terminal
Quiere lanzar el nodo con namespace? (s/n): n
[INFO] [1770745999.980046091] [test_node]: [main] Node created with ROS_DOMAIN_ID: 42
[INFO] [1770745999.980117509] [test_node]: [TestNode::TestNode] Initializing TestNode
[INFO] [1770745999.980127961] [test_node]: [TestNode::TestNode] Robot Name: robogait
[INFO] [1770745999.980623893] [test_node]: [TestNode::TestNode] TestNode initialized successfully
[INFO] [1770745999.980640942] [test_node]: [TestNode::TestNode] Publishing on topic: /robot_status at 1Hz
Presione Ctrl+C para detener.
```

**Si elige `s`**:

```terminal
Quiere lanzar el nodo con namespace? (s/n): s
Ingrese el robot namespace: robot1
[INFO] [1770746117.517901640] [test_node]: [main] Node created with ROS_DOMAIN_ID: 42
[INFO] [1770746117.517957721] [test_node]: [TestNode::TestNode] Initializing TestNode
[INFO] [1770746117.517965788] [test_node]: [TestNode::TestNode] Robot Name: robogait
[INFO] [1770746117.518353112] [test_node]: [TestNode::TestNode] TestNode initialized successfully
[INFO] [1770746117.518364822] [test_node]: [TestNode::TestNode] Publishing on topic: /robot1/robot_status at 1Hz
Presione Ctrl+C para detener.
```

**Para detener**: Presionar `Ctrl+C`

## Uso con GUI

1. **Terminal 1**: Ejecutar el test node

    ```bash
    ros2 run robogait_gui test_node_node
    ```

2. **Terminal 2**: Ejecutar la aplicación principal

    ```bash
    ros2 run robogait_gui robogait_gui
    ```

3. En la aplicación logearse con el usuario `admin` y contraseña `a`

4. Una vez que estemos en la ventana de conexión al robot debemos abrir el panel de settings pulsando en el engranaje.

    ![settings.png](/test/test_node/images/Settings.png)

    Nos aparecerá el panel de configuracion de desarrollado
        ![developer_settings.png](/test/test_node/images/developer_settings.png)

    En el podemos configurar tanto el descubrimiento por namespace o utilizando el tópico. También podemos cambiar el `ROS DOMAIN ID` (por defecto 0).

    * **Caso con namespace y topico**

        Si no modificamos nada por defecto debemos lanzar un nodo con namespace por ejemplo

        ```terminal
        ros2 run turtlebot3_teleop teleop_keyboard --prefixros-args -r __ns:=/tb3_1
        ```

        El namespace establecido es el mismo que debemos poner en el test_node cuando nos lo solicite.

        De esta forma podemos visualizar los robots al darle al boton **Buscar robot**

    * **Caso sin namespace y topico**

        Para este caso debemos desactivar el checkbox de arriba y aplicar los cambios con el botón `Aplicar`.

        Para este caso debemos lanzar un nodo sin namespace y el test_node con la opción de sin namespace.

        ```terminal
        ros2 launch turtlebot3_gazebo empty_world.launch.py
        ```

    * **Caso con/sin namespace y sin topico**

        En este caso no es necesario lanzar el test_node. Podemos modificar si queremos lanzarlo con namespace o sin el y empleando los comandos anteriormente mencionados para cada caso.

        En este caso el descubrimiento no se filtra por tópico sino que se realiza con mediante el grafo de ROS2.