#ifndef MPC_H
#define MPC_H

#include <vector>
#include "Eigen-3.3/Eigen/Core"

using namespace std;

class MPC {
public:
    MPC();

    virtual ~MPC();

    // Solve the model given an initial state and polynomial coefficients.
    // Return the first actuatotions.
    vector<double> Solve(Eigen::VectorXd state, Eigen::VectorXd coeffs);

    vector<double> GetPrectionX() {
        return prediction_x_;
    }

    vector<double> GetPrectionY() {
        return prediction_y_;
    }

private:
    vector<double> prediction_x_, prediction_y_;
};

#endif /* MPC_H */
