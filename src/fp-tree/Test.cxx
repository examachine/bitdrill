//
//
// C++ Implementation for module: Test
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "FP_Tree.hxx"

using namespace Utility;

void print_node_list(FP_Tree& fp_tree, int i)
{
  cout << "node list of " << i << endl;
  FP_Tree::Node* node = fp_tree.node_list(i);
  while (node) {
    cout << "node " << node->symbol << " : ";
    node = node->next_node;
  }
  cout << endl;
}

int main()
{
  int items[] = {'g','e','b','a','c','d','f'};
  set<int> s_items(items, items+7);

  vector<int> count;
  count.resize('z');
  count['g']=11;
  count['e']=10;
  count['d']=9;
  count['a']=8;
  count['b']=7;
  count['f']=6;
  count['c']=5;

  map<int,int> counts;
  FP_Tree fp_tree(s_items, count, counts, true);

  int t1[] = {'c','e','d'};
  fp_tree.insert(vector<int>(t1,t1+3));

  int t2[] = {'e','d','c'};
  fp_tree.insert(vector<int>(t2,t2+3));

  int t3[] = {'a','g','d','c'};
  fp_tree.insert(vector<int>(t3,t3+4));

  int t4[] = {'e','a','g'};
  fp_tree.insert(vector<int>(t4,t4+3));

  int t5[] = {'g','b','f','d'};
  fp_tree.insert(vector<int>(t5,t5+4));

  int t6[] = {'d','g','e'};
  fp_tree.insert(vector<int>(t6,t6+3));

  int t7[] = {'a','g','f'};
  fp_tree.insert(vector<int>(t7,t7+3));

  int t8[] = {'g','b','e'};
  fp_tree.insert(vector<int>(t8,t8+3));

  int t9[] = {'b','a','d'};
  fp_tree.insert(vector<int>(t9,t9+3));

  int t10[] = {'g','e','a'};
  fp_tree.insert(vector<int>(t10,t10+3));

  int t11[] = {'b','g','a'};
  fp_tree.insert(vector<int>(t11,t11+3));

  int t12[] = {'e','c','d'};
  fp_tree.insert(vector<int>(t12,t12+3));

  int t13[] = {'d','b','g'};
  fp_tree.insert(vector<int>(t13,t13+3));

  int t14[] = {'f','b','a'};
  fp_tree.insert(vector<int>(t14,t14+3));

  int t15[] = {'c','e','f','b'};
  fp_tree.insert(vector<int>(t15,t15+4));

  int t16[] = {'e','d','g','f'};
  fp_tree.insert(vector<int>(t16,t16+4));

  int t17[] = {'a','e','g','f'};
  fp_tree.insert(vector<int>(t17,t17+4));

  cout << "query t1 ";
  FP_Tree::Node* node = fp_tree.query(vector<int>(t1,t1+2));
  if (node) {
    cout << "freq " << node->count << endl;
    fp_tree.test_path(vector<int>(t1,t1+2));
  }
  else cout << "not found" << endl;

  cout << "query <g,e,a> ";
  int b1[] = {'g','e','a'};
  vector<int> q1(b1, b1+3);
  node = fp_tree.query(q1);
  if (node) {
    cout << "freq " << node->count << endl;
    fp_tree.test_path(q1);
  }
  else
    cout << "not found" << endl;

  print_node_list(fp_tree, 'g');
  print_node_list(fp_tree, 'e');
  print_node_list(fp_tree, 'b');
  print_node_list(fp_tree, 'a');

  cout << "fp_tree = " << fp_tree << endl;
  ofstream fp_tree_dot("fp-tree.dot");
  dot_graph(fp_tree_dot, fp_tree);
  cout << "fp_tree dot written to fp_tree.dot " << endl;

  cout << "item table" << fp_tree.get_item_table() << endl;

  cout << "discover patterns, epsilon=2" << endl;
  fp_tree.discover_patterns(2, &cout);

  return 0;
}
