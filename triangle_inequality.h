#ifndef __TRIANGLE_INEQUALITY_H__
#define __TRIANGLE_INEQUALITY_H__

#include <list>
#include <cstdlib>
#include <memory>
#include <set>
#include <Eigen/Dense>
#include "fusion.h"
#include "freeze_map.h"


class TriangleInequality 
{
 private:
  int i;
  int j;
  int k;
  int sign_ij;
  int sign_ik;
  int sign_jk;

 public:
  TriangleInequality(int i, 
                     int j, 
                     int k, 
                     int sign_ij, 
                     int sign_ik, 
                     int sign_jk);

  TriangleInequality(const int* buffer);

  TriangleInequality(const double* buffer);

  int get_i() { return i; }
  int get_j() { return j; }
  int get_k() { return k; }
  int get_sign_ij() { return sign_ij; }
  int get_sign_ik() { return sign_ik; }
  int get_sign_jk() { return sign_jk; }

  void add_to_model(mosek::fusion::Model::t model, 
                    mosek::fusion::Variable::t X) const;

  double eval(const Eigen::MatrixXd& X) const;

  void serialize(int* buffer) const;

  void serialize(double* buffer) const;
};

void choose_best_ineqs(const Eigen::MatrixXd& X, 
                       const FreezeMap& freezes,
                       const std::set<int>& avoid_ixs,
                       int M, 
                       std::list<std::shared_ptr<TriangleInequality>>& ineqs);

#endif  // __TRIANGLE_INEQUALITY_H__
