#include <random>
#include <cmath>
#include <Eigen/Dense>
#include "round.h"


void round_gw(const Eigen::MatrixXd& Y,
              Eigen::VectorXd& y) {
  int N = Y.rows();

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(Y);
  Eigen::MatrixXd gram_vector_cols =
    solver.eigenvectors() *
    solver.eigenvalues().cwiseSqrt().asDiagonal();

  std::default_random_engine generator;
  std::normal_distribution<double> distribution;
  auto gaussian = [&] (int) {return distribution(generator);};

  y =
    (gram_vector_cols * Eigen::VectorXd::NullaryExpr(N, gaussian))
    .cwiseSign();
}

void round_local(const Eigen::MatrixXd& A, Eigen::VectorXd& y) {
  int N = A.rows();

  double current_value = y.transpose() * A * y;
  double prev_value = -1e10;

  int i = 0;

  // While an improvement is achieved
  while (current_value >= prev_value + ROUND_LOCAL_THRESHOLD) {
    for (int i = 0; i < N; i++) {
      // Calculate effect of flipping y(i)
      double diff =
        4.0 * (-copysign(1.0, y(i)) * y.dot(A.col(i)) + A(i, i));
      // Apply if positive
      if (diff > 0) {
        y(i) *= -1;
        current_value += diff;
      }
    }
    prev_value = current_value;
    current_value = y.transpose() * A * y;
  }
}

void round_iter(const Eigen::MatrixXd& A,
                const Eigen::MatrixXd& Y,
                Eigen::VectorXd& y,
                double alpha) {
  int N = A.rows();

  Eigen::MatrixXd Y_mod = Y;
  Eigen::VectorXd previous_y = Eigen::VectorXd(N);

  y = Eigen::VectorXd(N);

  round_gw(Y_mod, y);
  double previous_obj = -std::numeric_limits<double>::infinity();
  double obj = y.dot(A * y);

  while (obj > previous_obj) {
    previous_y = y;

    Y_mod = (1 - alpha) * Y_mod + alpha * y * y.transpose();
    round_gw(Y_mod, y);
    round_local(A, y);

    previous_obj = obj;
    obj = y.dot(A * y);
  }

  // `previous_y` is the last iteration that made an improvement
  y = previous_y;
}
