//
// Created by projectx on 8/1/17.
//

#include "vehicle.h"
#include "utils.h"

Vehicle::Vehicle() {}

Vehicle::Vehicle(int id) : id_(id) {}

Vehicle::Vehicle(int id, double s, double d, double vel) : id_(id), s_(s), d_(d), vel_(vel) {
    LaneSegmentation();
}

Vehicle::~Vehicle() {}

void Vehicle::UpdateState(double s, double d, double vel) {
    s_   = s;
    d_   = d;
    vel_ = vel;
    LaneSegmentation();

    //state_s_ = {s_, vel_, 0};
}

void Vehicle::LaneSegmentation() {
    if (d_ < 0 || d_ > kTotalLaneNum * kLaneWidth)
        lane_idx_ = LaneSegment::kUnknownLane;
    else
        lane_idx_ = static_cast<int>(d_ / kLaneWidth);
}

int Vehicle::GetLane() {
    return lane_idx_;
}