#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <mpi.h>
#include <string>
#include <unistd.h>
#include <Eigen/Dense>
#include "eigen_util.h"
#include "node.h"
#include "mcbb_impl.h"


int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int rank, p;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);

  std::string filename;
  Eigen::MatrixXd A;
  int getopt_ret;
  int N, M;
  int verbosity = 0;
  bool is_sync = false;
  bool readable_output = false;
  while ((getopt_ret = getopt(argc, argv, "f:m:sv")) != -1) {
    switch (getopt_ret) {
    case 'f':
      filename = std::string(optarg);
      A = read_csv(optarg);
      N = A.rows();
    case 'm':
      M = std::atoi(optarg);
      break;
    case 'v':
      verbosity = std::atoi(optarg);
      break;
    case 's':
      is_sync = true;
      break;
    case 'r':
      readable_output = true;
      break;
    }
  }

  if (rank == 0) {
    if (readable_output) {
      printf("Solving %s\n", filename.c_str());
      printf("Running with %d workers\n", p - 1);
      printf("Using %d triangular inequalities\n", M);
    } else {
      printf("FILENAME=%s\n", filename.c_str());
      printf("WORKERS=%d\n", p - 1);
      printf("INEQUALITIES=%d\n", M);
    }
  }

  // --- Timed section ---
  MPI_Barrier(MPI_COMM_WORLD);
  double start_time = MPI_Wtime();

  if (is_sync) {
    mcbb_sync(&A, M, verbosity);
  } else {
    mcbb_async(&A, M, verbosity);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  double duration = MPI_Wtime() - start_time;
  // --- End timed section ---

  if (rank == 0) {
    if (readable_output) {
      printf("Time elapsed: %f seconds\n\n", duration);
    } else {
      printf("TIME=%f\n", duration);
    }
  }

  MPI_Finalize();
}
