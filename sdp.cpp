#include <cstdlib>
#include <Eigen/Dense>
#include "fusion.h"
#include "sdp.h"

void sdp(const Eigen::MatrixXd& A,
         const std::list<std::shared_ptr<TriangleInequality>>& inequalities,
         Eigen::MatrixXd& X) {
  int N = A.rows();

  mosek::fusion::Model::t model = new mosek::fusion::Model();

  mosek::fusion::Variable::t X_var =
    model->variable("X", mosek::fusion::Domain::inPSDCone(N));
  model->constraint(X_var->diag(), mosek::fusion::Domain::equalsTo(1.0));

  for (const std::shared_ptr<TriangleInequality>& ineq : inequalities) {
    ineq->add_to_model(model, X_var);
  }

  double* A_ptr = new double[N * N];
  Eigen::Map<Eigen::MatrixXd>(A_ptr, A.rows(), A.cols()) = A;
  std::shared_ptr<monty::ndarray<double, 1>>
    A_monty_ptr(new monty::ndarray<double, 1>(A_ptr, monty::shape(N * N)));
  mosek::fusion::Matrix::t A_mat =
    mosek::fusion::Matrix::dense(N, N, A_monty_ptr);

  model
    ->objective(mosek::fusion::ObjectiveSense::Maximize,
                mosek::fusion::Expr::sum(mosek::fusion::Expr::mulElm(A_mat,
                                                                     X_var)));

  model->solve();

  // Retrieve optimizer
  X = Eigen::Map<Eigen::MatrixXd>(X_var->level().get()->raw(), N, N);

  model->dispose();
}
