#include "node.h"
#include "node_queue.h"


bool NodeQueue::empty() {
  return (queue.empty() ||
          queue[0]->get_upper_bound() < lower_bound);
}

std::shared_ptr<Node> NodeQueue::pop() {
  std::pop_heap(queue.begin(), queue.end(), comparator);
  std::shared_ptr<Node> ret = queue[queue.size() - 1];
  queue.pop_back();
  return ret;
}

bool NodeQueue::push(std::shared_ptr<Node> node) {
  if (queue.empty() || node->get_upper_bound() > lower_bound) {
    queue.push_back(node);
    std::push_heap(queue.begin(), queue.end(), comparator);
    return true;
  }
  return false;
}

void NodeQueue::clean() {
  std::sort_heap(queue.begin(), queue.end(), comparator);
  std::reverse(queue.begin(), queue.end());
  int i;
  for (i = 0; i < queue.size(); i++) {
    if (queue[i]->get_upper_bound() <= lower_bound) {
      break;
    }
  }
  queue.resize(i);
  std::make_heap(queue.begin(), queue.end(), comparator);
}
