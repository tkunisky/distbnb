#include <list>
#include <Eigen/Dense>
#include "triangle_inequality.h"

// Solves the Goemans-Williamson SDP and retrieves primal and dual optimizers:
//
// Primal
//   maximize     <X, A>
//   subject to   A >= 0, A_{ii} = <A, E_{ii}> = 1
//
// Dual
//   minimize     \sum_{i = 1}^N y_i
//   subject to   diag(y) = \sum_{i = 1}^N y_i E_{ii} - A >= 0
void sdp(const Eigen::MatrixXd& A,
         const std::list<std::shared_ptr<TriangleInequality>>& inequalities,
         Eigen::MatrixXd& X);
