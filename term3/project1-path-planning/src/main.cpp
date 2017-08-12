#include <fstream>
#include <math.h>
#include <uWS/uWS.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "json.hpp"
#include "spline.h"

#include "vehicle.h"
#include "trajectory_planner.h"
#include "behavior_planner.h"
#include "utils.h"

using namespace std;


// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
string hasData(string s) {
    auto found_null = s.find("null");
    auto b1 = s.find_first_of("[");
    auto b2 = s.find_first_of("}");
    if (found_null != string::npos) {
        return "";
    } else if (b1 != string::npos && b2 != string::npos) {
        return s.substr(b1, b2 - b1 + 2);
    }
    return "";
}


int main() {
    uWS::Hub h;

    // Load up map values for waypoint's x,y,s and d normalized normal vectors
    vector<double> map_waypoints_x;
    vector<double> map_waypoints_y;
    vector<double> map_waypoints_s;
    vector<double> map_waypoints_dx;
    vector<double> map_waypoints_dy;

    // Waypoint map to read from
    string map_file_ = "../data/highway_map.csv";

    ifstream in_map_(map_file_.c_str(), ifstream::in);

    string line;
    while (getline(in_map_, line)) {
        istringstream iss(line);
        double x, y;
        float s, d_x, d_y;
        iss >> x;
        iss >> y;
        iss >> s;
        iss >> d_x;
        iss >> d_y;
        map_waypoints_x.push_back(x);
        map_waypoints_y.push_back(y);
        map_waypoints_s.push_back(s);
        map_waypoints_dx.push_back(d_x);
        map_waypoints_dy.push_back(d_y);
    }

    // add the max_s point to deal with multiple runs
    map_waypoints_x.push_back(map_waypoints_x[0]);
    map_waypoints_y.push_back(map_waypoints_y[0]);
    map_waypoints_s.push_back(kMapMaxS);
    map_waypoints_dx.push_back(map_waypoints_dx[0]);
    map_waypoints_dy.push_back(map_waypoints_dy[0]);

    int frame_index = 0;
    // initialize host vehicle
    Vehicle host_vehicle(kHostVehicleId);
    // initialize trajectory planner
    TrajectoryPlanner trajectory_planner(map_waypoints_x, map_waypoints_y, map_waypoints_dx, map_waypoints_dy, map_waypoints_s);
    // initialize behavior planner
    BehaviorPlanner behavior_planner;

    h.onMessage([&host_vehicle, &trajectory_planner, &behavior_planner, &frame_index](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
        // "42" at the start of the message means there's a websocket message event.
        // The 4 signifies a websocket message
        // The 2 signifies a websocket event
        //auto sdata = string(data).substr(0, length);
        //cout << sdata << endl;
        if (length && length > 2 && data[0] == '4' && data[1] == '2') {

            auto s = hasData(data);

            if (s != "") {
                auto j = json::parse(s);

                string event = j[0].get<string>();

                if (event == "telemetry") {
                    // j[1] is the data JSON object
                    /**
                     * ["x"] The car's x position in map coordinates
                     * ["y"] The car's y position in map coordinates
                     * ["s"] The car's s position in frenet coordinates
                     * ["d"] The car's d position in frenet coordinates
                     * ["yaw"] The car's yaw angle in the map
                     * ["speed"] The car's speed in MPH (1 mph = 0.44704 mps)
                     */

                    /**
                     * The d vector can be used to reference lane positions and each lane is 4 m.
                     * The middle points of each lane in the direction of the right hand are 2m, 6m, 10m
                     */

                    double car_x = j[1]["x"];
                    double car_y = j[1]["y"];
                    double car_s = j[1]["s"];
                    double car_d = j[1]["d"];
                    double car_yaw = j[1]["yaw"];
                    double car_speed = j[1]["speed"];

                    // Previous path data given to the Planner
                    vector<double> previous_path_x = j[1]["previous_path_x"];
                    vector<double> previous_path_y = j[1]["previous_path_y"];

                    // Previous path's end s and d values
                    double end_path_s = j[1]["end_path_s"];
                    double end_path_d = j[1]["end_path_d"];

                    // Sensor Fusion Data, a list of all other cars on the same side of the road.
                    auto sensor_fusion = j[1]["sensor_fusion"];

                    size_t previous_path_size = previous_path_x.size();

                    /**
                     *  Start
                     */
                    vector<double> next_x_vals, next_y_vals;
                    next_x_vals = previous_path_x;
                    next_y_vals = previous_path_y;

                    // Update host vehicle information
                    host_vehicle.UpdatePositionVelocity(car_s, car_d, car_speed);

                    // Upload current trajectory to trajectory planner
                    trajectory_planner.LoadCurrentTrajectory(next_x_vals, next_y_vals);


                    vector<double> start_s, start_d;
                    vector<double> end_s, end_d;

                    // If the host vehicle is about to start
                    if (frame_index == 0) {
                        // add more points to reduce jerk
                        int n = 225;

                        double target_s = host_vehicle.GetS() + 40.0;
                        double target_speed = kSpeedLimit;

                        start_s = {host_vehicle.GetS(), host_vehicle.GetVel(), 0};
                        end_s = {target_s, target_speed, 0.0};

                        start_d = {host_vehicle.GetD(), 0.0, 0.0};
                        end_d = {kLaneD[host_vehicle.GetLane()], 0.0, 0.0};

                        trajectory_planner.UpdateTrajectory(start_s, start_d, end_s, end_d, n);
                        host_vehicle.UpdateState(end_s, end_d);
                    }

                    /**
                     *
                     * If we don't have enough way points from previous path
                     * then we need to predict new trajectory way points
                     *
                     * However, these prediction will be add to the remaining trajectory
                     * In this case, we should use last point to predict
                     * end_path_s and end_path_d
                     *
                     * By the way, we need to wait until the lane change done before next behavior
                     *
                     */

                    else if (previous_path_size < kMinTrajectoryPtNum && host_vehicle.GetLane() == behavior_planner.GetTargetLane()) {
                        // collect sensor fusion results
                        vector<Vehicle> fusion_vehicles;
                        for (size_t i = 0; i < sensor_fusion.size(); i++) {
                            int fusion_id = sensor_fusion[i][0];
                            double fusion_vx = sensor_fusion[i][3];
                            double fusion_vy = sensor_fusion[i][4];
                            // mps, not mph!!!
                            double fusion_v = sqrt(fusion_vx * fusion_vx + fusion_vy * fusion_vy);
                            double fusion_s = sensor_fusion[i][5];
                            double fusion_d = sensor_fusion[i][6];

                            Vehicle veh(fusion_id, fusion_s, fusion_d, fusion_v);
                            fusion_vehicles.push_back(veh);
                        }

                        // update all localization information for behavior planner
                        behavior_planner.UpdateLocalization(host_vehicle, fusion_vehicles);
                        // make decision
                        behavior_planner.UpdateState();

                        // follow behavior planner's decision
                        double target_delta_s = behavior_planner.GetTargetDeltaS();
                        double target_s = end_path_s + target_delta_s;
                        double target_speed = behavior_planner.GetTargetSpeed();

                        start_s = host_vehicle.GetStateS();
                        end_s = {target_s, target_speed, 0.0};

                        // if host vehicle runs more than 1 lap
                        if (start_s[0] > end_s[0]) {
                            end_s[0] += kMapMaxS;
                        }

                        double target_d = behavior_planner.GetTargetD();
                        start_d = host_vehicle.GetStateD();
                        end_d = {target_d, 0.0, 0.0};

                        int target_prediction_pt_num = behavior_planner.GetTargetPredictionPtNum();

                        // calculate trajectory
                        trajectory_planner.UpdateTrajectory(start_s, start_d, end_s, end_d, target_prediction_pt_num);
                        host_vehicle.UpdateState(end_s, end_d);

                        std::cout << "Host_s = " << host_vehicle.GetS() << std::endl;
                        std::cout << "***************************************" << std::endl;
                    }

                    /**
                     *  End
                     */
                    json msgJson;
                    msgJson["next_x"] = trajectory_planner.GetTrajectoryX();
                    msgJson["next_y"] = trajectory_planner.GetTrajectoryY();

                    auto msg = "42[\"control\","+ msgJson.dump()+"]";

                    //this_thread::sleep_for(chrono::milliseconds(1000));
                    ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);

                }
            } else {
                // Manual driving
                std::string msg = "42[\"manual\",{}]";
                ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
            }
        }

        frame_index++;

    });

    // We don't need this since we're not using HTTP but if it's removed the
    // program
    // doesn't compile :-(
    h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data,
                       size_t, size_t) {
        const std::string s = "<h1>Hello world!</h1>";
        if (req.getUrl().valueLength == 1) {
            res->end(s.data(), s.length());
        } else {
            // i guess this should be done more gracefully?
            res->end(nullptr, 0);
        }
    });

    h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
        std::cout << "Connected!!!" << std::endl;
    });

    h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code,
                           char *message, size_t length) {
        ws.close();
        std::cout << "Disconnected" << std::endl;
    });

    int port = 4567;
    if (h.listen(port)) {
        std::cout << "Listening to port " << port << std::endl;
    } else {
        std::cerr << "Failed to listen to port" << std::endl;
        return -1;
    }
    h.run();
}