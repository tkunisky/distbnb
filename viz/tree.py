# Script to visualize a tree logged by the main executable. It assumes the
# input is a file containing two kinds of lines:
#
#   (Type 1) LABEL_1 LABEL_2 LABEL_3
#   (Type 2) LABEL_1 TIME
#
# where LABEL_X are strings with no spaces and TIME is a floating point number.
# Type 1 lines should define a tree structure, where each line means that the
# node of LABEL_1 has the nodes of LABEL_2 and LABEL_3 as children.
#
# For each node label, there should be three Type 2 lines, and the resulting
# tree graphic will have the node of LABEL_X attached to two line segments
# with length corresponding to the two gaps between these timestamps.

import argparse
import collections

import ete2


parser = argparse.ArgumentParser()
parser.add_argument('--input_file', type=str, required=True)
parser.add_argument('--output_file', type=str, required=True)
args = parser.parse_args()


def main():
    non_root_nodes = set()
    tree_dict = dict()
    node_times_dict = collections.defaultdict(list)

    # Read data file
    with open(input_file, 'r') as f:
        for l in f.readlines():
            if l.startswith('('):
                parts = l.split(' ')
                if len(parts) == 2:
                    tuple_txt, time = parts
                    time = float(time)
                    t = eval(tuple_txt)
                    node_times_dict[t].append(time)
                elif len(parts) == 3:
                    parent, child1, child2 = parts
                    parent = eval(parent)
                    child1 = eval(child1)
                    child2 = eval(child2)

                    tree_dict[parent] = (child1, child2)

                    non_root_nodes.add(child1)
                    non_root_nodes.add(child2)
                else:
                    assert False
            else:
                print l

    # Post-process time info
    node_times_dict = {k: sorted(v) for k, v in node_times_dict.iteritems()}

    # Post-process tree info, building an ete2 tree with two nodes for each
    # actual node (to allow us to split the corresponding line segment into
    # two parts in the final drawing).
    root_node_label = [n for n in tree_dict if n not in non_root_nodes][0]
    root_node = ete2.TreeNode()
    root_node_2 = ete2.TreeNode()
    root_node.add_child(root_node_2);
    tree_nodes_dict = {root_node_label: root_node}
    tree_nodes_2_dict = {root_node_label: root_node_2}

    def build_tree(node_label):
        if node_label in tree_dict:
            child1_label, child2_label = tree_dict[node_label]

            tree_nodes_dict[child1_label] = ete2.TreeNode()
            tree_nodes_dict[child2_label] = ete2.TreeNode()
            tree_nodes_2_dict[child1_label] = ete2.TreeNode()
            tree_nodes_2_dict[child2_label] = ete2.TreeNode()

            tree_nodes_2_dict[node_label].add_child(
                tree_nodes_dict[child1_label])
            tree_nodes_2_dict[node_label].add_child(
                tree_nodes_dict[child2_label])

            tree_nodes_dict[child1_label].add_child(
                tree_nodes_2_dict[child1_label])
            tree_nodes_dict[child2_label].add_child(
                tree_nodes_2_dict[child2_label])

            build_tree(child1_label)
            build_tree(child2_label)
        else:
            return

    build_tree(root_node_label)

    waiting_style = ete2.NodeStyle()
    waiting_style['hz_line_color'] = '#D62728'
    waiting_style['hz_line_width'] = 1.5
    waiting_style['size'] = 0.0
    waiting_style['fgcolor'] = '#444444'

    active_style = ete2.NodeStyle()
    active_style['hz_line_color'] = '#2CA02C'
    active_style['hz_line_width'] = 1.5
    active_style['size'] = 0
    active_style['fgcolor'] = '#000000'

    for node_label, node in tree_nodes_dict.iteritems():
        t1, t2, t3 = node_times_dict[node_label]
        node.dist = t2 - t1
        node.set_style(waiting_style)

    for node_label, node in tree_nodes_2_dict.iteritems():
        t1, t2, t3 = node_times_dict[node_label]
        node.dist = t3 - t2
        node.set_style(active_style)

    root_node.render(args.output_file)


if __name__ == '__main__':
    main()
