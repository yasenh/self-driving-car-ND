//
// Created by projectx on 8/3/17.
//

#ifndef PATH_PLANNING_TRAJECTORY_PLANNER_H
#define PATH_PLANNING_TRAJECTORY_PLANNER_H

#include <vector>

#include "spline.h"

#include "Eigen-3.3/Eigen/Core"
#include "Eigen-3.3/Eigen/QR"
#include "Eigen-3.3/Eigen/Dense"

using Eigen::MatrixXd;
using Eigen::VectorXd;
using tk::spline;

class TrajectoryPlanner {
public:
    /**
     * Constructor
     */
    TrajectoryPlanner(tk::spline spline_sx, tk::spline spline_sy, tk::spline spline_sdx, tk::spline spline_sdy);

    TrajectoryPlanner(std::vector<double> map_waypoints_x, std::vector<double> map_waypoints_y,
                                         std::vector<double> map_waypoints_dx, std::vector<double> map_waypoints_dy,
                                         std::vector<double> map_waypoints_s);

    /**
     * Destructor
     */
    virtual ~TrajectoryPlanner();

    void LoadCurrentTrajectory(std::vector<double> next_x_vals, std::vector<double> next_y_vals);
    void UpdateTrajectory(std::vector<double> start_s, std::vector<double> start_d, std::vector<double> end_s, std::vector<double> end_d, int prediction_pt_num);

    std::vector<double> GetTrajectoryX() {
        return trajectory_x_;
    }

    std::vector<double> GetTrajectoryY() {
        return trajectory_y_;
    }

private:
    // Jerk minimizing trajectories from Lesson 5: Trajectory Generation
    std::vector<double> JMT(std::vector<double> start, std::vector<double> end, double T);

    spline spline_sx_, spline_sy_, spline_sdx_, spline_sdy_;

    std::vector<double> trajectory_x_, trajectory_y_;
};


#endif //PATH_PLANNING_TRAJECTORY_PLANNER_H
