"""Launch Create 3 nodes with the diff-drive controller on global TF topics."""

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node


def generate_launch_description():
    common_share = get_package_share_directory('irobot_create_common_bringup')
    control_share = get_package_share_directory('irobot_create_control')

    namespace = LaunchConfiguration('namespace')
    gazebo = LaunchConfiguration('gazebo')

    control_params = PathJoinSubstitution(
        [control_share, 'config', 'control.yaml']
    )

    joint_state_broadcaster = Node(
        package='controller_manager',
        executable='spawner',
        name='spawner_joint_state_broadcaster',
        output='screen',
        arguments=[
            'joint_state_broadcaster',
            '-c', 'controller_manager',
            '--controller-manager-timeout', '30',
        ],
    )

    diffdrive_controller = Node(
        package='controller_manager',
        executable='spawner',
        name='spawner_diffdrive_controller',
        namespace=namespace,
        output='screen',
        parameters=[control_params],
        arguments=[
            'diffdrive_controller',
            '-c', 'controller_manager',
            '--controller-manager-timeout', '30',
            '--controller-ros-args',
            '--ros-args --remap /tf:=/tf --remap /tf_static:=/tf_static',
        ],
    )

    start_diffdrive_controller = RegisterEventHandler(
        OnProcessExit(
            target_action=joint_state_broadcaster,
            on_exit=[diffdrive_controller],
        )
    )

    def config_file(name):
        return PathJoinSubstitution([common_share, 'config', name])

    common_parameters = [{'use_sim_time': True}]
    create3_nodes = [
        Node(
            package='irobot_create_nodes',
            executable='hazards_vector_publisher',
            name='hazards_vector_publisher',
            parameters=[config_file('hazard_vector_params.yaml')] + common_parameters,
            output='screen',
        ),
        Node(
            package='irobot_create_nodes',
            executable='ir_intensity_vector_publisher',
            name='ir_intensity_vector_publisher',
            parameters=[config_file('ir_intensity_vector_params.yaml')] + common_parameters,
            output='screen',
        ),
        Node(
            package='irobot_create_nodes',
            executable='motion_control',
            name='motion_control',
            parameters=[{'use_sim_time': True, 'safety_override': 'backup_only'}],
            output='screen',
            remappings=[('/tf', '/tf'), ('/tf_static', '/tf_static')],
        ),
        Node(
            package='irobot_create_nodes',
            executable='wheel_status_publisher',
            name='wheel_status_publisher',
            parameters=[config_file('wheel_status_params.yaml')] + common_parameters,
            output='screen',
        ),
        Node(
            package='irobot_create_nodes',
            executable='mock_publisher',
            name='mock_publisher',
            parameters=[config_file('mock_params.yaml')] + common_parameters,
            output='screen',
        ),
        Node(
            package='irobot_create_nodes',
            executable='robot_state',
            name='robot_state',
            parameters=[config_file('robot_state_params.yaml')] + common_parameters,
            output='screen',
        ),
        Node(
            package='irobot_create_nodes',
            executable='kidnap_estimator_publisher',
            name='kidnap_estimator_publisher',
            parameters=[config_file('kidnap_estimator_params.yaml')] + common_parameters,
            output='screen',
        ),
        Node(
            package='irobot_create_nodes',
            executable='ui_mgr',
            name='ui_mgr',
            parameters=[
                config_file('ui_mgr_params.yaml'),
                {'use_sim_time': True, 'gazebo': gazebo},
            ],
            output='screen',
        ),
    ]

    return LaunchDescription([
        DeclareLaunchArgument(
            'gazebo', default_value='ignition', choices=['classic', 'ignition']
        ),
        DeclareLaunchArgument('namespace', default_value=''),
        joint_state_broadcaster,
        start_diffdrive_controller,
        *create3_nodes,
    ])