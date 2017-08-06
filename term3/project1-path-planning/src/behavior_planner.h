//
// Created by projectx on 8/5/17.
//

#ifndef PATH_PLANNING_BEHAVIOR_PLANNER_H
#define PATH_PLANNING_BEHAVIOR_PLANNER_H

#include <iostream>
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

private:
    void GetClosestVehicle(LaneSegment lane, double &distance_front, double &distance_rear);
    void CalculateCost();

    float target_speed_;

    Vehicle host_vehicle_;
    std::vector<Vehicle> fusion_vehicles_;

    double left_lane_front_, left_lane_rear_;
    double middle_lane_front_, middle_lane_rear_;
    double right_lane_front_, right_lane_rear_;

    BehaviorState state_;

};


#endif //PATH_PLANNING_BEHAVIOR_PLANNER_H
