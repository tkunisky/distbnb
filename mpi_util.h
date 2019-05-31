#include <memory>
#include <mpi.h>
#include "node.h"
#include "message.h"


// Sends a request to process `target_rank` to handle `node`.
// Assumes node_request_buffer has length 3*N + 1.
void send_work_request(int N,
		       int M,
                       int target_rank,
                       const Node* node,
                       int* node_request_buffer);


// Sends a request to process `target_rank` to terminate.
// Assumes node_request_buffer has length 3*N + 1.
void send_finish_request(int N,
			 int M,
                         int target_rank,
                         int* node_request_buffer);

void receive_work_response(int N,
			   int M,
                           int target_rank,
                           Node* node,
                           double* node_response_buffer,
                           MPI_Status* status);

void receive_work_response(int N,
			   int M,
                           std::map<int, std::shared_ptr<Node>>& node_map,
                           double* node_response_buffer,
                           MPI_Status* status);

MessageType receive_work_request(int N,
				 int M,
                                 Node* node,
                                 int* node_request_buffer,
                                 MPI_Status* status);

void send_work_response(int N,
			int M,
                        const Node* node,
                        double* node_response_buffer);
