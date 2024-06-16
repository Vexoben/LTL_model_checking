#ifndef NESTED_DFS_HPP
#define NESTED_DFS_HPP

#include <set>
#include <stack>
#include <vector>

class NestedDFSProcessor {
 private:
   int node_count;
   std::vector<std::vector<int>> edges;
   std::vector<bool> in_stack;
   std::stack<int> stk;
   bool circle_check_helper(int dest, int current, std::vector<bool> &visited);
 public:
   NestedDFSProcessor(int node_count) : node_count(node_count), edges(node_count) {}
   void add_edge(int from, int to) {
      edges[from].push_back(to);
   }
   bool circle_check(int id);
   std::vector<int> reachable_from(std::vector<int>);
};


#endif // NESTED_DFS_HPP