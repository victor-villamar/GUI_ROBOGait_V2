#!/usr/bin/env python3

from geometry_msgs.msg import PoseStamped
from nav_msgs.msg import Path  # Correct import for Path message
from nav2_simple_commander.robot_navigator import BasicNavigator, TaskResult
import rclpy
from rclpy.duration import Duration
import sys
import math

def print_path_info(path: Path) -> None:
    """Prints information about the planned path"""
    if not path.poses:
        print("Could not calculate a valid path!")
        return
    
    print("\n=== Global plan information ===")
    print(f"Number of points in the path: {len(path.poses)}")
    print(f"First point (start): x={path.poses[0].pose.position.x:.2f}, y={path.poses[0].pose.position.y:.2f}")
    print(f"Last point (goal): x={path.poses[-1].pose.position.x:.2f}, y={path.poses[-1].pose.position.y:.2f}")
    
    # Calculate approximate distance
    distance = 0.0
    for i in range(1, len(path.poses)):
        x1 = path.poses[i-1].pose.position.x
        y1 = path.poses[i-1].pose.position.y
        x2 = path.poses[i].pose.position.x
        y2 = path.poses[i].pose.position.y
        distance += ((x2-x1)**2 + (y2-y1)**2)**0.5
    
    print(f"Approximate distance: {distance:.2f} meters")
    print("=== End of plan information ===\n")


def main() -> None:
    rclpy.init()
    
    # Obtener poses de los argumentos si se proporcionan
    if len(sys.argv) > 7:
        initial_x = float(sys.argv[1])
        initial_y = float(sys.argv[2])
        intial_yaw = float(sys.argv[3]) * math.pi / 180.0 # change to radians
        goal_x = float(sys.argv[4])
        goal_y = float(sys.argv[5])
        goal_yaw = float(sys.argv[6]) * math.pi / 180.0 # change to radians
        map_file = sys.argv[7] if len(sys.argv) > 7 else None

    else:
        # Valores por defecto (los del ejemplo original)
        print("No se han proporcionado argumentos. Usando valores por defecto.")
        initial_x = 0.41
        initial_y = -0.19
        goal_x = 3.81
        goal_y = 1.04
        map_file = None
    
    navigator = BasicNavigator()
    
    # Crear publisher para el plan (opcional, para visualización en RViz)
    path_publisher = navigator.create_publisher(Path, '/global_plan', 10)
    
    # Configurar pose inicial
    initial_pose = PoseStamped()
    initial_pose.header.frame_id = 'map'
    initial_pose.header.stamp = navigator.get_clock().now().to_msg()
    initial_pose.pose.position.x = initial_x
    initial_pose.pose.position.y = initial_y

    initial_pose.pose.orientation.z = math.sin(intial_yaw / 2.0)
    initial_pose.pose.orientation.w = math.cos(intial_yaw / 2.0)
    
    # Configurar pose objetivo
    goal_pose = PoseStamped()
    goal_pose.header.frame_id = 'map'
    goal_pose.header.stamp = navigator.get_clock().now().to_msg()
    goal_pose.pose.position.x = goal_x
    goal_pose.pose.position.y = goal_y
    goal_pose.pose.orientation.z = math.sin(goal_yaw / 2.0)
    goal_pose.pose.orientation.w = math.cos(goal_yaw / 2.0)
    
    # Si se proporcionó un mapa, cargarlo
    if map_file:
        print(f"Loanding map name: {map_file}")
        navigator.changeMap(map_file)
    
    # Establecer pose inicial y activar Nav2
    navigator.setInitialPose(initial_pose)
    navigator.waitUntilNav2Active()
    
    # Obtener y mostrar el plan
    print("\Calculating trajectory from:")
    print(f" - Init: x={initial_x}, y={initial_y}")
    print(f" - Goal: x={goal_x}, y={goal_y}")
    
    path = navigator.getPath(initial_pose, goal_pose)
    
    if path:
        # Publicar el plan para visualización
        path_publisher.publish(path)
        
        # Mostrar información del plan
        print_path_info(path)
        
        # Ejecutar la navegación si el usuario lo desea

    else:
        print("A valid trajectory could not be calculated!")
    
    # Mostrar resultado final
    result = navigator.getResult()
    if result == TaskResult.SUCCEEDED:
        print('Navigation completed successfully!')
    elif result == TaskResult.CANCELED:
        print('Navegation canceled!')
    elif result == TaskResult.FAILED:
        print('Navigation failed!')
    
    rclpy.shutdown()

if __name__ == '__main__':
    # Instrucciones de uso
    if len(sys.argv) > 1 and (sys.argv[1] == '-h' or sys.argv[1] == '--help'):
        print("\nUso:")
        print(f"  {sys.argv[0]} [initial_x initial_y goal_x goal_y map_file]")
        print("\nEjemplo:")
        print(f"  {sys.argv[0]} 0.0 0.0 5.0 3.0 /path/to/map.yaml\n")
        sys.exit(0)
    
    main()