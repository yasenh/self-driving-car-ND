//
// Created by projectx on 8/1/17.
//

#ifndef PATH_PLANNING_VEHICLE_H
#define PATH_PLANNING_VEHICLE_H


class Vehicle {
public:
    /**
    * Constructor
    */
    Vehicle();
    Vehicle(int id, double s, double d, double vel);

    /**
    * Destructor
    */
    virtual ~Vehicle();

    //void update_state(double s, double d, double vel);
    int GetLane();

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

};


#endif //PATH_PLANNING_VEHICLE_H
