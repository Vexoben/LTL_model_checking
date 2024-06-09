#ifndef TS_HPP
#define TS_HPP

#include <set>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

class TSNode {
 private:
   int id;
   int is_initial;
   std::set<std::string> ap;
   std::set<int> transition;
 public:
   TSNode() {}
   TSNode(int id, int is_initial, const std::set<std::string> &ap) : id(id), is_initial(is_initial), ap(ap) {}
   TSNode(const TSNode &node) : id(node.id), is_initial(node.is_initial), ap(node.ap), transition(node.transition) {}
   int get_id() const {
      return id;
   }
   int get_is_initial() const {
      return is_initial;
   }
   std::set<std::string>& get_ap() {
      return ap;
   }
   std::set<int>& get_transition() {
      return transition;
   }
};

typedef std::shared_ptr<TSNode> TSNodePtr;

class TS {
 private:
   int node_count;
   std::vector<int> initial;
   std::vector<TSNodePtr> nodes;
   std::set<std::string> ap;
 public:
   TS() : node_count(0) {}
   TS(int node_count) : node_count(node_count) {}
   TS(const TS &ts) : node_count(ts.node_count), initial(ts.initial), ap(ts.ap) {
      for (auto &node : ts.nodes) {
         nodes.push_back(std::make_shared<TSNode>(*node));
      }
   }
   void add_node(TSNodePtr node) {
      ++node_count;
      nodes.push_back(node);
      if (node->get_is_initial()) {
         initial.push_back(node->get_id());
      }
   }
   void add_transition(int from, int to) {
      nodes[from]->get_transition().insert(to);
   }
   int get_node_count() const {
      return node_count;
   }
   std::vector<int>& get_initial() {
      return initial;
   }
   TSNodePtr get_node(int id) const {
      return nodes[id];
   }
   std::set<std::string>& get_ap() {
      return ap;
   }
   void set_ap(const std::set<std::string> &ap) {
      this->ap = ap;
   } 
   std::shared_ptr<TS> adjust_initial(int id) {
      std::shared_ptr<TS> ts = std::make_shared<TS>();
      ts->set_ap(ap);
      for (int i = 0; i < node_count; ++i) {
         TSNodePtr node = std::make_shared<TSNode>(nodes[i]->get_id(), nodes[i]->get_id() == id, std::set<std::string>(nodes[i]->get_ap()));
         ts->add_node(node);
      }
      for (int i = 0; i < node_count; ++i) {
         for (auto &to : nodes[i]->get_transition()) {
            ts->add_transition(nodes[i]->get_id(), to);
         }
      }
      return ts;
   }
   void print() {
      std::cout << "-----------------TS print begin-----------------" << std::endl;
      std::cout << "initial: ";
      for (auto &i : initial) {
         std::cout << i << ' ';
      }
      std::cout << std::endl;
      for (int i = 0; i < node_count; ++i) {
         std::cout << "node " << i << ": ";
         std::cout << "is_initial: " << nodes[i]->get_is_initial() << ' ';
         std::cout << "ap: ";
         for (auto &ap : nodes[i]->get_ap()) {
            std::cout << ap << ' ';
         }
         std::cout << std::endl;
         std::cout << "transition: ";
         for (auto &to : nodes[i]->get_transition()) {
            std::cout << to << ' ';
         }
         std::cout << std::endl;
      }
      std::cout << "-----------------TS print end-------------------" << std::endl;
   }
};

#endif