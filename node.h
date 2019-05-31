#ifndef __NODE_H__
#define __NODE_H__

#include <list>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <Eigen/Dense>
#include "freeze_map.h"
#include "triangle_inequality.h"

class Node
{
 private:
  const Eigen::MatrixXd* initial_A;
  std::list<std::shared_ptr<TriangleInequality>> inequalities;
  std::list<std::shared_ptr<TriangleInequality>> inequalities_post;
  FreezeMap freezes;
  bool executed;

  // Post-execution data
  int branch_i;
  int branch_j;
  double upper_bound;
  double lower_bound;
  Eigen::MatrixXd Y;
  Eigen::VectorXd y;
  Eigen::VectorXd x;

 public:
  // Constructor of root node
  Node(const Eigen::MatrixXd* A);
  // Constructor of child nodes
  Node(const Eigen::MatrixXd* A, 
       FreezeMap f, 
       std::list<std::shared_ptr<TriangleInequality>> ineqs);

  std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> branch(int i, int j);

  std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> branch_on_suggested() {
    return branch(this->branch_i, this->branch_j);
  }

  const std::list<std::shared_ptr<TriangleInequality>>& get_inequalities() 
    const {
    return inequalities;
  }

  const std::list<std::shared_ptr<TriangleInequality>>& get_post_inequalities() 
    const {
    return inequalities_post;
  }

  void set_post_inequalities(std::list<std::shared_ptr<TriangleInequality>> 
                             ineqs) {
    inequalities_post = ineqs;
  }

  const FreezeMap* get_freeze_map() const { return &freezes; }

  const Eigen::MatrixXd* get_initial_A() const { return initial_A; }

  int get_branch_i() const { return branch_i; }
  int set_branch_i(int bi) { branch_i = bi; }

  int get_branch_j() const { return branch_j; }
  int set_branch_j(int bj) { branch_j = bj; }

  double get_upper_bound() const { return upper_bound; }
  void set_upper_bound(double ub) { upper_bound = ub; }

  double get_lower_bound() const { return lower_bound; }
  void set_lower_bound(double lb) { lower_bound = lb; }

  Eigen::VectorXd get_lower_bound_witness() const { return y; }
  void set_lower_bound_witness(const Eigen::VectorXd& lbw) { y = lbw; }

  bool is_executed() const { return executed; }

  void execute(int num_post_ineqs);
};

#endif  // __NODE_H__
