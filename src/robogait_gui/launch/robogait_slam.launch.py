"""Launch SLAM Toolbox with global TF topics and robot-prefixed frames."""

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, IncludeLaunchDescription, OpaqueFunction
from launch.conditions import IfCondition, UnlessCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import PushRosNamespace, SetRemap
from nav2_common.launch import RewrittenYaml


def _launch_setup(context):
    namespace = LaunchConfiguration('namespace').perform(context).strip().strip('/')
    if not namespace:
        raise RuntimeError('The namespace argument must contain a robot namespace')

    slam_toolbox_share = get_package_share_directory('slam_toolbox')

    use_sim_time = LaunchConfiguration('use_sim_time')
    sync = LaunchConfiguration('sync')
    autostart = LaunchConfiguration('autostart')
    use_lifecycle_manager = LaunchConfiguration('use_lifecycle_manager')
    slam_params = LaunchConfiguration('params')

    rewritten_params = RewrittenYaml(
        source_file=slam_params,
        root_key=f'/{namespace}',
        param_rewrites={
            'map_name': f'/{namespace}/map',
            'scan_topic': f'/{namespace}/scan',
            'map_frame': 'map',
            'odom_frame': f'{namespace}/odom',
            'base_frame': f'{namespace}/base_link',
        },
        convert_types=True,
    )

    sync_launch = PathJoinSubstitution(
        [slam_toolbox_share, 'launch', 'online_sync_launch.py']
    )
    async_launch = PathJoinSubstitution(
        [slam_toolbox_share, 'launch', 'online_async_launch.py']
    )

    group = GroupAction([
        PushRosNamespace(namespace),
        SetRemap(src='/tf', dst='/tf'),
        SetRemap(src='/tf_static', dst='/tf_static'),
        SetRemap(src='tf', dst='/tf'),
        SetRemap(src='tf_static', dst='/tf_static'),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(sync_launch),
            launch_arguments={
                'use_sim_time': use_sim_time,
                'autostart': autostart,
                'use_lifecycle_manager': use_lifecycle_manager,
                'slam_params_file': rewritten_params,
            }.items(),
            condition=IfCondition(sync),
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(async_launch),
            launch_arguments={
                'use_sim_time': use_sim_time,
                'autostart': autostart,
                'use_lifecycle_manager': use_lifecycle_manager,
                'slam_params_file': rewritten_params,
            }.items(),
            condition=UnlessCondition(sync),
        ),
    ])

    return [group]


def generate_launch_description():
    navigation_share = get_package_share_directory('turtlebot4_navigation')
    default_params = PathJoinSubstitution(
        [navigation_share, 'config', 'slam.yaml']
    )

    return LaunchDescription([
        DeclareLaunchArgument('namespace', default_value='robot1'),
        DeclareLaunchArgument('use_sim_time', default_value='true'),
        DeclareLaunchArgument('sync', default_value='true'),
        DeclareLaunchArgument('autostart', default_value='true'),
        DeclareLaunchArgument('use_lifecycle_manager', default_value='false'),
        DeclareLaunchArgument('params', default_value=default_params),
        OpaqueFunction(function=_launch_setup),
    ])
