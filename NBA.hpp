#ifndef NBA_HPP
#define NBA_HPP

#include "Expr.hpp"

class NBANode {
 private:
   int id;
   int is_initial;
   int is_accepting;
   std::set<std::string> ap;
   std::set<int> transition;
 public:
   NBANode() : id(0), is_initial(0), is_accepting(0) {}
   NBANode(int id, int is_initial, const std::set<std::string> &ap) : id(id), is_initial(is_initial), is_accepting(0), ap(ap) {}
   int get_id() const {
      return id;
   }
   int get_is_initial() const {
      return is_initial;
   }
   int get_is_accepting() const {
      return is_accepting;
   }
   void set_is_accepting(int is_accepting) {
      this->is_accepting = is_accepting;
   }
   std::set<std::string>& get_ap() {
      return ap;
   }
   std::set<int>& get_transition() {
      return transition;
   }
};

typedef std::shared_ptr<NBANode> NBANodePtr;

class NBA_base {
 protected:
   int node_count;
   std::vector<int> initial;
   std::set<std::string> aps;
   std::vector<NBANodePtr> nodes;
   std::map<int, NBANodePtr> node_map;
 public:
   NBA_base() : node_count(0) {}
   NBA_base(int node_count) : node_count(node_count) {}
   void add_node(NBANodePtr node) {
      nodes.push_back(node);
      node_map[node->get_id()] = node;
      if (node->get_is_initial()) {
         initial.push_back(node->get_id());
      }
      ++node_count;
      for (auto & ap : node->get_ap()) {
         aps.insert(ap);
      }
   }
   void add_transition(int from, int to) {
      node_map[from]->get_transition().insert(to);
   }
   int get_node_count() const {
      return node_count;
   }
   std::vector<int>& get_initial() {
      return initial;
   }
   std::set<std::string>& get_ap() {
      return aps;
   }
   NBANodePtr get_node(int id) const {
      return node_map.at(id);
   }
   virtual void print() {
      std::cout << "AP: {";
      for (auto &ap : aps) {
         std::cout << ap;
         if (ap != *aps.rbegin()) std::cout << ", ";
      }
      std::cout << "}\n";
      std::cout << "initial: ";
      for (auto &i : initial) {
         std::cout << i << " ";
      }
      std::cout << "\n";
      for (auto &node : nodes) {
         std::cout << "Node " << node->get_id() << "  ";
         std::cout << "AP: {";
         for (auto &ap : node->get_ap()) {
            std::cout << ap;
            if (ap != *node->get_ap().rbegin()) {
               std::cout << ", ";
            }
         }
         std::cout << "} ";
         std::cout << "is accepting: " << node->get_is_accepting() << "\n";
         std::cout << "Transition: ";
         for (auto &to : node->get_transition()) {
            std::cout << to << " ";
         }
         std::cout << "\n";
      }
   }
};

class NBA : public NBA_base {
 private:
   std::set<int> accepting;
 public:
   NBA() : NBA_base() {}
   NBA(int node_count) : NBA_base(node_count) {}
   void add_accepting(int node) {
      accepting.insert(node);
      get_node(node)->set_is_accepting(1);
   }
   std::set<int>& get_accepting() {
      return accepting;
   }
   void print() {
      std::cout << "-----------------NBA output begin--------------------\n";
      NBA_base::print();
      std::cout << "Accepting: ";
      for (auto &node : accepting) {
         std::cout << node << " ";
      }
      std::cout << "\n";
      std::cout << "-----------------NBA output end----------------------\n";
   }
};

class GNBA : public NBA_base {
 private:
   std::vector<std::set<int>> accepting;
 public:
   GNBA() : NBA_base() {}
   GNBA(int node_count) : NBA_base(node_count) {}
   void add_accepting(const std::set<int> &nodes) {
      accepting.push_back(nodes);
   }
   std::vector<std::set<int>>& get_accepting() {
      return accepting;
   }
   void print() {
      std::cout << "----------------GNBA output begin--------------------\n";
      NBA_base::print();
      std::cout << "Accepting:\n";
      for (std::vector<std::set<int>>::size_type i = 0; i < accepting.size(); ++i) {
         std::cout << "{" << i << ": ";
         for (auto &node : accepting[i]) {
            std::cout << node << " ";
         }
         std::cout <<"}\n";
      }
      std::cout << "----------------GNBA output end----------------------\n";
   }
};

std::shared_ptr<GNBA> LTL_to_GNBA(std::shared_ptr<ElementarySet> elementaries);
std::shared_ptr<NBA> GNBA_to_NBA(std::shared_ptr<GNBA> gnba);

#endif 