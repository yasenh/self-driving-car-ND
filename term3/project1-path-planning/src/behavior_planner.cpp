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
    int lane = host_vehicle_.GetLane();

    std::vector<double> cost;

    // change_left, keep_lane, change_right
    switch (lane) {
        case LaneSegment::kLeftLane:
            cost.push_back(kMaxCost);
            cost.push_back(1.0 / lane_front_s_[0]);
            cost.push_back(1.0 / lane_front_s_[1] + 0.4 / lane_rear_s_[1]);
            break;
        case LaneSegment::kMiddleLane:
            cost.push_back(1.0 / lane_front_s_[0] + 0.4 / lane_rear_s_[0]);
            cost.push_back(1.0 / lane_front_s_[1]);
            cost.push_back(1.0 / lane_front_s_[2] + 0.4 / lane_rear_s_[2]);
            break;
        case LaneSegment::kRightLane:
            cost.push_back(1.0 / lane_front_s_[1] + 0.4 / lane_rear_s_[1]);
            cost.push_back(1.0 / lane_front_s_[2]);
            cost.push_back(kMaxCost);
            break;
        default:
            cost.push_back(kMaxCost);
            cost.push_back(kMaxCost);
            cost.push_back(kMaxCost);
            std::cerr << "Out of boundary !!!!!!" << std::endl;
            break;
    }

    std::cout << "Left = " << cost[0] << " Keep = " << cost[1] << " Right = " << cost[2] << std::endl;
    std::vector<double>::iterator result = std::min_element(std::begin(cost), std::end(cost));
    auto min_index = std::distance(std::begin(cost), result);
    std::cout << "min element at: " << min_index << " cost = " << cost[min_index] << std::endl;

    state_ = static_cast<BehaviorState>(min_index);

}

BehaviorState BehaviorPlanner::UpdateState() {
    for (int i = 0; i < kTotalLaneNum; i++) {
        GetClosestVehicle(static_cast<LaneSegment>(i), lane_front_s_[i], lane_front_vel_[i], lane_rear_s_[i], lane_rear_vel_[i]);
    }

    CalculateCost();




    int lane = host_vehicle_.GetLane();
    int target_lane;



    switch (state_) {
        case BehaviorState::kKeepLane:
            std::cout << "Front " << lane_front_s_[lane] << " Rear " << lane_rear_s_[lane] << std::endl;
            if (lane_front_s_[lane] >= kDistanceBuffer) {
                target_delta_s_ = kTimeInterval * kPredictionPtNum * kSpeedLimit;
                target_speed_ = kSpeedLimit;
            } else {
                std::cout << "Front_vel = " << lane_front_vel_[lane] << std::endl;
                double distance = kTimeInterval * kPredictionPtNum * kSpeedLimit * 0.8;
                target_delta_s_ = distance;

                double speed = lane_front_vel_[lane] < kSpeedLimit ? lane_front_vel_[lane] : kSpeedLimit;
                target_speed_ = speed;
            }
            target_lane = lane;
            target_d_ = kLaneD[target_lane];
            break;
        case BehaviorState::kChangeLeft:
            target_lane = lane - 1;
            target_d_ = kLaneD[target_lane];
            target_speed_ = kSpeedLimit;
            break;
        case BehaviorState::kChangeRight:
            target_lane = lane + 1;
            target_d_ = kLaneD[target_lane];
            target_speed_ = kSpeedLimit;
            break;
    }


//    std::cout << target_s_ << " , " << target_speed_ << std::endl;


}