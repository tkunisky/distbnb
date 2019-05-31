#ifndef __FREEZE_MAP_H__
#define __FREEZE_MAP_H__

#include <map>
#include <utility>

// A FreezeMap tracks which indices have been identified together and with
// which sign flips. It consists of a map from indices ("representatives") to
// a set of pairs of indices and signs. All indices appearing are distinct.
// For example, the FreezeMap
//
//   {1: [(2, +), (4, -)],
//    5: [(3, -), (8, -)],
//    6: [],
//    7: [(9, +)]}
//
// represents the variable identifications
//
//   x1 = x2 = -x4
//   x5 = -x3 = -x8
//   x7 = x9
//
// Note that we adopt the convention of including an empty list of "partners"
// for indices that have not been identified with any others. This helps with
// serialization/deserialization in a fixed-width format. Also, it ensure that
// the "active" indices are exactly the keys of the FreezeMap.

typedef std::map<int, std::map<int, int>> FreezeMap;

// Returns a string serialization of a FreezeMap. The serialization is
// guaranteed to not contain spaces and to parse to a valid Python tuple-of-
// tuples which is unique per semantically unique FreezeMap.
std::string FreezeMap_to_string(const FreezeMap* f);

// Prints a human-readable string version of a FreezeMap (for debugging).
void FreezeMap_print(const FreezeMap* f);

// Serializes f to an array of 3N integers.
void FreezeMap_serialize(const FreezeMap* f, int* s);

// Deserializes f from an array of 3N integers.
void FreezeMap_deserialize(int N, const int* s, FreezeMap* f);

// Counts the number of frozen indices in f.
int FreezeMap_num_frozen(const FreezeMap* f);

// Returns B such that x'Ax = z'Bz whenever x is a +/- 1 vector and z is its
// restriction to the non-frozen indices of f (those appearing as keys).
Eigen::MatrixXd FreezeMap_transform_matrix(const Eigen::MatrixXd& A,
                                           const FreezeMap* f);

// Returns z such that, if [i_1, ..., i_m] are the (sorted) keys of f, then
// z[i_k] = y[k] and z_[j] = s * y[k] whenever (j, s) is in f[i_k].
Eigen::VectorXd FreezeMap_expand_vector(const Eigen::VectorXd& y,
                                        const FreezeMap* f);

// Returns two FreezeMaps corresponding to branching on x_i = x_j and
// x_i = -x_j.
std::pair<FreezeMap, FreezeMap> FreezeMap_branch(const FreezeMap* f,
                                                 int i,
                                                 int j);

#endif  // __FREEZE_MAP_H__
