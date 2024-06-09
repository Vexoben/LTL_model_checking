#include "SCCProcessor.hpp"

// Tarjan's algorithm for finding strongly connected components
void SCCProcessor::tarjan(int node, int &time) {
   dfn[node] = low[node] = ++time;
   stk.push(node);
   in_stack[node] = true;
   for (std::vector<int>::size_type i = 0; i < edges[node].size(); ++i) {
      int to = edges[node][i];
      if (!dfn[to]) {
         tarjan(to, time);
         low[node] = std::min(low[node], low[to]);
      } else if (in_stack[to]) {
         low[node] = std::min(low[node], dfn[to]);
      }
   }
   if (dfn[node] == low[node]) {
      scc.push_back(std::set<int>());
      int top;
      do {
         top = stk.top();
         stk.pop();
         in_stack[top] = false;
         scc.back().insert(top);
         scc_belong[top] = ((int) scc.size()) - 1;
      } while (top != node);
   }
}

void SCCProcessor::calc_scc() {
   dfn = std::vector<int>(node_count, 0);
   low = std::vector<int>(node_count, 0);
   scc_belong = std::vector<int>(node_count, 0);
   in_stack = std::vector<bool>(node_count, false);
   int time = 0;
   for (int i = 0; i < node_count; ++i) {
      if (!dfn[i]) {
         tarjan(i, time);
      }
   }
}

// Check if a strongly connected component contains a circle
bool SCCProcessor::scc_contains_circle(int id) {
   std::set<int> scc_set = scc[id];
   if (scc_set.size() > 1) {
      return true;
   } else if (scc_set.size() == 1) {
      int node = *scc_set.begin();
      return has_self_loop[node];
   }
   return false;
}

// Find all nodes reachable from a set of nodes
std::vector<int> SCCProcessor::reachable_from(std::vector<int> nodes) {
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