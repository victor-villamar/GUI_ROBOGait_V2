"""Launch TurtleBot 4 simulation using ROBOGait's global TF layout."""

from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from irobot_create_common_bringup.offset import OffsetParser, RotationalOffsetX, RotationalOffsetY
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, IncludeLaunchDescription, OpaqueFunction
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node, PushRosNamespace, SetRemap
import xacro


def _global_tf_remappings():
    return [('/tf', '/tf'), ('/tf_static', '/tf_static')]


def _normalise_namespace(value):
    namespace = value.strip().strip('/')
    if not namespace:
        raise RuntimeError('The namespace argument must contain a robot namespace')
    return namespace


def _robot_description(model, namespace):
    description_share = Path(get_package_share_directory('turtlebot4_description'))
    xacro_file = description_share / 'urdf' / model / 'turtlebot4.urdf.xacro'
    document = xacro.process_file(
        str(xacro_file),
        mappings={'gazebo': 'ignition', 'namespace': namespace},
    )
    description = document.toxml()

    replacements = {
        '<remapping>/tf:=tf</remapping>': '<remapping>/tf:=/tf</remapping>',
        '<remapping>/tf_static:=tf_static</remapping>':
            '<remapping>/tf_static:=/tf_static</remapping>',
    }
    for source, destination in replacements.items():
        if source not in description:
            raise RuntimeError(
                f'Unable to adapt TurtleBot 4 robot description: missing {source}'
            )
        description = description.replace(source, destination)

    return description


def _launch_robot(context):
    namespace = _normalise_namespace(LaunchConfiguration('namespace').perform(context))
    model = LaunchConfiguration('model').perform(context)
    use_sim_time = LaunchConfiguration('use_sim_time')
    world = LaunchConfiguration('world')
    x = LaunchConfiguration('x')
    y = LaunchConfiguration('y')
    z = LaunchConfiguration('z')
    yaw = LaunchConfiguration('yaw')

    robot_name = f'{namespace}/turtlebot4'
    dock_name = f'{namespace}/standard_dock'
    frame_prefix = f'{namespace}/'

    gz_bringup_share = get_package_share_directory('turtlebot4_gz_bringup')
    common_bringup_share = get_package_share_directory('irobot_create_common_bringup')
    create_gz_share = get_package_share_directory('irobot_create_gz_bringup')
    viz_share = get_package_share_directory('turtlebot4_viz')

    bridge_launch = PathJoinSubstitution(
        [gz_bringup_share, 'launch', 'ros_gz_bridge.launch.py']
    )
    turtlebot_nodes_launch = PathJoinSubstitution(
        [gz_bringup_share, 'launch', 'turtlebot4_nodes.launch.py']
    )
    create_nodes_launch = str(
        Path(__file__).with_name('robogait_create3_nodes.launch.py')
    )
    create_gz_nodes_launch = PathJoinSubstitution(
        [create_gz_share, 'launch', 'create3_gz_nodes.launch.py']
    )
    dock_description_launch = PathJoinSubstitution(
        [common_bringup_share, 'launch', 'dock_description.launch.py']
    )

    dock_offset_x = RotationalOffsetX(0.157, yaw)
    dock_offset_y = RotationalOffsetY(0.157, yaw)
    x_dock = OffsetParser(x, dock_offset_x)
    y_dock = OffsetParser(y, dock_offset_y)
    z_robot = OffsetParser(z, -0.0025)
    yaw_dock = OffsetParser(yaw, 3.1416)

    robot_group = GroupAction([
        PushRosNamespace(namespace),
        # Apply the global TF contract to every node in this scope, including
        # nodes created by the upstream TurtleBot 4 and Create 3 launch files.
        # Per-node remappings do not propagate into IncludeLaunchDescription.
        SetRemap(src='/tf', dst='/tf'),
        SetRemap(src='/tf_static', dst='/tf_static'),
        SetRemap(src=f'/model/{robot_name}/tf', dst='/tf'),
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[{
                'use_sim_time': use_sim_time,
                'frame_prefix': frame_prefix,
                'robot_description': _robot_description(model, namespace),
            }],
            remappings=_global_tf_remappings(),
        ),
        Node(
            package='joint_state_publisher',
            executable='joint_state_publisher',
            name='joint_state_publisher',
            output='screen',
            parameters=[{'use_sim_time': use_sim_time}],
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([dock_description_launch]),
            launch_arguments={'gazebo': 'ignition'}.items(),
        ),
        Node(
            package='ros_gz_sim',
            executable='create',
            arguments=[
                '-name', robot_name,
                '-x', x,
                '-y', y,
                '-z', z_robot,
                '-Y', yaw,
                '-topic', 'robot_description',
            ],
            output='screen',
        ),
        Node(
            package='ros_gz_sim',
            executable='create',
            arguments=[
                '-name', dock_name,
                '-x', x_dock,
                '-y', y_dock,
                '-z', z,
                '-Y', yaw_dock,
                '-topic', 'standard_dock_description',
            ],
            output='screen',
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([bridge_launch]),
            launch_arguments={
                'model': model,
                'robot_name': robot_name,
                'dock_name': dock_name,
                'namespace': namespace,
                'use_sim_time': use_sim_time,
                'world': world,
            }.items(),
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([turtlebot_nodes_launch]),
            launch_arguments={
                'model': model,
                'param_file': LaunchConfiguration('param_file'),
            }.items(),
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([create_nodes_launch]),
            launch_arguments={
                'namespace': namespace,
                'gazebo': 'ignition',
            }.items(),
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([create_gz_nodes_launch]),
            launch_arguments={
                'robot_name': robot_name,
                'dock_name': dock_name,
            }.items(),
        ),
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='rplidar_stf',
            output='screen',
            arguments=[
                '0', '0', '0', '0', '0', '0.0',
                f'{frame_prefix}rplidar_link',
                f'{robot_name}/rplidar_link/rplidar',
            ],
            remappings=_global_tf_remappings(),
        ),
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='camera_stf',
            output='screen',
            arguments=[
                '0', '0', '0', '1.5707', '-1.5707', '0',
                f'{frame_prefix}oakd_rgb_camera_optical_frame',
                f'{robot_name}/oakd_rgb_camera_frame/rgbd_camera',
            ],
            remappings=_global_tf_remappings(),
        ),
        Node(
            condition=IfCondition(LaunchConfiguration('rviz')),
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='screen',
            arguments=['-d', str(Path(viz_share) / 'rviz' / 'navigation.rviz')],
            remappings=_global_tf_remappings(),
        ),
    ])

    return [robot_group]


def generate_launch_description():
    gz_bringup_share = get_package_share_directory('turtlebot4_gz_bringup')
    simulation_launch = PathJoinSubstitution(
        [gz_bringup_share, 'launch', 'sim.launch.py']
    )
    default_params = PathJoinSubstitution(
        [gz_bringup_share, 'config', 'turtlebot4_node.yaml']
    )

    arguments = [
        DeclareLaunchArgument('namespace', default_value='robot1'),
        DeclareLaunchArgument('rviz', default_value='false', choices=['true', 'false']),
        DeclareLaunchArgument('world', default_value='warehouse'),
        DeclareLaunchArgument('model', default_value='standard', choices=['standard', 'lite']),
        DeclareLaunchArgument('use_sim_time', default_value='true', choices=['true', 'false']),
        DeclareLaunchArgument('param_file', default_value=default_params),
        DeclareLaunchArgument('x', default_value='0.0'),
        DeclareLaunchArgument('y', default_value='0.0'),
        DeclareLaunchArgument('z', default_value='0.0'),
        DeclareLaunchArgument('yaw', default_value='0.0'),
        # Accepted for compatibility with turtlebot4_spawn.launch.py. Navigation and
        # SLAM are deliberately launched separately by ROBOGait.
        DeclareLaunchArgument('nav2', default_value='false', choices=['false']),
        DeclareLaunchArgument('slam', default_value='false', choices=['false']),
        DeclareLaunchArgument('localization', default_value='false', choices=['false']),
    ]

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([simulation_launch]),
        launch_arguments={'world': LaunchConfiguration('world')}.items(),
    )

    return LaunchDescription(arguments + [gazebo, OpaqueFunction(function=_launch_robot)])
