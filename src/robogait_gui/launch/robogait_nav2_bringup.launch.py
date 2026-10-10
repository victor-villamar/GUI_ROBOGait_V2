"""Launch namespaced Nav2 nodes using global TF topics and prefixed frames."""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    GroupAction,
    OpaqueFunction,
    SetEnvironmentVariable,
    SetLaunchConfiguration,
)
from launch.conditions import IfCondition
from launch.substitutions import EqualsSubstitution, LaunchConfiguration, NotEqualsSubstitution
from launch_ros.actions import Node, PushRosNamespace, SetParameter
from launch_ros.descriptions import ParameterFile
from nav2_common.launch import ReplaceString, RewrittenYaml


def _normalise_namespace(context):
    namespace = LaunchConfiguration('namespace').perform(context).strip().strip('/')
    if not namespace:
        raise RuntimeError('The namespace argument must contain a robot namespace')
    return [SetLaunchConfiguration('namespace', namespace)]


def generate_launch_description():
    bringup_share = get_package_share_directory('nav2_bringup')

    namespace = LaunchConfiguration('namespace')
    use_sim_time = LaunchConfiguration('use_sim_time')
    autostart = LaunchConfiguration('autostart')
    params_file = LaunchConfiguration('params_file')
    map_yaml_file = LaunchConfiguration('map')
    use_localization = LaunchConfiguration('use_localization')
    use_respawn = LaunchConfiguration('use_respawn')
    log_level = LaunchConfiguration('log_level')

    namespaced_params = ReplaceString(
        source_file=params_file,
        replacements={'<robot_namespace>': (namespace,)},
    )
    configured_params = ParameterFile(
        RewrittenYaml(
            source_file=namespaced_params,
            root_key=namespace,
            param_rewrites={'autostart': autostart},
            convert_types=True,
        ),
        allow_substs=True,
    )

    tf_remappings = [('/tf', '/tf'), ('/tf_static', '/tf_static')]
    localization_nodes = ['map_server', 'amcl']
    navigation_nodes = [
        'controller_server',
        'smoother_server',
        'planner_server',
        'route_server',
        'behavior_server',
        'velocity_smoother',
        'collision_monitor',
        'bt_navigator',
        'waypoint_follower',
        'docking_server',
    ]

    common_node_arguments = ['--ros-args', '--log-level', log_level]

    localization_group = GroupAction(
        condition=IfCondition(use_localization),
        actions=[
            Node(
                condition=IfCondition(EqualsSubstitution(map_yaml_file, '')),
                package='nav2_map_server',
                executable='map_server',
                name='map_server',
                output='screen',
                respawn=use_respawn,
                respawn_delay=2.0,
                parameters=[configured_params],
                arguments=common_node_arguments,
                remappings=tf_remappings,
            ),
            Node(
                condition=IfCondition(NotEqualsSubstitution(map_yaml_file, '')),
                package='nav2_map_server',
                executable='map_server',
                name='map_server',
                output='screen',
                respawn=use_respawn,
                respawn_delay=2.0,
                parameters=[configured_params, {'yaml_filename': map_yaml_file}],
                arguments=common_node_arguments,
                remappings=tf_remappings,
            ),
            Node(
                package='nav2_amcl',
                executable='amcl',
                name='amcl',
                output='screen',
                respawn=use_respawn,
                respawn_delay=2.0,
                parameters=[configured_params],
                arguments=common_node_arguments,
                remappings=tf_remappings,
            ),
            Node(
                package='nav2_lifecycle_manager',
                executable='lifecycle_manager',
                name='lifecycle_manager_localization',
                output='screen',
                arguments=common_node_arguments,
                parameters=[{'autostart': autostart, 'node_names': localization_nodes}],
            ),
        ],
    )

    navigation_group = GroupAction(actions=[
        Node(
            package='nav2_controller', executable='controller_server', output='screen',
            respawn=use_respawn, respawn_delay=2.0, parameters=[configured_params],
            arguments=common_node_arguments,
            remappings=tf_remappings + [('cmd_vel', 'cmd_vel_nav')],
        ),
        Node(
            package='nav2_smoother', executable='smoother_server', name='smoother_server',
            output='screen', respawn=use_respawn, respawn_delay=2.0,
            parameters=[configured_params], arguments=common_node_arguments,
            remappings=tf_remappings,
        ),
        Node(
            package='nav2_planner', executable='planner_server', name='planner_server',
            output='screen', respawn=use_respawn, respawn_delay=2.0,
            parameters=[configured_params], arguments=common_node_arguments,
            remappings=tf_remappings,
        ),
        Node(
            package='nav2_route', executable='route_server', name='route_server',
            output='screen', respawn=use_respawn, respawn_delay=2.0,
            parameters=[configured_params], arguments=common_node_arguments,
            remappings=tf_remappings,
        ),
        Node(
            package='nav2_behaviors', executable='behavior_server', name='behavior_server',
            output='screen', respawn=use_respawn, respawn_delay=2.0,
            parameters=[configured_params], arguments=common_node_arguments,
            remappings=tf_remappings + [('cmd_vel', 'cmd_vel_nav')],
        ),
        Node(
            package='nav2_bt_navigator', executable='bt_navigator', name='bt_navigator',
            output='screen', respawn=use_respawn, respawn_delay=2.0,
            parameters=[configured_params], arguments=common_node_arguments,
            remappings=tf_remappings,
        ),
        Node(
            package='nav2_waypoint_follower', executable='waypoint_follower',
            name='waypoint_follower', output='screen', respawn=use_respawn,
            respawn_delay=2.0, parameters=[configured_params],
            arguments=common_node_arguments, remappings=tf_remappings,
        ),
        Node(
            package='nav2_velocity_smoother', executable='velocity_smoother',
            name='velocity_smoother', output='screen', respawn=use_respawn,
            respawn_delay=2.0, parameters=[configured_params],
            arguments=common_node_arguments,
            remappings=tf_remappings + [('cmd_vel', 'cmd_vel_nav')],
        ),
        Node(
            package='nav2_collision_monitor', executable='collision_monitor',
            name='collision_monitor', output='screen', respawn=use_respawn,
            respawn_delay=2.0, parameters=[configured_params],
            arguments=common_node_arguments, remappings=tf_remappings,
        ),
        Node(
            package='opennav_docking', executable='opennav_docking',
            name='docking_server', output='screen', respawn=use_respawn,
            respawn_delay=2.0, parameters=[configured_params],
            arguments=common_node_arguments, remappings=tf_remappings,
        ),
        Node(
            package='nav2_lifecycle_manager', executable='lifecycle_manager',
            name='lifecycle_manager_navigation', output='screen',
            arguments=common_node_arguments,
            parameters=[{'autostart': autostart, 'node_names': navigation_nodes}],
        ),
    ])

    namespaced_stack = GroupAction(actions=[
        PushRosNamespace(namespace),
        SetParameter(name='use_sim_time', value=use_sim_time),
        localization_group,
        navigation_group,
    ])

    arguments = [
        DeclareLaunchArgument('namespace', default_value='robot1'),
        DeclareLaunchArgument('use_namespace', default_value='true', choices=['true', 'True']),
        DeclareLaunchArgument('use_sim_time', default_value='true'),
        DeclareLaunchArgument('autostart', default_value='true'),
        DeclareLaunchArgument(
            'params_file',
            default_value=os.path.join(bringup_share, 'params', 'nav2_params.yaml'),
        ),
        DeclareLaunchArgument('map', default_value=''),
        DeclareLaunchArgument('use_localization', default_value='true'),
        DeclareLaunchArgument('slam', default_value='False', choices=['False', 'false']),
        DeclareLaunchArgument('use_composition', default_value='False', choices=['False', 'false']),
        DeclareLaunchArgument('use_respawn', default_value='False'),
        DeclareLaunchArgument('log_level', default_value='info'),
    ]

    return LaunchDescription(arguments + [
        SetEnvironmentVariable('RCUTILS_LOGGING_BUFFERED_STREAM', '1'),
        OpaqueFunction(function=_normalise_namespace),
        namespaced_stack,
    ])
