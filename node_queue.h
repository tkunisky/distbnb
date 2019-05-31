// This defines a class that reimplements part of the interface of a priority
// queue on std::shared_ptr<Node>, but that tracks the best lower bound seen
// so far and ignores nodes with upper bound smaller than that.

#ifndef __NODE_QUEUE_H__
#define __NODE_QUEUE_H__

#include <memory>
#include "node.h"


struct LessThanByUpperBound {
  bool operator()(const std::shared_ptr<Node>& lhs,
                  const std::shared_ptr<Node>& rhs) const {
    return lhs->get_upper_bound() < rhs->get_upper_bound();
  }
};

class NodeQueue
{
 private:
  std::vector<std::shared_ptr<Node>> queue;
  double lower_bound;

 public:
  static LessThanByUpperBound comparator;

  bool empty();
  std::shared_ptr<Node> pop();
  bool push(std::shared_ptr<Node>);
  void clean();
  int size() { return queue.size(); }

  double get_lower_bound() { return lower_bound; }
  bool aggregate_lower_bound(double b) {
    if (lower_bound < b) {
      lower_bound = b;
      return true;
    }
    return false;
  }
};

#endif  // __NODE_QUEUE_H__
