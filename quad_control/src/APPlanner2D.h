#ifndef _APPLANNER2D_
#define _APPLANNER2D_

#include "ros/ros.h"
#include "geometry_msgs/Pose.h"
#include "nav_msgs/MapMetaData.h"
#include <Eigen/Dense>
#include "quad_control/PlanRequest.h"
#include "MapAnalyzer.h"

using namespace std;

// Column vector of 6 double
typedef Eigen::Matrix<double, 6, 1> Vector6d;


/*
 * APPlanner2D realizes path planning via artificial potentials method, for a 2D
 * known environment. The planning is realized considering the virtual force
 * acting on the robot as a velocity, and the integration is realized through
 * Euler method.
 * It is assumed that each C-Obstacle has the same range of influence, and all
 * the repulsive force gains k_(r,i) are the same.
 * Notice that the planning is made with respect to the world ENU frame.
 */
class APPlanner2D{

public:
    APPlanner2D();

    /*
     * Analyze the map and detect obstacles. It can be done only once for
     * multiple planning instances on the same map.
     */
    void setMap(nav_msgs::OccupancyGrid &map);

    /* ---- REWRITE THIS ----
     * Plan the trajectory.
     * Notice that, after the service is called in the first place, the map is
     * stored. So, you can use again the same map, setting to 0 the width or
     * height in req.map.info. In this way, the map is not analyzed again, and
     * execution time is saved.
     */
    void plan(quad_control::PlanRequestPtr req);


private:

    /*
     * Compute position and orientation error, between actual and desired pose.
     * The orientation error is computed considering RPY angles.
     */
    Vector6d _computeError(geometry_msgs::Pose q, geometry_msgs::Pose qd);

    /*
     * Compute next configuration using Euler integration: qnext = q + ft*T.
     * Parameters:
     *  - q: robot pose
     *  - ft: total force acting on the robot
     */
    geometry_msgs::Pose _eulerIntegration(geometry_msgs::Pose q, Vector6d ft);

    /*
     * Compute the virtual forces applied on the robot, via the artificial
     * potentials method.
     * Parameters:
     *  - q: robot pose
     *  - e: error
     */
    Vector6d _computeForce(geometry_msgs::Pose q, Vector6d e);
        
    /*
     * Compute repulsive forces acting on the robot.
     * Parameters:
     *  - rx, ry: robot position (in meters)
     */
    Vector6d _computeRepulsiveForce(double rx, double ry);


private:
    ros::NodeHandle _nh;
    ros::Subscriber _sub;
    ros::Publisher _pathPub, _pub;

    double _ka, _kr;    // Attractive and repulsive forces gain
    double _eta;        // Range of influence
    double _gamma;      // Used for repulsive forces computation, can be 2 or 3
    double _p_eps;      // if position error <= eps, it is assumed error = 0
    double _o_eps;      // if orientation error <= eps, it is assumed error = 0
    double _sampleTime;
    bool _mapReady;     // true if the map has been set at least once

    nav_msgs::MapMetaData _mapInfo;
    MapAnalyzer _mapAnalyzer;
};

#endif
