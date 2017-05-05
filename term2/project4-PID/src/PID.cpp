#include "PID.h"

using namespace std;

/*
* TODO: Complete the PID class.
*/

PID::PID() {
    p_error_ = i_error_ = d_error_ = 0;
    prev_cte_ = 0;
}

PID::~PID() {}

void PID::Init(double Kp, double Ki, double Kd) {
    Kp_ = Kp;
    Ki_ = Ki;
    Kd_ = Kd;
}

void PID::UpdateError(double cte) {
    p_error_ = cte;

    d_error_ = cte - prev_cte_;
    prev_cte_ = cte;

    i_error_ += cte;

}

double PID::TotalError() {
    double total_error = -Kp_ * p_error_ - Ki_ * i_error_ - Kd_ * d_error_;
    return total_error;
}

