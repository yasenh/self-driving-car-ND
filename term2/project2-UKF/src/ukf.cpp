#include "ukf.h"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 */
UKF::UKF() {
    // initially set to false, set to true in first call of ProcessMeasurement
    is_initialized_ = false;

    // if this is false, laser measurements will be ignored (except during init)
    use_laser_ = true;

    // if this is false, radar measurements will be ignored (except during init)
    use_radar_ = true;

    // State dimension
    n_x_ = 5;

    // Augmented state dimension
    n_aug_ = 7;

    // Sigma point spreading parameter
    lambda_ = 3 - n_x_;

    // initial state vector
    x_ = VectorXd(n_x_);

    // initial covariance matrix
    P_ =  MatrixXd::Identity(n_x_, n_x_);


    // predicted sigma points matrix
    Xsig_pred_ = MatrixXd(n_x_, 2 * n_aug_ + 1);;

    // Weights of sigma points
    weights_ = VectorXd(2 * n_aug_ + 1);

    // Process noise standard deviation longitudinal acceleration in m/s^2
    std_a_ = 3.0;

    // Process noise standard deviation yaw acceleration in rad/s^2
    std_yawdd_ = 0.5;

    // Laser measurement noise standard deviation position1 in m
    std_laspx_ = 0.08;

    // Laser measurement noise standard deviation position2 in m
    std_laspy_ = 0.08;

    // Radar measurement noise standard deviation radius in m
    std_radr_ = 0.3;

    // Radar measurement noise standard deviation angle in rad
    std_radphi_ = 0.0175;

    // Radar measurement noise standard deviation radius change in m/s
    std_radrd_ = 0.15;

    //measurement noise covariance matrix
    R_radar_ = MatrixXd(3, 3);
    R_radar_ <<     std_radr_ * std_radr_, 0                         , 0,
                    0                    , std_radphi_ * std_radphi_ , 0,
                    0                    , 0                         , std_radrd_*std_radrd_;

    R_laser_ = MatrixXd(2, 2);
    R_laser_ <<     std_laspx_ * std_laspx_, 0 ,
                    0                      , std_laspy_ * std_laspy_ ;

    NIS_radar_ = NIS_laser_ = 0;
}

UKF::~UKF() {}

/**
 * @param {MeasurementPackage} meas_package The latest measurement data of
 * either radar or laser.
 */
void UKF::ProcessMeasurement(MeasurementPackage meas_package) {
    /**
    TODO:

    Complete this function! Make sure you switch between lidar and radar
    measurements.
    */

    /*****************************************************************************
     *  Initialization
     ****************************************************************************/
    if (!is_initialized_) {
        /**
         * Initialize the state x_ with the first measurement.
         * Create the covariance matrix.
         */

        // First measurement
        std::cout << "UKF: " << std::endl;

        if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
            // Convert radar from polar to cartesian coordinates and initialize state
            float ro = static_cast<float>(meas_package.raw_measurements_(0));
            float phi = static_cast<float>(meas_package.raw_measurements_(1));
            x_ << ro * cos(phi), ro * sin(phi), 0, 0, 0;
        }
        else if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
            // Initialize state
            // Set the state with the initial location and zero velocity
            x_ << meas_package.raw_measurements_[0], meas_package.raw_measurements_[1], 0, 0, 0;
        }

        previous_timestamp_ = meas_package.timestamp_;
        // Done initializing, no need to predict or update
        is_initialized_ = true;
        return;
    }

    /*****************************************************************************
     * Prediction
     * Update the state transition matrix F according to the new elapsed time. Time is measured in seconds.
     * Update the process noise covariance matrix.
     ****************************************************************************/

    //compute the time elapsed between the current and previous measurements
    float dt = static_cast<float>((meas_package.timestamp_ - previous_timestamp_) / 1000000.0);	//dt - expressed in seconds
    previous_timestamp_ = meas_package.timestamp_;

    while (dt > 0.1) {
        const double delta_t = 0.05;
        Prediction(delta_t);
        dt -= delta_t;
    }
    Prediction(dt);

    /*****************************************************************************
     * Update
     * Use the sensor type to perform the update step.
     * Update the state and covariance matrices.
     ****************************************************************************/

    if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
        //set measurement dimension, radar can measure r, phi, and r_dot
        n_z_ = 3;
    }
    else if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
        //set measurement dimension, laser can measure px, py
        n_z_ = 2;
    }

    //create matrix for sigma points in measurement space
    Zsig_ = MatrixXd(n_z_, 2 * n_aug_ + 1);

    //mean predicted measurement
    z_pred_ = VectorXd(n_z_);

    //measurement covariance matrix S
    S_ = MatrixXd(n_z_, n_z_);


    if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
        PredictRadarMeasurement();
        // update NIS
        NIS_radar_ = (meas_package.raw_measurements_-z_pred_).transpose()*S_.inverse()*(meas_package.raw_measurements_-z_pred_);
    }
    else if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
        PredictLaserMeasurement();
        // update NIS
        NIS_laser_ = (meas_package.raw_measurements_-z_pred_).transpose()*S_.inverse()*(meas_package.raw_measurements_-z_pred_);
    }

    UpdateState(meas_package.raw_measurements_);



}

/**
 * Predicts sigma points, the state, and the state covariance matrix.
 * @param {double} delta_t the change in time (in seconds) between the last
 * measurement and this one.
 */
void UKF::Prediction(double delta_t) {
    /**
    TODO:

    Complete this function! Estimate the object's location. Modify the state
    vector, x_. Predict sigma points, the state, and the state covariance matrix.
    */

    MatrixXd Xsig_aug = MatrixXd(n_aug_, 2 * n_aug_ + 1);
    GenerateAugmentedSigmaPoints(&Xsig_aug);
    SigmaPointPrediction(Xsig_aug, delta_t);
    PredictMeanAndCovariance();
}

/**
 * Updates the state and the state covariance matrix using a laser measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateLidar(MeasurementPackage meas_package) {
    /**
    TODO:

    Complete this function! Use lidar data to update the belief about the object's
    position. Modify the state vector, x_, and covariance, P_.

    You'll also need to calculate the lidar NIS.
    */
}

/**
 * Updates the state and the state covariance matrix using a radar measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateRadar(MeasurementPackage meas_package) {
    /**
    TODO:

    Complete this function! Use radar data to update the belief about the object's
    position. Modify the state vector, x_, and covariance, P_.

    You'll also need to calculate the radar NIS.
    */

    PredictRadarMeasurement();
}

void UKF::GenerateAugmentedSigmaPoints(MatrixXd* Xsig_out) {
    //create augmented mean vector
    VectorXd x_aug = VectorXd(7);

    //create augmented state covariance
    MatrixXd P_aug = MatrixXd(7, 7);

    //create sigma point matrix
    MatrixXd Xsig_aug = MatrixXd(n_aug_, 2 * n_aug_ + 1);

    //create augmented mean state
    x_aug.head(5) = x_;
    x_aug(5) = 0;
    x_aug(6) = 0;

    //create augmented covariance matrix
    P_aug.fill(0.0);
    P_aug.topLeftCorner(5,5) = P_;
    P_aug(5,5) = std_a_ * std_a_;
    P_aug(6,6) = std_yawdd_ * std_yawdd_;

    //create square root matrix
    MatrixXd L = P_aug.llt().matrixL();

    //create augmented sigma points
    Xsig_aug.col(0)  = x_aug;
    for (int i = 0; i< n_aug_; i++) {
        Xsig_aug.col(i+1)        = x_aug + sqrt(lambda_+n_aug_) * L.col(i);
        Xsig_aug.col(i+1+n_aug_) = x_aug - sqrt(lambda_+n_aug_) * L.col(i);
    }

    //write result
    *Xsig_out = Xsig_aug;
}


void UKF::SigmaPointPrediction(MatrixXd Xsig_aug, double delta_t) {
    //predict sigma points
    for (int i = 0; i< 2*n_aug_+1; i++) {
        //extract values for better readability
        double p_x = Xsig_aug(0,i);
        double p_y = Xsig_aug(1,i);
        double v = Xsig_aug(2,i);
        double yaw = Xsig_aug(3,i);
        double yawd = Xsig_aug(4,i);
        double nu_a = Xsig_aug(5,i);
        double nu_yawdd = Xsig_aug(6,i);

        //predicted state values
        double px_p, py_p;

        //avoid division by zero
        if (fabs(yawd) > 0.001) {
            px_p = p_x + v/yawd * ( sin (yaw + yawd*delta_t) - sin(yaw));
            py_p = p_y + v/yawd * ( cos(yaw) - cos(yaw+yawd*delta_t) );
        }
        else {
            px_p = p_x + v*delta_t*cos(yaw);
            py_p = p_y + v*delta_t*sin(yaw);
        }

        double v_p = v;
        double yaw_p = yaw + yawd*delta_t;
        double yawd_p = yawd;

        //add noise
        px_p = px_p + 0.5*nu_a*delta_t*delta_t * cos(yaw);
        py_p = py_p + 0.5*nu_a*delta_t*delta_t * sin(yaw);
        v_p = v_p + nu_a*delta_t;

        yaw_p = yaw_p + 0.5*nu_yawdd*delta_t*delta_t;
        yawd_p = yawd_p + nu_yawdd*delta_t;

        //write predicted sigma point into right column
        Xsig_pred_(0,i) = px_p;
        Xsig_pred_(1,i) = py_p;
        Xsig_pred_(2,i) = v_p;
        Xsig_pred_(3,i) = yaw_p;
        Xsig_pred_(4,i) = yawd_p;
    }
}


void UKF::PredictMeanAndCovariance() {
    // set weights
    double weight_0 = lambda_/(lambda_+n_aug_);
    weights_(0) = weight_0;
    for (int i=1; i<2*n_aug_+1; i++) {  //2n+1 weights
        double weight = 0.5/(n_aug_+lambda_);
        weights_(i) = weight;
    }

    //predicted state mean
    x_.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //iterate over sigma points
        x_ = x_+ weights_(i) * Xsig_pred_.col(i);
    }

    //predicted state covariance matrix
    P_.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //iterate over sigma points

        // state difference
        VectorXd x_diff = Xsig_pred_.col(i) - x_;
        //angle normalization
        while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
        while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;

        P_ = P_ + weights_(i) * x_diff * x_diff.transpose() ;
    }
}


void UKF::PredictRadarMeasurement() {
    //transform sigma points into measurement space
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points

        // extract values for better readibility
        double p_x = Xsig_pred_(0,i);
        double p_y = Xsig_pred_(1,i);
        double v  = Xsig_pred_(2,i);
        double yaw = Xsig_pred_(3,i);

        double v1 = cos(yaw)*v;
        double v2 = sin(yaw)*v;

        // measurement model
        Zsig_(0,i) = sqrt(p_x*p_x + p_y*p_y);                        //r
        Zsig_(1,i) = atan2(p_y,p_x);                                 //phi

        if (Zsig_(0, i) <0.001) {
            Zsig_(2, i) = (p_x * v1 + p_y * v2) / 0.001;        //r_dot
        } else {
            Zsig_(2, i) = (p_x * v1 + p_y * v2) / Zsig_(0, i);  //r_dot;
        }
    }


    z_pred_.fill(0.0);
    for (int i=0; i < 2*n_aug_+1; i++) {
        z_pred_ = z_pred_ + weights_(i) * Zsig_.col(i);
    }


    S_.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points
        //residual
        VectorXd z_diff = Zsig_.col(i) - z_pred_;

        //angle normalization
        while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
        while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

        S_ = S_ + weights_(i) * z_diff * z_diff.transpose();
    }

    S_ = S_ + R_radar_;
}

void UKF::PredictLaserMeasurement() {
    //transform sigma points into measurement space
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points

        // extract values for better readibility
        double p_x = Xsig_pred_(0,i);
        double p_y = Xsig_pred_(1,i);

        // measurement model
        Zsig_(0,i) = p_x;                        //px
        Zsig_(1,i) = p_y;                        //py
    }

    z_pred_.fill(0.0);
    for (int i=0; i < 2*n_aug_+1; i++) {
        z_pred_ = z_pred_ + weights_(i) * Zsig_.col(i);
    }


    S_.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points
        //residual
        VectorXd z_diff = Zsig_.col(i) - z_pred_;

        //angle normalization
        while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
        while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

        S_ = S_ + weights_(i) * z_diff * z_diff.transpose();
    }

    S_ = S_ + R_laser_;
}


void UKF::UpdateState(const VectorXd &z) {
    //create matrix for cross correlation Tc
    MatrixXd Tc = MatrixXd(n_x_, n_z_);

    //calculate cross correlation matrix
    Tc.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points

        //residual
        VectorXd z_diff = Zsig_.col(i) - z_pred_;
        //angle normalization
        while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
        while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

        // state difference
        VectorXd x_diff = Xsig_pred_.col(i) - x_;
        //angle normalization
        while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
        while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;

        Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
    }

    //Kalman gain K;
    MatrixXd K = Tc * S_.inverse();

    //residual
    VectorXd z_diff = z - z_pred_;

    //angle normalization
    while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;

    while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

    //update state mean and covariance matrix
    x_ = x_ + K * z_diff;
    P_ = P_ - K*S_*K.transpose();
}