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

double distance(double x1, double y1, double x2, double y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

int ClosestWaypoint(double x, double y, vector<double> maps_x, vector<double> maps_y) {
    double closestLen = 100000; //large number
    int closestWaypoint = 0;

    for(int i = 0; i < maps_x.size(); i++) {
        double map_x = maps_x[i];
        double map_y = maps_y[i];
        double dist = distance(x,y,map_x,map_y);
        if(dist < closestLen) {
            closestLen = dist;
            closestWaypoint = i;
        }
    }
    return closestWaypoint;
}

int NextWaypoint(double x, double y, double theta, vector<double> maps_x, vector<double> maps_y) {
    int closestWaypoint = ClosestWaypoint(x,y,maps_x,maps_y);
    double map_x = maps_x[closestWaypoint];
    double map_y = maps_y[closestWaypoint];
    double heading = atan2((map_y - y), (map_x - x));
    double angle = abs(theta-heading);
    if(angle > pi()/4) {
        closestWaypoint++;
    }
    return closestWaypoint;
}

// Transform from Cartesian x,y coordinates to Frenet s,d coordinates
vector<double> getFrenet(double x, double y, double theta, vector<double> maps_x, vector<double> maps_y) {
    int next_wp = NextWaypoint(x,y, theta, maps_x,maps_y);

    int prev_wp;
    prev_wp = next_wp-1;
    if(next_wp == 0) {
        prev_wp  = maps_x.size() - 1;
    }

    double n_x = maps_x[next_wp]-maps_x[prev_wp];
    double n_y = maps_y[next_wp]-maps_y[prev_wp];
    double x_x = x - maps_x[prev_wp];
    double x_y = y - maps_y[prev_wp];

    // find the projection of x onto n
    double proj_norm = (x_x * n_x + x_y * n_y) / (n_x * n_x + n_y * n_y);
    double proj_x = proj_norm * n_x;
    double proj_y = proj_norm * n_y;

    double frenet_d = distance(x_x, x_y, proj_x, proj_y);

    //see if d value is positive or negative by comparing it to a center point

    double center_x = 1000-maps_x[prev_wp];
    double center_y = 2000-maps_y[prev_wp];
    double centerToPos = distance(center_x,center_y,x_x,x_y);
    double centerToRef = distance(center_x,center_y,proj_x,proj_y);

    if(centerToPos <= centerToRef) {
        frenet_d *= -1;
    }

    // calculate s value
    double frenet_s = 0;
    for(int i = 0; i < prev_wp; i++) {
        frenet_s += distance(maps_x[i],maps_y[i],maps_x[i+1],maps_y[i+1]);
    }

    frenet_s += distance(0,0,proj_x,proj_y);

    return {frenet_s, frenet_d};
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
    // The max s value before wrapping around the track back to 0
    double max_s = 6945.554;

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

    int frame_index = 0;
    Vehicle host_vehicle(kHostVehicleId);
    TrajectoryPlanner trajectory_planner(map_waypoints_x, map_waypoints_y, map_waypoints_dx, map_waypoints_dy, map_waypoints_s);
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

                    // Main car's localization Data

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
                     *  Start Test
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
                     * TODO: Another more efficient way is to save every start & end status!!!
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

                        behavior_planner.UpdateLocalization(host_vehicle, fusion_vehicles);
                        behavior_planner.UpdateState();

                        double target_delta_s = behavior_planner.GetTargetDeltaS();
                        double target_speed = behavior_planner.GetTargetSpeed();


                        start_s = host_vehicle.GetStateS();
                        end_s = {end_path_s + target_delta_s, target_speed, 0.0};



                        double target_d = behavior_planner.GetTargetD();
                        start_d = host_vehicle.GetStateD();
                        end_d = {target_d, 0.0, 0.0};

                        int target_prediction_pt_num = behavior_planner.GetTargetPredictionPtNum();
                        trajectory_planner.UpdateTrajectory(start_s, start_d, end_s, end_d, target_prediction_pt_num);
                        host_vehicle.UpdateState(end_s, end_d);

                        //std::cout << "target_delta_s = " << target_delta_s << " target_vel = " << target_speed << std::endl;
                        //std::cout << "start_d = " << start_d[0] << " target_d = " << start_d[0] << std::endl;

                        std::cout << "target_d in main = " << target_d << std::endl;
                        std::cout << "HOST_VEH_S = " << host_vehicle.GetS() << std::endl;

                        std::cout << "***************************************" << std::endl;
                    }







                    /**
                     *  End Test
                     *
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