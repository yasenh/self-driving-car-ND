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


void BehaviorPlanner::GetClosestVehicle(LaneSegment lane, double &distance_front, double &velocity_front, double &distance_rear, double &velocity_rear) {
    distance_front = kMaxDistance;
    distance_rear = kMaxDistance;

    velocity_front = kMaxSpeed;
    velocity_rear = kMaxSpeed;

    for (size_t i = 0; i < fusion_vehicles_.size(); i++) {
        if (fusion_vehicles_[i].GetLane() == lane) {
            double distance = fusion_vehicles_[i].GetS() - host_vehicle_.GetS();
            double velocity = fusion_vehicles_[i].GetVel();
            if (distance >= 0) {
                // In the front
                if (distance < distance_front) {
                    distance_front = distance;
                    velocity_front = velocity;
                }
            }
            else {
                // In the rear
                distance *= -1;
                if (distance < distance_rear) {
                    distance_rear = distance;
                    velocity_rear = velocity;
                }
            }
        } // end of same lane
    } // end of loop
}

void BehaviorPlanner::CalculateCost() {
    
}

BehaviorState BehaviorPlanner::UpdateState() {
    for (int i = 0; i < kTotalLaneNum; i++) {
        GetClosestVehicle(static_cast<LaneSegment>(i), lane_front_s_[i], lane_front_vel_[i], lane_rear_s_[i], lane_rear_vel_[i]);
    }

    CalculateCost();

    std::cout << "Front " << lane_front_s_[1] << " Rear " << lane_rear_s_[1] << std::endl;
    if (lane_front_s_[1] >= kDistanceBuffer) {
        target_delta_s_ = kTimeInterval * kPredictionPtNum * kSpeedLimit;
        target_speed_ = kSpeedLimit;
    }
    else {
        //target_delta_s_ = lane_front_s_[1];

        std::cout << "Front_vel = " << lane_front_vel_[1] << std::endl;


        //double distance = lane_front_s_[1] < kDistanceBuffer ? lane_front_s_[1] : kDistanceBuffer;
        double distance = kTimeInterval * kPredictionPtNum * kSpeedLimit * 0.8;
        target_delta_s_ = distance;

        double speed = lane_front_vel_[1] < kSpeedLimit ? lane_front_vel_[1] : kSpeedLimit;
        target_speed_ = speed;
    }


//    std::cout << target_s_ << " , " << target_speed_ << std::endl;


}