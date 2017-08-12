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
 */


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

    int current_lane = host_vehicle_.GetLane();

    /**
     *  1. We don't want the vehicle change back and forth too frequently
     *  2. We need to ensure the safety
     */
    if (keep_lane_time_ < kMinLaneKeepingTime && lane_front_s_[current_lane] >= kSafetyDistance) {
        state_ = BehaviorState::kKeepLane;
        keep_lane_time_++;
        return;
    }

    std::vector<double> cost;

    // change_left, keep_lane, change_right
    double cost_change_left, cost_keep_lane, cost_change_right;

    // left FR - 0, 1; middle FR - 2, 3; right FR - 4, 5
    std::vector<double> cost_distance;
    std::vector<double> cost_velocity;

    // TODO: global optimization, thinking about all 3 lanes rather than adjacent lane only

    // calculate cost function
    for (size_t i = 0; i < kTotalLaneNum; i++) {
        double cost_temp;
        // if the vehicle in front/rear is too far away, we can ignore it
        if (lane_front_s_[i] < kMaxPerceptionDistance) {
            cost_temp = exp(-(lane_front_s_[i] - kSafetyDistance));
            cost_temp = cost_temp > 10 ? 10 : cost_temp;
            cost_distance.push_back(cost_temp + kMinCost);

            cost_temp = exp(-(lane_front_vel_[i] - kSpeedLimit));
            cost_temp = cost_temp > 10 ? 10 : cost_temp;
            cost_velocity.push_back(cost_temp);
        }
        else {
            cost_distance.push_back(kMinCost);
            cost_velocity.push_back(0);
        }

        if (lane_rear_s_[i] < kMaxPerceptionDistance) {
            cost_temp = 0.4 * exp(-(lane_rear_s_[i] - kSafetyDistance));
            cost_temp = cost_temp > 1 ? 1 : cost_temp;
            cost_distance.push_back(cost_temp);
        }
        else {
            cost_distance.push_back(0);
        }
    }

    // calculate all cost (distance, velocity, etc..)
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
    // find minimal cost in order to choose next behavior
    std::vector<double>::iterator result = std::min_element(std::begin(cost), std::end(cost));
    auto min_index = std::distance(std::begin(cost), result);
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
     *  2. target_speed_
     *  3. target_d_
     *  4. target_prediction_pt_num_
     */

    double last_target_speed = target_speed_;
    switch (state_) {
        case BehaviorState::kChangeLeft:
            target_lane_ = lane - 1;
            // give more time for turning to avoid jerk
            target_delta_s_ = kPredictionDistance * 2;
            target_speed_ = lane_front_vel_[target_lane_] > kSpeedLimit ? kSpeedLimit : lane_front_vel_[target_lane_];
            break;
        case BehaviorState::kKeepLane:
            target_lane_ = lane;
            if (lane_front_s_[lane] >= kDistanceBuffer) {
                std::cout << "FRONT CLEAR" << std::endl;
                target_delta_s_ = kPredictionDistance;
                target_speed_ = kSpeedLimit;
            } else {
                std::cout << "FRONT CAR WITHIN SAFETY DISTANCE" << std::endl;
                target_delta_s_ = kPredictionDistance * 1.5;
                target_speed_ = lane_front_vel_[target_lane_] > kSpeedLimit ? kSpeedLimit : lane_front_vel_[target_lane_];
                target_speed_ = (target_speed_ + last_target_speed) / 2.0f;
            }
            break;
        case BehaviorState::kChangeRight:
            target_lane_ = lane + 1;
            // give more time for turning to avoid jerk
            target_delta_s_ = kPredictionDistance * 2;
            target_speed_ = lane_front_vel_[target_lane_] > kSpeedLimit ? kSpeedLimit : lane_front_vel_[target_lane_];
            break;
        default:
            target_lane_ = lane;
            target_delta_s_ = kPredictionDistance;
            target_speed_ = kSpeedLimit;
            break;
    }

    // we don't want the vehicle change too much in short time
    double delta_speed = target_speed_ - last_target_speed;
    if (last_target_speed > kMinSpeed) {
        if (delta_speed > kSpeedBuffer) {
            target_speed_ = last_target_speed + kSpeedBuffer;
        } else if (delta_speed < -kSpeedBuffer) {
            target_speed_ = last_target_speed - kSpeedBuffer;
        }
    }

    target_d_ = kLaneD[target_lane_];
    target_prediction_pt_num_ = static_cast<int>(target_delta_s_ / (target_speed_ * kTimeInterval));

    //std::cout << "Behavior = " << state_ << std::endl;

}