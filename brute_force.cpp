#include <Eigen/Dense>


void brute_force(const Eigen::MatrixXd& A, Eigen::VectorXd& result) {
  int N = A.rows();

  double best_value = -std::numeric_limits<double>::infinity();

  // Enumerate candidates by binary expansion of numbers between 0 and 2^N - 1.
  int two_pow_N = 1;
  for (int i = 0; i < N; i++) {
    two_pow_N *= 2;
  }
  for (int i = 0; i < two_pow_N; i++) {
    // Set candidate to have +/- 1 entries according to binary expansion of i.
    Eigen::VectorXd candidate(N);
    candidate(N - 1) = +1;
    int k = i / 2;
    for (int j = 0; j < N - 1; j++) {
      if (k % 2) {
        candidate(j) = +1;
      } else {
        candidate(j) = -1;
      }

      k = k / 2;
    }

    double value = candidate.dot(A * candidate);
    if (value > best_value) {
      best_value = value;
      result = candidate;
    }
  }
}
