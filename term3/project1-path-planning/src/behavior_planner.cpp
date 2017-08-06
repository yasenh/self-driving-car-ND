//
// Created by projectx on 8/5/17.
//

#include "behavior_planner.h"

/**
 * Finite state machine
 * The state we will use for high way self-driving scenario:
 * 1. Keep lane
 * 2. Prepare for lane chang left/right
 * 3. Change lane left/right
 *
 *
 */

// TODO: think about keep lane in middle ASAP

/**
 * The data that we need to pass to our transition functions as input:
 * 1. Predictions
 * 2. Map
 * 3. Speed limit
 * 4. Localization data
 * 5. Current state
 */

BehaviorPlanner::BehaviorPlanner(){

}

BehaviorPlanner::~BehaviorPlanner() {}

void BehaviorPlanner::UpdateLocalization(const Vehicle &host_vehicle, const std::vector<Vehicle> &fusion_vehicles) {
    host_vehicle_ = host_vehicle;
    fusion_vehicles_ = fusion_vehicles;
}


void BehaviorPlanner::GetClosestVehicle(LaneSegment lane, double &distance_front, double &distance_rear) {
    double min_distance_front = kMaxDistance;
    double min_distance_rear = kMaxDistance;

    for (size_t i = 0; i < fusion_vehicles_.size(); i++) {
        if (fusion_vehicles_[i].GetLane() == lane) {
            double distance = fusion_vehicles_[i].GetS() - host_vehicle_.GetS();
            if (distance >= 0) {
                // In the front
                if (distance < min_distance_front) {
                    min_distance_front = distance;
                }
            }
            else {
                // In the rear
                distance *= -1;
                if (distance < min_distance_rear) {
                    min_distance_rear = distance;
                }
            }
        } // end of same lane
    } // end of loop

    distance_front = min_distance_front;
    distance_rear = min_distance_rear;
}

void BehaviorPlanner::CalculateCost() {

}

BehaviorState BehaviorPlanner::UpdateState() {
    GetClosestVehicle(LaneSegment::kLeftLane, left_lane_front_, left_lane_rear_);
    GetClosestVehicle(LaneSegment::kMiddleLane, middle_lane_front_, middle_lane_rear_);
    GetClosestVehicle(LaneSegment::kRightLane, right_lane_front_, right_lane_rear_);

    

    std::cout << "Front" << middle_lane_front_ << " Rear " << middle_lane_rear_ << std::endl;
}