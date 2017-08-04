//
// Created by projectx on 8/3/17.
//

#include "trajectory_planner.h"
#include "utils.h"

TrajectoryPlanner::TrajectoryPlanner(tk::spline spline_sx, tk::spline spline_sy, tk::spline spline_sdx, tk::spline spline_sdy)
        : spline_sx_(spline_sx), spline_sy_(spline_sy), spline_sdx_(spline_sdx), spline_sdy_(spline_sdy) {}

TrajectoryPlanner::TrajectoryPlanner(std::vector<double> map_waypoints_x, std::vector<double> map_waypoints_y,
                                     std::vector<double> map_waypoints_dx, std::vector<double> map_waypoints_dy,
                                     std::vector<double> map_waypoints_s) {

    spline_sx_.set_points(map_waypoints_s, map_waypoints_x);
    spline_sy_.set_points(map_waypoints_s, map_waypoints_y);
    spline_sdx_.set_points(map_waypoints_s, map_waypoints_dx);
    spline_sdy_.set_points(map_waypoints_s, map_waypoints_dy);

}

TrajectoryPlanner::~TrajectoryPlanner() {}

std::vector<double> TrajectoryPlanner::JMT(std::vector<double> start, std::vector<double> end, double T) {

    /**
     * Calculate the Jerk Minimizing Trajectory that connects the initial state to the final state in time T.

     *INPUTS
      start - the vehicles start location given as a length three array corresponding to initial values of [s, s_dot, s_double_dot]
      end   - the desired end state for vehicle. Like "start" this is a length three array.
      T     - The duration, in seconds, over which this maneuver should occur.

     *OUTPUT
      an array of length 6, each value corresponding to a coefficent in the polynomial
      s(t) = a_0 + a_1 * t + a_2 * t**2 + a_3 * t**3 + a_4 * t**4 + a_5 * t**5

     *EXAMPLE

     *> JMT( [0, 10, 0], [10, 10, 0], 1)
     *[0.0, 10.0, 0.0, 0.0, 0.0, 0.0]
     */

    MatrixXd A = MatrixXd(3, 3);
    A <<    T*T*T, T*T*T*T, T*T*T*T*T,
            3*T*T, 4*T*T*T, 5*T*T*T*T,
            6*T  , 12*T*T , 20*T*T*T;

    MatrixXd B = MatrixXd(3,1);
    B <<    end[0]-(start[0]+start[1]*T+.5*start[2]*T*T),
            end[1]-(start[1]+start[2]*T),
            end[2]-start[2];

    MatrixXd Ai = A.inverse();
    MatrixXd C = Ai*B;

    std::vector <double> result = {start[0], start[1], .5*start[2]};
    for(int i = 0; i < C.size(); i++) {
        result.push_back(C.data()[i]);
    }

    return result;
}


void TrajectoryPlanner::LoadCurrentTrajectory(std::vector<double> next_x_vals, std::vector<double> next_y_vals) {
    trajectory_x_ = next_x_vals;
    trajectory_y_ = next_y_vals;
}


void TrajectoryPlanner::UpdateTrajectory(std::vector<double> start_s, std::vector<double> start_d, std::vector<double> end_s, std::vector<double> end_d, int prediction_pt_num) {
    double prediction_time = prediction_pt_num * kTimeInterval;

    std::vector<double> jmt_s = JMT(start_s, end_s, prediction_time);
    std::vector<double> jmt_d = JMT(start_d, end_d, prediction_time);

    for (size_t i = 0; i < prediction_pt_num; i++) {
        double t = kTimeInterval * i;
        double t2 = t * t;
        double t3 = t2 * t;
        double t4 = t3 * t;
        double t5 = t4 * t;


        double wp_s = jmt_s[0] + jmt_s[1] * t + jmt_s[2] * t2 + jmt_s[3] * t3 + jmt_s[4] * t4 + jmt_s[5] * t5;
        double wp_d = jmt_d[0] + jmt_d[1] * t + jmt_d[2] * t2 + jmt_d[3] * t3 + jmt_d[4] * t4 + jmt_d[5] * t5;

        double wp_x = spline_sx_(wp_s);
        double wp_y = spline_sy_(wp_s);
        double wp_dx = spline_sdx_(wp_s);
        double wp_dy = spline_sdy_(wp_s);

        wp_x += wp_dx * wp_d;
        wp_y += wp_dy * wp_d;

        trajectory_x_.push_back(wp_x);
        trajectory_y_.push_back(wp_y);
    }
}

