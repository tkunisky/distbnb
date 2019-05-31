// Implements logic for choosing a pair of branching indices (i, j) based on
// the primal optimizer matrix X.

#include <cstdlib>
#include <Eigen/Dense>

// Chooses an "easy" branching pair, for which rows i and j of X are as close
// as possible to hypercube vectors.
std::pair<int, int> branch_easy(const Eigen::MatrixXd& X);

// Chooses a "hard" branching pair, where entry X_{ij} is as close as possible
// to zero.
std::pair<int, int> branch_hard(const Eigen::MatrixXd& X);
