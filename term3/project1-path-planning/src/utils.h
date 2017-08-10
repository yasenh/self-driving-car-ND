//
// Created by projectx on 8/2/17.
//

#ifndef PATH_PLANNING_UTILS_H
#define PATH_PLANNING_UTILS_H

enum LaneSegment {
    kUnknownLane    = -1,
    kLeftLane       = 0,
    kMiddleLane     = 1,
    kRightLane      = 2,
};

enum BehaviorState {
    kChangeLeft  = 0,
    kKeepLane    = 1,
    kChangeRight = 2,
};

const int kHostVehicleId = -1;
const int kLaneWidth     = 4;
const int kTotalLaneNum  = 3;

//Every 0.02s = 20 ms the car will move exactly to the next point
const float kTimeInterval      = 0.02;

//const int kPredictionPtNum     = 100;
//const int kMinTrajectoryPtNum  = 80;

const int kPredictionPtNum     = 50;
const int kMinTrajectoryPtNum  = 40;


const float kLaneD[kTotalLaneNum] = {2.2, 6.0, 9.8};

// mps
const double kSpeedLimit  = 19.5;
const double kSpeedBuffer = 2.0;

const double kMaxDistance = 9999;
const double kMaxSpeed = 9999;
const double kMaxCost = 9999;

const double kChangeLaneFactor = 1.1;
const double kMiddleLaneFactor = 0.9;

const double kDistanceBuffer = 35.0;
const double kSafetyDistance = 20.0;

const int kMinLaneChangeFrame = 100;
const int kMinLaneKeepingTime = 5;

#endif //PATH_PLANNING_UTILS_H
