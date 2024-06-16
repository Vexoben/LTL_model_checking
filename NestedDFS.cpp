#include "NestedDFS.hpp"

// Find all nodes reachable from a set of nodes
std::vector<int> NestedDFSProcessor::reachable_from(std::vector<int> nodes) {
   std::vector<int> reachable;
   std::vector<bool> visited(node_count, false);
   for (std::vector<int>::size_type i = 0; i < nodes.size(); ++i) {
      if (!visited[nodes[i]]) {
         std::stack<int> stk;
         stk.push(nodes[i]);
         visited[nodes[i]] = true;
         while (!stk.empty()) {
            int node = stk.top();
            stk.pop();
            reachable.push_back(node);
            for (std::vector<int>::size_type j = 0; j < edges[node].size(); ++j) {
               int to = edges[node][j];
               if (!visited[to]) {
                  stk.push(to);
                  visited[to] = true;
               }
            }
         }
      }
   }
   return reachable;
}

bool NestedDFSProcessor::circle_check_helper(int dest, int current, std::vector<bool> &visited) {
   for (std::vector<int>::size_type i = 0; i < edges[current].size(); ++i) {
      int to = edges[current][i];
      if (to == dest) {
         return true;
      }
      if (!visited[to]) {
         visited[to] = true;
         if (circle_check_helper(dest, to, visited)) {
            return true;
         }
      }
   }
   return false;
}

bool NestedDFSProcessor::circle_check(int id) {
   std::vector<bool> visited(node_count, 0);
   visited[id] = true;
   return circle_check_helper(id, id, visited);
}