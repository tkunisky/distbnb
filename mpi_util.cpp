#include <list>
#include <memory>
#include <mpi.h>
#include "node.h"
#include "freeze_map.h"
#include "message.h"
#include "triangle_inequality.h"

void send_work_request(int N,
                       int M,
                       int target_rank,
                       const Node* node,
                       int* node_request_buffer) {
  node_request_buffer[3*N + 6*M] = MESSAGE_WORK;

  const FreezeMap* freezes = node->get_freeze_map();
  FreezeMap_serialize(freezes, node_request_buffer);

  int ineq_ix = 0;
  for (std::shared_ptr<TriangleInequality> ineq : node->get_inequalities()) {
    ineq->serialize(node_request_buffer + 3*N + 6*ineq_ix);
    ineq_ix++;
  }
  for (int i = 3*N + 6*ineq_ix; i < 3*N + 6*M; i++) {
    node_request_buffer[i] = -1;
  }

  MPI_Send(node_request_buffer,
           3*N + 6*M + 1,
           MPI_INT,
           target_rank,
           0,
           MPI_COMM_WORLD);
}

void send_finish_request(int N,
                         int M,
                         int target_rank,
                         int* node_request_buffer) {
  node_request_buffer[3*N + 6*M] = MESSAGE_FINISH;
  MPI_Send(node_request_buffer,
           3*N + 6*M + 1,
           MPI_INT,
           target_rank,
           0,
           MPI_COMM_WORLD);
}

void receive_work_response(int N,
                           int M,
                           std::map<int, std::shared_ptr<Node>>& node_map,
                           double* node_response_buffer,
                           MPI_Status* status) {
  MPI_Recv(node_response_buffer,
           N + 4 + 6*M,
           MPI_DOUBLE,
           MPI_ANY_SOURCE,
           0,
           MPI_COMM_WORLD,
           status);

  std::shared_ptr<Node> node = node_map.find(status->MPI_SOURCE)->second;

  node->set_lower_bound(node_response_buffer[N]);
  node->set_upper_bound(node_response_buffer[N + 1]);
  node->set_branch_i(node_response_buffer[N + 2]);
  node->set_branch_j(node_response_buffer[N + 3]);

  Eigen::VectorXd lower_bound_witness(N);
  lower_bound_witness =
    Eigen::Map<Eigen::VectorXd>(node_response_buffer, N);
  node->set_lower_bound_witness(lower_bound_witness);

  std::list<std::shared_ptr<TriangleInequality>> post_inequalities{};
  for (int i = N + 4; i < N + 4 + 6*M; i += 6) {
    if (node_response_buffer[i] != -1) {
      post_inequalities.push_back(std::shared_ptr<TriangleInequality>(new TriangleInequality(node_response_buffer + i)));
    }
  }

  node->set_post_inequalities(post_inequalities);
}

void receive_work_response(int N,
			   int M,
                           int target_rank,
                           Node* node,
                           double* node_response_buffer,
                           MPI_Status* status) {
  MPI_Recv(node_response_buffer,
           N + 4 + 6*M,
           MPI_DOUBLE,
           target_rank,
           0,
           MPI_COMM_WORLD,
           status);

  node->set_lower_bound(node_response_buffer[N]);
  node->set_upper_bound(node_response_buffer[N + 1]);
  node->set_branch_i(node_response_buffer[N + 2]);
  node->set_branch_j(node_response_buffer[N + 3]);

  Eigen::VectorXd lower_bound_witness(N);
  lower_bound_witness =
    Eigen::Map<Eigen::VectorXd>(node_response_buffer, N);
  node->set_lower_bound_witness(lower_bound_witness);

  std::list<std::shared_ptr<TriangleInequality>> post_inequalities{};
  for (int i = N + 4; i < N + 4 + 6*M; i += 6) {
    if (node_response_buffer[i] != -1) {
      post_inequalities.push_back(std::shared_ptr<TriangleInequality>(new TriangleInequality(node_response_buffer + i)));
    }
  }

  node->set_post_inequalities(post_inequalities);
}

void send_work_response(int N,
                        int M,
                        const Node* node,
                        double* node_response_buffer) {
  Eigen::VectorXd lower_bound_witness =
    FreezeMap_expand_vector(node->get_lower_bound_witness(),
                            node->get_freeze_map());

  for (int i = 0; i < N; i++) {
    node_response_buffer[i] = lower_bound_witness(i);
  }

  node_response_buffer[N] = node->get_lower_bound();
  node_response_buffer[N + 1] = node->get_upper_bound();
  node_response_buffer[N + 2] = node->get_branch_i() + 0.5;
  node_response_buffer[N + 3] = node->get_branch_j() + 0.5;

  int ineq_ix = 0;
  for (std::shared_ptr<TriangleInequality> ineq : node->get_post_inequalities()) {
    ineq->serialize(node_response_buffer + N + 4 + 6*ineq_ix);
    ineq_ix++;
  }
  for (int i = N + 4 + 6*ineq_ix; i < N + 4 + 6*M; i++) {
    node_response_buffer[i] = -1;
  }

  MPI_Send(node_response_buffer,
           N + 4 + 6*M,
           MPI_DOUBLE,
           0,
           0,
           MPI_COMM_WORLD);
}

MessageType receive_work_request(int N,
                                 int M,
                                 Node* node,
                                 int* node_request_buffer,
                                 MPI_Status* status) {
  FreezeMap freeze_map;

  MPI_Recv(node_request_buffer,
           3*N + 6*M + 1,
           MPI_INT,
           0,
           0,
           MPI_COMM_WORLD,
           status);

  MessageType message_type = 
    static_cast<MessageType>(node_request_buffer[3*N + 6*M]);

  if (message_type == MESSAGE_WORK) {
    FreezeMap_deserialize(N, node_request_buffer, &freeze_map);
    std::list<std::shared_ptr<TriangleInequality>> inequalities{};
    for (int i = 3*N; i < 3*N + 6*M; i += 6) {
      if (node_request_buffer[i] != -1) {
        inequalities.push_back(std::shared_ptr<TriangleInequality>(new TriangleInequality(node_request_buffer + i)));
      }
    }

    *node = Node(node->get_initial_A(), freeze_map, inequalities);
  }

  return message_type;
}
