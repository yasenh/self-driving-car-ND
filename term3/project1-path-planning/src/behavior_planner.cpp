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
    keep_lane_time_ = 0;
    target_lane_ = LaneSegment::kMiddleLane;
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

    std::cout << "Lane Keep Time = " << keep_lane_time_ << std::endl;
    if (keep_lane_time_ < kMinLaneKeepingTime) {
        state_ = BehaviorState::kKeepLane;
        keep_lane_time_++;
        return;
    }


    int current_lane = host_vehicle_.GetLane();

    std::vector<double> cost;

    // change_left, keep_lane, change_right
    double cost_change_left, cost_keep_lane, cost_change_right;

    // left FR - 0, 1; middle FR - 2, 3; right FR - 4, 5
    std::vector<double> cost_distance;

    for (size_t i = 0; i < kTotalLaneNum; i++) {
        double cost_temp = exp(-(lane_front_s_[i] - kSafetyDistance));
        cost_temp = cost_temp > 2 ? 2 : cost_temp;
        cost_distance.push_back(cost_temp);

        cost_temp = 0.4 * exp(-(lane_rear_s_[i] - kSafetyDistance));
        cost_temp = cost_temp > 1 ? 1 : cost_temp;
        cost_distance.push_back(cost_temp);
    }

    double safety_velocity = kSpeedLimit; //- kSpeedBuffer;
    std::vector<double> cost_velocity;

    std::cout << "Middle front vel = " << lane_front_vel_[1] << std::endl;
    for (size_t i = 0; i < kTotalLaneNum; i++) {
        double cost_temp = exp(-(lane_front_vel_[i] - safety_velocity));
        cost_temp = cost_temp > 1 ? 1 : cost_temp;
        cost_velocity.push_back(cost_temp);
    }


    switch (current_lane) {
        case LaneSegment::kLeftLane:
            // Left -> Out
            cost_change_left = kMaxCost;
            // Left -> Left
            cost_keep_lane = cost_distance[0] + cost_velocity[0];
            // Left -> Middle
            if (lane_front_s_[1] <= kSafetyDistance || lane_rear_s_[1] <= kSafetyDistance) {
                cost_change_right = kMaxCost;
            }
            else {
                cost_change_right = (cost_distance[2] + cost_distance[3] + cost_velocity[1]) * kChangeLaneFactor *
                                    kMiddleLaneFactor;
            }
            break;
        case LaneSegment::kMiddleLane:
            // Middle -> Left
            if (lane_front_s_[0] <= kSafetyDistance || lane_rear_s_[0] <= kSafetyDistance) {
                cost_change_left = kMaxCost;
            }
            else {
                cost_change_left = (cost_distance[0] + cost_distance[1] + cost_velocity[0]) * kChangeLaneFactor;
            }
            // Middle -> Middle
            cost_keep_lane = (cost_distance[2] + cost_velocity[1]) * kMiddleLaneFactor;
            // Middle -> Right
            if (lane_front_s_[2] <= kSafetyDistance || lane_rear_s_[2] <= kSafetyDistance) {
                cost_change_right = kMaxCost;
            }
            else {
                cost_change_right = (cost_distance[4] + cost_distance[5] + cost_velocity[2]) * kChangeLaneFactor;
            }
            break;
        case LaneSegment::kRightLane:
            // Right -> Middle
            if (lane_front_s_[1] <= kSafetyDistance || lane_rear_s_[1] <= kSafetyDistance) {
                cost_change_left = kMaxCost;
            }
            else {
                cost_change_left = (cost_distance[2] + cost_distance[3] + cost_velocity[1]) * kChangeLaneFactor *
                                   kMiddleLaneFactor;
            }
            // Right -> Right
            cost_keep_lane = cost_distance[4] + cost_velocity[2];
            // Right -> Out
            cost_change_right = kMaxCost;
            break;
        default:
            cost_change_left = kMaxCost;
            cost_keep_lane = kMaxCost;
            cost_change_right = kMaxCost;
            std::cerr << "Out of boundary !!!!!!" << std::endl;
            break;
    }

    cost.push_back(cost_change_left);
    cost.push_back(cost_keep_lane);
    cost.push_back(cost_change_right);

    std::cout << "Left = " << cost[0] << " Keep = " << cost[1] << " Right = " << cost[2] << std::endl;
    std::vector<double>::iterator result = std::min_element(std::begin(cost), std::end(cost));
    auto min_index = std::distance(std::begin(cost), result);
//    std::cout << "min element at: " << min_index << " cost = " << cost[min_index] << std::endl;

    state_ = static_cast<BehaviorState>(min_index);

    if (state_ == BehaviorState::kKeepLane) {
        keep_lane_time_++;
    }
    else {
        keep_lane_time_ = 0;
    }
}

BehaviorState BehaviorPlanner::UpdateState() {
    for (int i = 0; i < kTotalLaneNum; i++) {
        GetClosestVehicle(static_cast<LaneSegment>(i), lane_front_s_[i], lane_front_vel_[i], lane_rear_s_[i], lane_rear_vel_[i]);
    }

    CalculateCost();

    int lane = host_vehicle_.GetLane();

    /**
     *  Calculate
     *  1. target_delta_s_
     *  2. target_speed
     *  3. target_d_
     *  4. target_prediction_pt_num_
     */

    switch (state_) {
        case BehaviorState::kChangeLeft:
            target_lane_ = lane - 1;
            target_d_ = kLaneD[target_lane_];
            // give more time for turning to avoid jerk
            target_delta_s_ = kTimeInterval * kPredictionPtNum * kSpeedLimit * 2;
            target_speed_ = lane_front_vel_[target_lane_] > kSpeedLimit ? kSpeedLimit : lane_front_vel_[target_lane_];
            //target_prediction_pt_num_ = kPredictionPtNum;
            break;
        case BehaviorState::kKeepLane:
            target_lane_ = lane;
            target_d_ = kLaneD[target_lane_];
            std::cout << "Front " << lane_front_s_[lane] << " Rear " << lane_rear_s_[lane] << std::endl;
            if (lane_front_s_[lane] >= kDistanceBuffer) {
                target_delta_s_ = kTimeInterval * kPredictionPtNum * kSpeedLimit;
                target_speed_ = kSpeedLimit;
                //target_prediction_pt_num_ = kPredictionPtNum;
            } else {
                std::cout << "Front_vel = " << lane_front_vel_[lane] << std::endl;
                //double distance = kTimeInterval * kPredictionPtNum * kSpeedLimit * 0.8;
                double distance = kTimeInterval * kPredictionPtNum * kSpeedLimit;

                target_delta_s_ = distance;
                double speed = lane_front_vel_[target_lane_] > kSpeedLimit ? kSpeedLimit : lane_front_vel_[target_lane_];
                //target_speed_ = speed - kSpeedBuffer;
                target_speed_ = speed;

            }
            break;
        case BehaviorState::kChangeRight:
            target_lane_ = lane + 1;
            target_d_ = kLaneD[target_lane_];
            // give more time for turning to avoid jerk
            target_delta_s_ = kTimeInterval * kPredictionPtNum * kSpeedLimit * 2;
            target_speed_ = lane_front_vel_[target_lane_] > kSpeedLimit ? kSpeedLimit : lane_front_vel_[target_lane_];
            //target_prediction_pt_num_ = kPredictionPtNum;
            break;
        default:
            target_lane_ = lane;
            target_d_ = kLaneD[target_lane_];
            target_delta_s_ = kTimeInterval * kPredictionPtNum * kSpeedLimit;
            target_speed_ = kSpeedLimit;
            //target_prediction_pt_num_ = kPredictionPtNum;
            break;
    }

    target_prediction_pt_num_ = static_cast<int>(target_delta_s_ / (target_speed_ * kTimeInterval));

    std::cout << "Behavior = " << state_ << std::endl;
    std::cout << "Current Lane = " << lane << std::endl;
    std::cout << "Target Lane = " << target_lane_ << std::endl;

//    std::cout << target_s_ << " , " << target_speed_ << std::endl;


}