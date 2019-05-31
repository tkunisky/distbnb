// Implements brute force solution by enumeration of the +/- 1 quadratic
// optimization problem. (Only tractable for small instances.)

#include <Eigen/Dense>

void brute_force(const Eigen::MatrixXd& A, Eigen::VectorXd& result);
