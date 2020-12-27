#ifndef _H_CONTROLLER_
#define _H_CONTROLLER_

#include "ros/ros.h"
#include <Eigen/Dense>
#include <tf/transform_listener.h>
#include "quad_control/UAVPose.h"
#include "geometry_msgs/Accel.h"
#include "nav_msgs/Odometry.h"
#include "quad_control/Trajectory.h"
#include "LP2Filter.h"

/*
 * This is a hierarchical controller for the VTOL UAV. Starting from desired
 * positions, velocities and accelerations, computes control wrenches.
 */
class Controller{

public:
    Controller();

    void run();

    void trajectoryReceived(quad_control::TrajectoryPtr traj);

    void odomReceived(nav_msgs::OdometryPtr odom);


private:
    ros::NodeHandle _nh;
    ros::Subscriber _trajSub;               // Trajectory
    ros::Subscriber _odomSub;               // Odometry
    ros::Publisher _pub;                    // Command wrench

    double _m;                              // Mass
    Eigen::Matrix3d _Ib;                    // Inertia
    Eigen::Matrix<double,3,6> _Kp, _Ke;     // Proportional gains
    Eigen::Matrix<double,3,6> _Kpi, _Kei;   // Integral gains
    double _rate;

    quad_control::Trajectory _traj;         // Trajectory
    bool _trajReady;

    ros::Time _startTime;
    bool _started, _completed;

    bool _hovering;

    quad_control::UAVPose _dp;              // Desired pos
    geometry_msgs::Accel _dv, _da;          // Desired vel and acc

    nav_msgs::Odometry _odom;
    Eigen::Matrix3d _Rb;
    bool _odomReady;

    Eigen::Matrix<double,6,1> _epInt, _eoInt;   // Integral
    Eigen::Vector3d _deta;
    Eigen::Vector3d _mud;
    double _uT;
    Eigen::Vector3d _tau;

    LP2Filter<Eigen::Vector2d> _filter;     // Low-pass 2nd order filter
    int _filterSteps;


    void _getCurrentTrajPoint();
    void _innerLoop();
    void _outerLoop();

};

#endif
