#include "kalman_filter.h"

KalmanFilter::KalmanFilter() {}

KalmanFilter::~KalmanFilter() {}

void KalmanFilter::Init(Eigen::VectorXd &x_in, Eigen::MatrixXd &P_in, Eigen::MatrixXd &F_in,
                        Eigen::MatrixXd &H_in, Eigen::MatrixXd &R_in, Eigen::MatrixXd &Q_in) {
    x_ = x_in;
    P_ = P_in;
    F_ = F_in;
    H_ = H_in;
    R_ = R_in;
    Q_ = Q_in;
}

void KalmanFilter::Predict() {
    x_ = F_ * x_;
    Eigen::MatrixXd Ft = F_.transpose();
    P_ = F_ * P_ * Ft + Q_;
}

void KalmanFilter::Update(const Eigen::VectorXd &z) {
    Eigen::VectorXd z_pred = H_ * x_;
    Eigen::VectorXd y = z - z_pred;
    Eigen::MatrixXd Ht = H_.transpose();
    Eigen::MatrixXd S = H_ * P_ * Ht + R_;
    Eigen::MatrixXd Si = S.inverse();
    Eigen::MatrixXd PHt = P_ * Ht;
    Eigen::MatrixXd K = PHt * Si;

    //new estimate
    x_ = x_ + (K * y);
    long x_size = x_.size();
    Eigen::MatrixXd I = Eigen::MatrixXd::Identity(x_size, x_size);
    P_ = (I - K * H_) * P_;
}

void KalmanFilter::UpdateEKF(const Eigen::VectorXd &z) {
    if(z[0] < 0.001) {
        std::cout << "Skipping radar measurement with very small ro because theta and ro_dot will not be well-defined" << std::endl;
        return;
    }

    double &px = x_[0];
    double &py = x_[1];
    double &vx = x_[2];
    double &vy = x_[3];

    double c1 = px*px + py*py;
    double c2 = sqrt(c1);
    double c3 = (vx * py - vy * px) / (c1 * c2);

    if(c1 < 0.0001) {
        std::cout << "State too close to 0,0 for comparison to radar measurement. Re-initializing to radar measurement." << std::endl;
        x_[0] = z[0] * sin(z[1]);
        x_[1] = z[0] * cos(z[1]);
        return;
    }


    Eigen::VectorXd z_pred = Eigen::VectorXd(3);
    z_pred << c2, atan2(py,px), (px * vx + py * vy) / c2;

    Eigen::VectorXd y = z - z_pred;

    while(y[1] > M_PI) {
        y[1] -= 2 * M_PI;
    }
    while(y[1] < -M_PI) {
        y[1] += 2 * M_PI;
    }

//
//    //compute the Jacobian matrix
//    H_ <<    px/c2,  py/c2,     0,     0,
//            -py/c1,  px/c1,     0,     0,
//             py*c3, -px*c3, px/c2, py/c2;

    Eigen::MatrixXd Ht = H_.transpose();
    Eigen::MatrixXd S = H_ * P_ * Ht + R_;
    Eigen::MatrixXd Si = S.inverse();
    Eigen::MatrixXd PHt = P_ * Ht;
    Eigen::MatrixXd K = PHt * Si;

    //new estimate
    x_ = x_ + (K * y);
    long x_size = x_.size();
    Eigen::MatrixXd I = Eigen::MatrixXd::Identity(x_size, x_size);
    P_ = (I - K * H_) * P_;
}
