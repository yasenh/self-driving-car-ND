//
// Created by projectx on 8/1/17.
//

#ifndef PATH_PLANNING_VEHICLE_H
#define PATH_PLANNING_VEHICLE_H


#include <vector>

class Vehicle {
public:
    /**
     * Constructor
     */
    Vehicle();

    Vehicle(int id);

    Vehicle(int id, double s, double d, double vel);

    /**
     * Destructor
     */
    virtual ~Vehicle();

    void UpdateState(double s, double d, double vel);

    int GetLane();

    double GetS() {
        return s_;
    }

    double GetD() {
        return d_;
    }

    double GetVel() {
        return vel_;
    }

    std::vector<double> GetStateS() {
        return state_s_;
    }

    void SetStateS(std::vector<double> state_s) {
        state_s_ = state_s;
    }

    std::vector<double> GetStateD() {
        return state_d_;
    }

    void SetStateD(std::vector<double> state_d) {
        state_d_ = state_d;
    }



private:
    void LaneSegmentation();

    // Vehicle id from sensor fusion
    int id_;
    // Frenet position
    double s_, d_;
    // The vehicle speed in MPH
    double vel_;
    // LEFT = 0, MIDDLE = 1, RIGHT = 2
    int lane_idx_;

    std::vector<double> state_s_, state_d_;

};


#endif //PATH_PLANNING_VEHICLE_H
