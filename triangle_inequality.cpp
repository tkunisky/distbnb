#include <list>
#include <queue>
#include <cstdlib>
#include <set>
#include <Eigen/Dense>
#include "fusion.h"
#include "triangle_inequality.h"


TriangleInequality::TriangleInequality(int i, 
                                       int j, 
                                       int k, 
                                       int sign_ij, 
                                       int sign_ik, 
                                       int sign_jk) {
  this->i = i;
  this->j = j;
  this->k = k;
  this->sign_ij = sign_ij;
  this->sign_ik = sign_ik;
  this->sign_jk = sign_jk;
}

TriangleInequality::TriangleInequality(const int* buffer) {
  this->i = buffer[0];
  this->j = buffer[1];
  this->k = buffer[2];
  this->sign_ij = buffer[3];
  this->sign_ik = buffer[4];
  this->sign_jk = buffer[5];
}

TriangleInequality::TriangleInequality(const double* buffer) {
  this->i = buffer[0];
  this->j = buffer[1];
  this->k = buffer[2];
  this->sign_ij = buffer[3];
  this->sign_ik = buffer[4];
  this->sign_jk = buffer[5];
}

void TriangleInequality::serialize(int* buffer) const {
  buffer[0] = i;
  buffer[1] = j;
  buffer[2] = k;
  buffer[3] = sign_ij;
  buffer[4] = sign_ik;
  buffer[5] = sign_jk;
}

void TriangleInequality::serialize(double* buffer) const {
  buffer[0] = i + 0.5;
  buffer[1] = j + 0.5;
  buffer[2] = k + 0.5;
  buffer[3] = sign_ij * 1.5;
  buffer[4] = sign_ik * 1.5;
  buffer[5] = sign_jk * 1.5;
}

double TriangleInequality::eval(const Eigen::MatrixXd& X) const {
  return sign_ij * X(i, j) + sign_jk * X(j, k) + sign_ik * X(i, k) + 1;
}

void TriangleInequality::add_to_model(mosek::fusion::Model::t model, 
				      mosek::fusion::Variable::t X) const {
  mosek::fusion::Expression::t expr_ij = 
    mosek::fusion::Expr::mul(X->index(i, j), sign_ij);
  mosek::fusion::Expression::t expr_jk =
    mosek::fusion::Expr::mul(X->index(j, k), sign_jk);
  mosek::fusion::Expression::t expr_ik = 
    mosek::fusion::Expr::mul(X->index(i, k), sign_ik);
  mosek::fusion::Expression::t expr_lhs = 
    mosek::fusion::Expr::add(expr_ij, 
                             mosek::fusion::Expr::add(expr_jk, expr_ik));
                             
  model->constraint(expr_lhs, mosek::fusion::Domain::greaterThan(-1.0));
}

void choose_best_ineqs(const Eigen::MatrixXd& X, 
                       const FreezeMap& freezes,
                       const std::set<int>& avoid_ixs,
                       int M, 
                       std::list<std::shared_ptr<TriangleInequality>>& ineqs) {
  int N = X.rows();

  // Defines an ordering in which the _smallest_ triangle inequalities are 
  // those most violated
  auto compare = [&X](const std::shared_ptr<TriangleInequality>& lhs,
		      const std::shared_ptr<TriangleInequality>& rhs) {
    return lhs->eval(X) < rhs->eval(X);
  };

  std::priority_queue<
    std::shared_ptr<TriangleInequality>,
    std::vector<std::shared_ptr<TriangleInequality>>,
    decltype(compare)> ineq_queue(compare);
    
  int sign_choices[4][3] = {
    { +1, +1, +1 },
    { +1, -1, -1 },
    { -1, +1, -1 },
    { -1, -1, +1 }};

  for (int i = 0; i < N; i++) {
    if (avoid_ixs.find(i) != avoid_ixs.end()) {
      continue;
    }
    for (int j = i + 1; j < N; j++) {
      if (avoid_ixs.find(j) != avoid_ixs.end()) {
        continue;
      }
      for (int k = j + 1; k < N; k++) {
        if (avoid_ixs.find(k) != avoid_ixs.end()) {
          continue;
        }
        for (int l = 0; l < 4; l++) {
          std::shared_ptr<TriangleInequality> 
            this_ineq(new TriangleInequality(i, j, k, 
                                             sign_choices[l][0], 
                                             sign_choices[l][1], 
                                             sign_choices[1][2]));
          if (ineq_queue.size() < M) {
            ineq_queue.push(this_ineq);
          } else if (this_ineq->eval(X) < ineq_queue.top()->eval(X)) {
            ineq_queue.pop();
            ineq_queue.push(this_ineq);
          }
        }
      }
    }
  }

  std::vector<int> keys;
  keys.reserve(freezes.size());
  int key_ix = 0;
  for (FreezeMap::const_iterator it=freezes.begin(); it != freezes.end(); ++it) {
    keys[key_ix] = it->first;
    key_ix++;
  }

  ineqs.clear();
  while (!ineq_queue.empty()) {
    int i = keys[ineq_queue.top()->get_i()];
    int j = keys[ineq_queue.top()->get_j()];
    int k = keys[ineq_queue.top()->get_k()];
    ineqs.push_back(std::shared_ptr<TriangleInequality>(new TriangleInequality(i, j, k, ineq_queue.top()->get_sign_ij(), ineq_queue.top()->get_sign_ik(), ineq_queue.top()->get_sign_jk())));
    ineq_queue.pop();
  }
}
