#include <iostream>
#include "FusionEKF.h"

/*
 * Constructor.
 */
FusionEKF::FusionEKF() {
    is_initialized_ = false;
    previous_timestamp_ = 0;

    // initializing matrices

    //measurement covariance
    R_laser_ = Eigen::MatrixXd(2, 2);
    R_radar_ = Eigen::MatrixXd(3, 3);

    R_laser_ << 0.0225, 0,
                0     , 0.0225;

//    R_radar_ << 0.0225, 0     , 0,
//            0     , 0.0225, 0,
//            0     , 0     , 0.0225;

    R_radar_ << 0.5, 0     , 0,
            0     , 0.2, 0,
            0     , 0     , 0.0225;

    //measurement matrix
    H_laser_ = Eigen::MatrixXd(2, 4);
    H_radar_ = Eigen::MatrixXd(3, 4);

    H_laser_ << 1, 0, 0, 0,
            0, 1, 0, 0;

    /**
    TODO:
      * Finish initializing the FusionEKF.
    */
    //create a 4D state vector, we don't know yet the values of the x state
    ekf_.x_ = Eigen::VectorXd(4);

    //state covariance matrix P
    ekf_.P_ = Eigen::MatrixXd(4, 4);
    ekf_.P_ << 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1000, 0,
            0, 0, 0, 1000;

    ekf_.R_ = Eigen::MatrixXd(2, 2);
    ekf_.H_ = Eigen::MatrixXd(2, 4);

    //the initial transition matrix F_
    ekf_.F_ = Eigen::MatrixXd(4, 4);
    ekf_.F_ << 1, 0, 1, 0,
            0, 1, 0, 1,
            0, 0, 1, 0,
            0, 0, 0, 1;

    //set the acceleration noise components
    noise_ax_ = 5;
    noise_ay_ = 5;
}

/**
* Destructor.
*/
FusionEKF::~FusionEKF() {}

void FusionEKF::ProcessMeasurement(const MeasurementPackage &measurement_pack) {
    /*****************************************************************************
     *  Initialization
     ****************************************************************************/
    if (!is_initialized_) {
        /**
         * Initialize the state ekf_.x_ with the first measurement.
         * Create the covariance matrix.
         */

        // First measurement
        std::cout << "EKF: " << std::endl;

        if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
            // Convert radar from polar to cartesian coordinates and initialize state
            float ro = static_cast<float>(measurement_pack.raw_measurements_(0));
            float phi = static_cast<float>(measurement_pack.raw_measurements_(1));
            ekf_.x_ << ro * cos(phi), ro * sin(phi), 0, 0;
        }
        else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
            // Initialize state
            // Set the state with the initial location and zero velocity
            ekf_.x_ << measurement_pack.raw_measurements_[0], measurement_pack.raw_measurements_[1], 0, 0;
        }

        previous_timestamp_ = measurement_pack.timestamp_;
        // done initializing, no need to predict or update
        is_initialized_ = true;
        return;
    }

    /*****************************************************************************
     *  Prediction
     ****************************************************************************/

    /**
     TODO:
       * Update the state transition matrix F according to the new elapsed time.
        - Time is measured in seconds.
       * Update the process noise covariance matrix.
     */

    //compute the time elapsed between the current and previous measurements
    float dt = static_cast<float>((measurement_pack.timestamp_ - previous_timestamp_) / 1000000.0);	//dt - expressed in seconds
    //std::cout << dt << std::endl;
    previous_timestamp_ = measurement_pack.timestamp_;

    float dt_2 = dt * dt;
    float dt_3 = dt_2 * dt;
    float dt_4 = dt_3 * dt;

    //Modify the F matrix so that the time is integrated
    ekf_.F_(0, 2) = dt;
    ekf_.F_(1, 3) = dt;

    //set the process covariance matrix Q
    ekf_.Q_ = Eigen::MatrixXd(4, 4);
    ekf_.Q_ <<  dt_4/4*noise_ax_, 0, dt_3/2*noise_ax_, 0,
            0, dt_4/4*noise_ay_, 0, dt_3/2*noise_ay_,
            dt_3/2*noise_ax_, 0, dt_2*noise_ax_, 0,
            0, dt_3/2*noise_ay_, 0, dt_2*noise_ay_;

    ekf_.Predict();

    /*****************************************************************************
     *  Update
     ****************************************************************************/

    /**
     TODO:
       * Use the sensor type to perform the update step.
       * Update the state and covariance matrices.
     */

    if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
        // Radar updates
        H_radar_ = tools.CalculateJacobian(ekf_.x_);
        ekf_.R_ = R_radar_;
        ekf_.H_ = H_radar_;
        ekf_.UpdateEKF(measurement_pack.raw_measurements_);
    } else {
        // Laser updates
        ekf_.R_ = R_laser_;
        ekf_.H_ = H_laser_;
        ekf_.Update(measurement_pack.raw_measurements_);
    }

    // print the output
    //std::cout << "x_ = " << ekf_.x_ << std::endl;
    //std::cout << "P_ = " << ekf_.P_ << std::endl;
}
