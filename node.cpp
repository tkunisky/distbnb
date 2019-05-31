#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <Eigen/Dense>
#include "freeze_map.h"
#include "branch.h"
#include "node.h"
#include "sdp.h"
#include "round.h"
#include "brute_force.h"
#include "triangle_inequality.h"


Node::Node(const Eigen::MatrixXd* A,
           FreezeMap f,
           std::list<std::shared_ptr<TriangleInequality>> ineqs) {
  this->initial_A = A;
  this->freezes = f;
  this->executed = false;
  this->upper_bound = std::numeric_limits<double>::infinity();
  this->inequalities_post = std::list<std::shared_ptr<TriangleInequality>>{};
  this->inequalities = ineqs;
}


Node::Node(const Eigen::MatrixXd* A) {
  initial_A = A;
  int N = (*A).rows();

  inequalities = std::list<std::shared_ptr<TriangleInequality>>{};
  inequalities_post = std::list<std::shared_ptr<TriangleInequality>>{};

  freezes = FreezeMap{};
  for (int i = 0; i < N; i++) {
    freezes.insert(std::make_pair(i, std::map<int, int>()));
  }

  upper_bound = std::numeric_limits<double>::infinity();
}

std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> Node::branch(int i,
                                                                     int j) {
  std::pair<FreezeMap, FreezeMap> freeze_maps =
    FreezeMap_branch(&freezes, i, j);

  // BranchMap of adding xi = xj
  FreezeMap freezes_pos = freezes;
  freezes_pos[j].insert(freezes_pos[i].begin(), freezes_pos[i].end());
  freezes_pos[j].insert(std::make_pair(i, +1));
  freezes_pos.erase(i);

  // BranchMap of adding xi = -xj
  FreezeMap freezes_neg = freezes;
  std::for_each(freezes_neg[i].begin(),
                freezes_neg[i].end(),
                [](std::pair<const int, int>& p) {
                  p.second = -p.second;
                });
  freezes_neg[j].insert(freezes_neg[i].begin(), freezes_neg[i].end());
  freezes_neg[j].insert(std::make_pair(i, -1));
  freezes_neg.erase(i);

  std::shared_ptr<Node> pnode_pos(new Node(initial_A,
                                           freezes_pos,
                                           inequalities_post));
  std::shared_ptr<Node> pnode_neg(new Node(initial_A,
                                           freezes_neg,
                                           inequalities_post));

  // Propagate current node's upper bound to children
  pnode_pos->set_upper_bound(upper_bound);
  pnode_neg->set_upper_bound(upper_bound);

  return std::make_pair(pnode_pos, pnode_neg);
}

void Node::execute(int num_post_ineqs) {
  // Compute number of active variables
  int N = initial_A->rows();
  int M = N - FreezeMap_num_frozen(&freezes);

  // Compute "effective" A matrix at this node
  Eigen::MatrixXd node_A = FreezeMap_transform_matrix(*initial_A, &freezes);

  if (M <= 6) {
    Eigen::VectorXd optimizer(M);
    brute_force(node_A, optimizer);
    double value = optimizer.dot(node_A * optimizer);

    this->upper_bound = value;
    this->lower_bound = value;
    this->y = optimizer;
    this->Y = optimizer * optimizer.transpose();
  } else {
    // Run the SDP for upper bound

    // Translate inequalities to local indices
    std::map<int, int> key_to_ix;
    int key_ix = 0;
    for (FreezeMap::const_iterator it=freezes.begin(); it != freezes.end(); ++it) {
      key_to_ix[it->first] = key_ix;
      key_ix++;
    }

    std::list<std::shared_ptr<TriangleInequality>> converted_inequalities{};
    for (std::shared_ptr<TriangleInequality> ineq : this->inequalities) {
      converted_inequalities.push_back(std::shared_ptr<TriangleInequality>(new TriangleInequality(key_to_ix[ineq->get_i()], key_to_ix[ineq->get_j()], key_to_ix[ineq->get_k()], ineq->get_sign_ij(), ineq->get_sign_ik(), ineq->get_sign_jk())));
    }

    Y = Eigen::MatrixXd(M, M);
    sdp(node_A, converted_inequalities, Y);
    this->upper_bound = (Y * node_A).trace();

    // Run rounding for lower bound
    round_iter(node_A, Y, y, 0.5);
    this->lower_bound = y.dot(node_A * y);
  }

  std::pair<int, int> branch_pair = branch_easy(Y);
  int ix = 0;
  for (FreezeMap::const_iterator it=freezes.begin();
       it != freezes.end();
       ++it) {
    int i = it->first;

    if (ix == branch_pair.first) {
      this->branch_i = i;
    } else if (ix == branch_pair.second) {
      this->branch_j = i;
    }

    ix++;
  }
  if (this->branch_i > this->branch_j) {
    int tmp = branch_i;
    branch_i = branch_j;
    branch_j = tmp;
  }

  std::set<int> avoid_ixs = { branch_pair.first, branch_pair.second };
  choose_best_ineqs(Y,
                    this->freezes,
                    avoid_ixs,
                    num_post_ineqs,
                    this->inequalities_post);

  executed = true;
}
