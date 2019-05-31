#include <cstdlib>
#include <Eigen/Dense>
#include <limits>
#include <utility>
#include "branch.h"


std::pair<int, int> branch_easy(const Eigen::MatrixXd& X) {
  int N = X.rows();

  Eigen::VectorXd v =
    (X.cwiseAbs() - Eigen::MatrixXd::Constant(N, N, 1.0))
    .cwiseAbs2()
    .rowwise()
    .sum();
  int best_i = 0;
  int best_j = 0;
  v.minCoeff(&best_i);
  v[best_i] = std::numeric_limits<double>::infinity();
  v.minCoeff(&best_j);

  return std::make_pair(best_i, best_j);
}


std::pair<int, int> branch_hard(const Eigen::MatrixXd& X) {
  int N = X.rows();

  Eigen::Index best_i = 0;
  Eigen::Index best_j = 0;
  X.cwiseAbs().minCoeff(&best_i, &best_j);

  return std::make_pair(best_i, best_j);
}
