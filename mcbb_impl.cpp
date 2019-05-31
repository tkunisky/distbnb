#include <iostream>
#include <list>
#include <cstdio>
#include <cstdlib>
#include <Eigen/Dense>
#include <mpi.h>
#include "node.h"
#include "node_queue.h"
#include "message.h"
#include "mcbb_impl.h"
#include "mpi_util.h"

// Assumes:
// - MPI_Init has been called and MPI_Finalize will be called later.
// - Every process has the same `A` and is calling this function.


void mcbb_sync(const Eigen::MatrixXd* A, int M, int verbosity) {
  // --- Setup ---

  int N = A->rows();

  // Buffer for sending work/termination requests to worker nodes
  int* node_request_buffer = new int[3*N + 6*M + 1];
  // Buffer for returning results to parent node
  double* node_response_buffer = new double[N + 4 + 6*M];

  int rank, p, num_workers;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  // Reserve process 0 for coordination
  num_workers = p - 1;

  NodeQueue node_queue;
  if (rank == 0) {  // --- Root coordinating process ---
    Eigen::VectorXd best_lower_bound_witness(N);
    MPI_Status root_status;

    std::shared_ptr<Node> root_node(new Node(A));
    node_queue.push(root_node);

    std::list<std::shared_ptr<Node>> node_batch{};

    bool saturation_achieved = false;
    int round_count = 0;

    while (!node_queue.empty()) {
      node_batch.clear();
      // Send out as many nodes as possible
      for (int i = 1; i <= p - 1; i++) {
        if (!node_queue.empty()) {
          std::shared_ptr<Node> this_node = node_queue.pop();
          node_batch.push_back(this_node);

          send_work_request(N, M, i, this_node.get(), node_request_buffer);
        } else {
          break;
        }
      }

      if (node_batch.size() == p - 1 && !saturation_achieved) {
        printf("Round %d : saturation achieved\n", round_count);
        saturation_achieved = true;
      }

      if (node_batch.size() < p - 1 && saturation_achieved) {
        printf("Round %d : saturation lost\n", round_count);
        saturation_achieved = false;
      }

      if (round_count % 10 == 0) {
        node_queue.clean();
        printf("Round %d : surplus queue size = %d\n",
               round_count,
               node_queue.size());
      }

      // Wait to hear back from all active workers and process each result.
      // Aggregate the new best lower bound and corresponding vector, and set
      // the upper bounds of each node.
      int worker_ix = 1;
      for (std::list<std::shared_ptr<Node>>::const_iterator it =
             node_batch.begin();
           it != node_batch.end();
           ++it) {
        receive_work_response(N,
                              M,
                              worker_ix,
                              (*it).get(),
                              node_response_buffer,
                              &root_status);

        if (node_queue.aggregate_lower_bound((*it)->get_lower_bound())) {
          best_lower_bound_witness = (*it)->get_lower_bound_witness();
        }

        worker_ix++;
      }

      // Update node queue with branches as needed
      for (std::list<std::shared_ptr<Node>>::const_iterator it =
             node_batch.begin();
           it != node_batch.end();
           ++it) {
        if ((*it)->get_upper_bound() >= node_queue.get_lower_bound()) {
          std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> children =
            (*it)->branch_on_suggested();
          node_queue.push(children.first);
          node_queue.push(children.second);
        }
      }
      round_count++;
    }
    round_count--;

    for (int i = 1; i <= p - 1; i++) {
      send_finish_request(N, M, i, node_request_buffer);
    }

    printf("Final value: %.4f\n", node_queue.get_lower_bound());
    printf("Finished in %d rounds\n", round_count);
  } else {         // --- Worker process ---
    MPI_Status worker_status;
    Node worker_node(A);

    while (receive_work_request(N,
                                M,
                                &worker_node,
                                node_request_buffer,
                                &worker_status) != MESSAGE_FINISH) {
      worker_node.execute(M);
      send_work_response(N, M, &worker_node, node_response_buffer);
    }
  }

  delete[] node_request_buffer;
  delete[] node_response_buffer;
}

void mcbb_async(const Eigen::MatrixXd* A, int M, int verbosity) {
  // --- Setup ---

  int N = A->rows();

  // Buffer for sending work/termination requests to worker nodes
  int* node_request_buffer = new int[3*N + 6*M + 1];
  // Buffer for returning results to parent node
  double* node_response_buffer = new double[N + 4 + 6*M];

  int rank, p, num_workers;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  // Reserve process 0 for coordination
  num_workers = p - 1;

  int total_nodes = 0;

  NodeQueue node_queue{};
  if (rank == 0) {  // --- Root coordinating process ---
    Eigen::VectorXd best_lower_bound_witness(N);
    MPI_Status root_status;

    std::shared_ptr<Node> root_node(new Node(A));
    node_queue.push(root_node);

    if (verbosity) {
      std::cout << FreezeMap_to_string(root_node->get_freeze_map());
      printf(" %f\n", MPI_Wtime());
    }

    total_nodes++;

    // Tracks which nodes are currently pending on which processes
    std::map<int, std::shared_ptr<Node>> node_assignments{};

    while (!node_queue.empty() || !node_assignments.empty()) {
      for (int i = 1; i <= p - 1; i++) {
        // If process is idle and there is work to do, send it out
        if ((node_assignments.find(i) == node_assignments.end()) &&
            !node_queue.empty()) {
          std::shared_ptr<Node> this_node = node_queue.pop();

          send_work_request(N, M, i, this_node.get(), node_request_buffer);

          if (verbosity) {
            std::cout << FreezeMap_to_string(this_node->get_freeze_map());
            printf(" %f\n", MPI_Wtime());
          }

          node_assignments[i] = this_node;
        }
      }

      receive_work_response(N,
                            M,
                            node_assignments,
                            node_response_buffer,
                            &root_status);

      int response_source = root_status.MPI_SOURCE;
      std::shared_ptr<Node> response_node = node_assignments[response_source];

      if (verbosity) {
        std::cout << FreezeMap_to_string(response_node->get_freeze_map());
        printf(" %f\n", MPI_Wtime());
      }

      if (node_queue.aggregate_lower_bound(response_node->get_lower_bound())) {
        best_lower_bound_witness = response_node->get_lower_bound_witness();
      }

      if (response_node->get_upper_bound() > node_queue.get_lower_bound()) {
        std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> children =
          response_node->branch_on_suggested();
        node_queue.push(children.first);
        node_queue.push(children.second);

        if (verbosity) {
          std::cout << FreezeMap_to_string(response_node->get_freeze_map())
                    << " "
                    << FreezeMap_to_string(children.first->get_freeze_map())
                    << " "
                    << FreezeMap_to_string(children.second->get_freeze_map())
                    << std::endl;
        }

        if (verbosity) {
          std::cout << FreezeMap_to_string(children.first->get_freeze_map());
          printf(" %f\n", MPI_Wtime());

          std::cout << FreezeMap_to_string(children.second->get_freeze_map());
          printf(" %f\n", MPI_Wtime());
        }

        total_nodes += 2;
      }

      node_assignments.erase(response_source);
    }

    for (int i = 1; i <= p - 1; i++) {
      send_finish_request(N, M, i, node_request_buffer);
    }

    printf("Nodes processed: %d\n", total_nodes);
    printf("Final value: %.4f\n", node_queue.get_lower_bound());
  } else {         // --- Worker process ---
    MPI_Status worker_status;
    Node worker_node(A);

    while (receive_work_request(N,
                                M,
                                &worker_node,
                                node_request_buffer,
                                &worker_status) != MESSAGE_FINISH) {
      worker_node.execute(M);
      send_work_response(N, M, &worker_node, node_response_buffer);
    }
  }

  delete[] node_request_buffer;
  delete[] node_response_buffer;
}
