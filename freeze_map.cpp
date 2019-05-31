#include <map>
#include <string>
#include <utility>
#include <Eigen/Dense>
#include "freeze_map.h"


std::string FreezeMap_to_string(const FreezeMap* f) {
  std::string s = "(";

  bool first = true;
  for (FreezeMap::const_iterator it=f->begin(); it != f->end(); ++it) {
    int i = it->first;

    if (first) {
      first = false;
    } else {
      s.append(",");
    }
    s.append(std::to_string(i));
    s.append(",(");

    bool inner_first = true;
    for (std::map<int, int>::const_iterator jt=it->second.begin();
         jt != it->second.end();
         ++jt) {
      if (inner_first) {
        inner_first = false;
      } else {
        s.append(",");
      }
      int j = jt->first;
      int s_ij = jt->second;
      s.append(std::to_string(j));
      s.append(",");
      s.append(std::to_string(s_ij));
    }
    s.append(")");
  }

  s.append(")");

  return s;
}


void FreezeMap_serialize(const FreezeMap* f, int* s) {
  int s_ix = 0;
  for (FreezeMap::const_iterator it=f->begin(); it != f->end(); ++it) {
    int i = it->first;

    s[s_ix] = i;
    s_ix++;
    s[s_ix] = i;
    s_ix++;
    s[s_ix] = 0;
    s_ix++;

    for (std::map<int, int>::const_iterator jt=it->second.begin();
             jt != it->second.end();
             ++jt) {
      int j = jt->first;
      int s_ij = jt->second;

      s[s_ix] = j;
      s_ix++;
      s[s_ix] = i;
      s_ix++;
      s[s_ix] = s_ij;
      s_ix++;
    }
  }
}


void FreezeMap_deserialize(int N, const int* s, FreezeMap* f) {
  f->clear();

  for (int i = 0; i < N; i++) {
    int s1 = s[i * 3];
    int s2 = s[i * 3 + 1];
    int s3 = s[i * 3 + 2];

    if (s3 == 0) {
      f->insert(std::make_pair(s1, std::map<int, int>()));
    } else {
      (*f)[s2][s1] = s3;
    }
  }
}


int FreezeMap_num_frozen(const FreezeMap* f) {
  int ret = 0;
  for (FreezeMap::const_iterator it=f->begin(); it != f->end(); ++it) {
    ret += it->second.size();
  }
  return ret;
}


Eigen::MatrixXd FreezeMap_transform_matrix(const Eigen::MatrixXd& A,
                                           const FreezeMap* f) {
  int N = A.rows();
  int M = N - FreezeMap_num_frozen(f);

  Eigen::MatrixXd ret(M, M);

  int row_ix = 0;
  int col_ix = 0;
  for (FreezeMap::const_iterator it=f->begin(); it != f->end(); ++it) {
    int i = it->first;

    col_ix = 0;
    for (FreezeMap::const_iterator kt=f->begin(); kt != f->end(); ++kt) {
      int k = kt->first;

      ret(row_ix, col_ix) = A(i, k);

      for (std::map<int, int>::const_iterator jt=it->second.begin();
           jt != it->second.end();
           ++jt) {
        int j = jt->first;
        int s_ij = jt->second;

        for (std::map<int, int>::const_iterator jt2=kt->second.begin();
             jt2 != kt->second.end();
             ++jt2) {
          int j2 = jt2->first;
          int s_ij2 = jt2->second;

          ret(row_ix, col_ix) += s_ij * s_ij2 * A(j, j2);
        }
      }

      for (std::map<int, int>::const_iterator jt=it->second.begin();
           jt != it->second.end();
           ++jt) {
        int j = jt->first;
        int s_ij = jt->second;

        ret(row_ix, col_ix) += s_ij * A(k, j);
      }
      for (std::map<int, int>::const_iterator jt=kt->second.begin();
           jt != kt->second.end();
           ++jt) {
        int j = jt->first;
        int s_kj = jt->second;

        ret(row_ix, col_ix) += s_kj * A(i, j);
      }

      col_ix++;
    }
    row_ix++;
  }

  return ret;
}

Eigen::VectorXd FreezeMap_expand_vector(const Eigen::VectorXd& y,
                                        const FreezeMap* f) {
  int N = FreezeMap_num_frozen(f) + f->size();

  Eigen::VectorXd ret(N);

  int ix = 0;
  for (FreezeMap::const_iterator it=f->begin(); it != f->end(); ++it) {
    int i = it->first;

    ret(i) = y(ix);

    for (std::map<int, int>::const_iterator jt=it->second.begin();
         jt != it->second.end();
         ++jt) {
      int j = jt->first;
      int s_ij = jt->second;

      ret(j) = s_ij * y(ix);
    }

    ix++;
  }

  return ret;
}

void FreezeMap_print(const FreezeMap* f) {
  for (FreezeMap::const_iterator it=f->begin(); it != f->end(); ++it) {
    printf("%d : ", it->first);
    for (std::map<int, int>::const_iterator jt=it->second.begin();
	 jt != it->second.end();
	 ++jt) {
      printf("(%d, %d) ", jt->first, jt->second);
    }
    printf("\n");
  }
}

std::pair<FreezeMap, FreezeMap> FreezeMap_branch(const FreezeMap* f,
                                                 int i,
                                                 int j) {
  FreezeMap freezes_pos = *f;
  freezes_pos[j].insert(freezes_pos[i].begin(), freezes_pos[i].end());
  freezes_pos[j].insert(std::make_pair(i, +1));
  freezes_pos.erase(i);

  FreezeMap freezes_neg = *f;
  std::for_each(freezes_neg[i].begin(),
                freezes_neg[i].end(),
                [](std::pair<const int, int>& p) {
                  p.second = -p.second;
                });
  freezes_neg[j].insert(freezes_neg[i].begin(), freezes_neg[i].end());
  freezes_neg[j].insert(std::make_pair(i, -1));
  freezes_neg.erase(i);

  return std::make_pair(freezes_pos, freezes_neg);
}
