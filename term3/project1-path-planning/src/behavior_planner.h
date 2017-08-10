//
// Created by projectx on 8/5/17.
//

#ifndef PATH_PLANNING_BEHAVIOR_PLANNER_H
#define PATH_PLANNING_BEHAVIOR_PLANNER_H

#include <iostream>
#include <algorithm>
#include <math.h>
#include "vehicle.h"
#include "utils.h"

class BehaviorPlanner {
public:
    /**
     * Constructor
     */
    BehaviorPlanner();

    /**
     * Destructor
     */
    virtual ~BehaviorPlanner();

    void UpdateLocalization(const Vehicle &host_vehicle, const std::vector<Vehicle> &fusion_vehicles);
    BehaviorState UpdateState();

    double GetTargetDeltaS() {
        return target_delta_s_;
    }

    int GetTargetLane() {
        return target_lane_;
    }

    double GetTargetD() {
        return target_d_;
    }

    double GetTargetSpeed() {
        return target_speed_;
    }

    int GetTargetPredictionPtNum(){
        return target_prediction_pt_num_;
    }

private:
    void GetClosestVehicle(LaneSegment lane, double &distance_front, double &velocity_front, double &distance_rear, double &velocity_rear);
    void CalculateCost();

    double target_delta_s_;
    double target_d_;
    double target_speed_;
    int target_prediction_pt_num_;

    int target_lane_;

    Vehicle host_vehicle_;
    std::vector<Vehicle> fusion_vehicles_;

    double lane_front_s_[kTotalLaneNum], lane_rear_s_[kTotalLaneNum];
    double lane_front_vel_[kTotalLaneNum], lane_rear_vel_[kTotalLaneNum];

    int keep_lane_time_;

    BehaviorState state_;

};


#endif //PATH_PLANNING_BEHAVIOR_PLANNER_H
