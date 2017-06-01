#include <math.h>
#include <uWS/uWS.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include "Eigen-3.3/Eigen/Core"
#include "Eigen-3.3/Eigen/QR"
#include "MPC.h"
#include "json.hpp"

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
    auto b2 = s.rfind("}]");
    if (found_null != string::npos) {
        return "";
    } else if (b1 != string::npos && b2 != string::npos) {
        return s.substr(b1, b2 - b1 + 2);
    }
    return "";
}

// Evaluate a polynomial.
double polyeval(Eigen::VectorXd coeffs, double x) {
    double result = 0.0;
    for (int i = 0; i < coeffs.size(); i++) {
        result += coeffs[i] * pow(x, i);
    }
    return result;
}

// Fit a polynomial.
// Adapted from
// https://github.com/JuliaMath/Polynomials.jl/blob/master/src/Polynomials.jl#L676-L716
Eigen::VectorXd polyfit(Eigen::VectorXd xvals, Eigen::VectorXd yvals, int order) {
    assert(xvals.size() == yvals.size());
    assert(order >= 1 && order <= xvals.size() - 1);
    Eigen::MatrixXd A(xvals.size(), order + 1);

    for (int i = 0; i < xvals.size(); i++) {
        A(i, 0) = 1.0;
    }

    for (int j = 0; j < xvals.size(); j++) {
        for (int i = 0; i < order; i++) {
            A(j, i + 1) = A(j, i) * xvals(j);
        }
    }

    auto Q = A.householderQr();
    auto result = Q.solve(yvals);
    return result;
}

const int kLatencyMs = 100;
const double kLf = 2.67;

int main() {
    uWS::Hub h;

    // MPC is initialized here!
    MPC mpc;

    h.onMessage([&mpc](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length,
                       uWS::OpCode opCode) {
        // "42" at the start of the message means there's a websocket message event.
        // The 4 signifies a websocket message
        // The 2 signifies a websocket event
        string sdata = string(data).substr(0, length);
        std::cout << "*************************" << std::endl;
        cout << sdata << endl;
        if (sdata.size() > 2 && sdata[0] == '4' && sdata[1] == '2') {
            string s = hasData(sdata);
            if (s != "") {
                auto j = json::parse(s);
                string event = j[0].get<string>();
                if (event == "telemetry") {
                    // j[1] is the data JSON object
                    vector<double> ptsx = j[1]["ptsx"];
                    vector<double> ptsy = j[1]["ptsy"];
                    double px = j[1]["x"];
                    double py = j[1]["y"];
                    double psi = j[1]["psi"];
                    double v = j[1]["speed"];
                    double steer_value = j[1]["steering_angle"];
                    double throttle_value = j[1]["throttle"];

                    // The position information we get is based on global coordinate
                    // we need to convert it to vehicle coordinate before fitting polynomials
                    // in vehicle coordinate x = y = psi = 0
                    // Reference: https://en.wikipedia.org/wiki/Rotation_matrix

                    /**
                     * Global Coordinate
                     *    ^ y
                     *    |
                     *    |
                     *    |
                     *    |
                     *    -----------> x
                     */


                    /**
                     * Vehicle Coordinate
                     * Recall that the x axis always points in the direction of the car’s heading and the y axis points to the left of the car.
                     *                        ^ x_vehicle
                     *                        |
                     *                        |
                     *                        |
                     *                        |
                     *   y_vehicle <-----------
                     */

                    /**
                     *  If we fix the coordinate and rotate point (x,y), counter-clockwise through an angle θ is positive.
                     *  However, the x-y axis is not fixed.
                     *  Instead of rotate points, we could image we are rotating the x-y plane in this case.
                     *  In other words we are rotating the (x,y) in other direction (clockwise) actually,
                     *  so the rotation angle should be θ = -psi
                     */

                    vector<double> wp_x_vehicle, wp_y_vehicle;
                    for(int i = 0; i < ptsx.size(); i++) {
                        double x = ptsx[i] - px;
                        double y = ptsy[i] - py;
                        wp_x_vehicle.push_back(x * cos(-psi) - y * sin(-psi));
                        wp_y_vehicle.push_back(x * sin(-psi) + y * cos(-psi));
                    }
                    psi = 0;
                    // convert std::vetcor to Eigen::VectorXd
                    Eigen::VectorXd wp_x_vehicle_vec, wp_y_vehicle_vec;
                    wp_x_vehicle_vec = Eigen::VectorXd::Map(wp_x_vehicle.data(), wp_x_vehicle.size());
                    wp_y_vehicle_vec = Eigen::VectorXd::Map(wp_y_vehicle.data(), wp_y_vehicle.size());

                    /**
                      *  Implementation question 3 - Polynomial Fitting and MPC Preprocessing
                      *
                      *  First of all, we convert waypoints from global to local
                      *  then we implement polynomial fitting in order 3
                      */

                    Eigen::VectorXd coeffs = polyfit(wp_x_vehicle_vec, wp_y_vehicle_vec, 3);

                    // The cross track error is calculated by evaluating at polynomial at x, f(x)
                    // and subtracting y.
                    double cte = polyeval(coeffs, 0);

                    // epsi = psi - psi_desired
                    // psi_desired can be calculated as the tangential angle of the polynomial f evaluated at x
                    // arctan(f'(x))
                    // y = f(x) = c[0] + c[1] * x + c[2] * x^2 + c[3] * x^3
                    // derivative of c[0] + c[1] * x + c[2] * x^2 + c[3] * x^3 -> c[1] + 2 * c[2] * x + 3 * c[3] * x^2
                    // at point (0, 0) which is vehicle coordinate, x = y = psi = 0
                    double epsi = -atan(coeffs[1]);

                    /**
                     *  Implementation question 4 - Model Predictive Control with Latency
                     *
                     *  This could easily be modeled by a simple dynamic system and incorporated into the vehicle model.
                     */


                    // Because of the latency, the input of MPC should be compensated

                    double dt = kLatencyMs / 1000;
//                    double state_x = v * dt;
//                    double state_y = 0;

                    // convert the velocity into m/s.
                    double state_x = v * cos(psi) * dt * 0.44704 ;
                    double state_y = v * sin(psi) * dt * 0.44704;
                    double state_psi = v * steer_value / kLf * dt;
                    double state_v = v + throttle_value * dt;
                    double state_cte = cte + v * sin(epsi) * dt;
                    double state_epsi = epsi + v * steer_value /kLf * dt;


                    Eigen::VectorXd state(6);
                    //state << 0, 0, 0, v, cte, epsi;
                    //state << state_x, state_y, state_psi, state_v, state_cte, state_epsi;
                    state << state_x, state_y, psi, v, cte, epsi;


                    /*
                    * Calculate steeering angle and throttle using MPC.
                    *
                    * Both are in between [-1, 1].
                    *
                    */

                    /**
                     * Implementation question 4 - The Model
                     * State [x,y,ψ,v] - vehicle position x, y, orientation and velocity in vehicle coordinate
                     * Actuators: [δ,a] - steering angle and throttle value
                     *
                     * Update equations:
                     * x_[t+1] = x[t] + v[t] * cos(psi[t]) * dt
                     * y_[t+1] = y[t] + v[t] * sin(psi[t]) * dt
                     * psi_[t+1] = psi[t] + v[t] / Lf * delta[t] * dt
                     * v_[t+1] = v[t] + a[t] * dt
                     * cte[t+1] = f(x[t]) - y[t] + v[t] * sin(epsi[t]) * dt
                     * epsi[t+1] = psi[t] - psides[t] + v[t] * delta[t] / Lf * dt
                     *
                     */


                    auto vars = mpc.Solve(state, coeffs);

                    // convert the steering angle from radians to the normalized value range[-1,1]
                    steer_value = -vars[0] / deg2rad(25);
                    throttle_value = vars[1];

                    json msgJson;
                    msgJson["steering_angle"] = steer_value;
                    msgJson["throttle"] = throttle_value;

                    //.. add (x,y) points to list here, points are in reference to the vehicle's coordinate system
                    // the points in the simulator are connected by a Green line

                    msgJson["mpc_x"] = mpc.GetPrectionX();
                    msgJson["mpc_y"] = mpc.GetPrectionY();

                    //.. add (x,y) points to list here, points are in reference to the vehicle's coordinate system
                    // the points in the simulator are connected by a Yellow line

                    // TODO: Figure out why ground truth is not stable

                    msgJson["next_x"] = wp_x_vehicle;
                    msgJson["next_y"] = wp_y_vehicle;


                    auto msg = "42[\"steer\"," + msgJson.dump() + "]";
                    std::cout << msg << std::endl;
                    // Latency
                    // The purpose is to mimic real driving conditions where
                    // the car does actuate the commands instantly.
                    //
                    // Feel free to play around with this value but should be to drive
                    // around the track with 100ms latency.
                    //
                    // NOTE: REMEMBER TO SET THIS TO 100 MILLISECONDS BEFORE
                    // SUBMITTING.
                    this_thread::sleep_for(chrono::milliseconds(kLatencyMs));
                    ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
                }
            } else {
                // Manual driving
                std::string msg = "42[\"manual\",{}]";
                ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
            }
        }
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
