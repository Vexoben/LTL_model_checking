#ifndef SCCPROCESSOR_HPP
#define SCCPROCESSOR_HPP

#include <set>
#include <stack>
#include <vector>

// Strongly Connected Component Processor
class SCCProcessor {
 private:
   int node_count;
   std::vector<bool> has_self_loop;
   std::vector<int> dfn, low, scc_belong;
   std::vector<std::vector<int>> edges;
   std::vector<std::set<int>> scc;
   std::vector<bool> in_stack;
   std::stack<int> stk;
   void tarjan(int node, int &time);
 public:
   SCCProcessor(int node_count) : node_count(node_count), has_self_loop(node_count), edges(node_count) {}
   void add_edge(int from, int to) {
      edges[from].push_back(to);
      if (from == to) {
         has_self_loop[from] = true;
      }
   }
   int get_scc_belong(int node) {
      return scc_belong[node];
   }
   void calc_scc();
   bool scc_contains_circle(int id);
   std::vector<int> reachable_from(std::vector<int>);
};

#endif