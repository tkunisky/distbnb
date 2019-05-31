#ifndef __EIGEN_UTIL_H__
#define __EIGEN_UTIL_H__

#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include <string>

// Reads a CSV file into an Eigen matrix. Supports CSV comments starting with
// the '#' character.
Eigen::MatrixXd read_csv(std::string filename);

#endif  // __EIGEN_UTIL_H__
