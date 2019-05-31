#include <Eigen/Dense>

// Threshold for improvement in local rounding search.
const double ROUND_LOCAL_THRESHOLD = 1e-6;

// Performs the Goemans-Williamson random hyperplane rounding.
void round_gw(const Eigen::MatrixXd& Y, Eigen::VectorXd& y);

// Does a greedy search for a local maximum of y'Ay.
void round_local(const Eigen::MatrixXd& A, Eigen::VectorXd& y);

// Iterates the Goemans-Williamson rounding, greedy search, and adjustment
// of the pseudomoment matrix.
void round_iter(const Eigen::MatrixXd& A,
                const Eigen::MatrixXd& Y,
                Eigen::VectorXd& y,
                double alpha);
