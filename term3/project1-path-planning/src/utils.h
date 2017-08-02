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

const int kHostVehicleId = -1;
const int kLaneWidth     = 4;
const int kTotalLaneNum  = 3;

//Every 0.02s = 20 ms the car will move exactly to the next point
const double kTimeInterval      = 0.02;
const double kPredictionTime    = 2.0;
const int kPredictionPtNum      = static_cast<int>(kPredictionTime / kTimeInterval);

#endif //PATH_PLANNING_UTILS_H
