#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include <string>

// Implementation based on:
// https://gist.github.com/infusion/43bd2aa421790d5b4582
Eigen::MatrixXd read_csv(std::string filename) {
  std::ifstream in_file(filename);
  std::string line;

  int N = 0;
  while (std::getline(in_file, line)) {
    N++;
  }

  in_file.clear();
  in_file.seekg(0, std::ios::beg);

  Eigen::MatrixXd A(N, N);

  int i = 0;
  while (std::getline(in_file, line)) {
    // Skip comment lines
    if (line.rfind("#", 0) == 0) {
      continue;
    }

    char* line_str = (char*) line.c_str();

    int j = 0;
    char* value_ptr = line_str;
    for (int str_ix = 0; str_ix < line.length(); str_ix++) {
      if (line_str[str_ix] == ',') {
        // Uses that atof parsing will automatically stop at a comma
        A(i, j) = std::atof(value_ptr);

        value_ptr = line_str + str_ix + 1;
        j++;
      }
    }
    A(i, j) = std::atof(value_ptr);

    i++;
  }

  in_file.close();

  return A;
}
