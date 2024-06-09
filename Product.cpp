#include "Product.hpp"

std::set<std::string> APIntersection(const std::set<std::string> &ap1, const std::set<std::string> &ap2) {
   std::set<std::string> ap;
   for (const std::string &a : ap1) {
      if (ap2.find(a) != ap2.end()) {
         ap.insert(a);
      }
   }
   return ap;
}

bool APEqual(const std::set<std::string> &ap1, const std::set<std::string> &ap2, const std::set<std::string> &scope) {
   for (auto &a : scope) {
      if (ap1.find(a) != ap1.end() && ap2.find(a) == ap2.end()) {
         return false;
      }
      if (ap1.find(a) == ap1.end() && ap2.find(a) != ap2.end()) {
         return false;
      }
   }
   return true;
}

// Product of a TS and an NBA
std::shared_ptr<TS> ProductTSWithNBA(std::shared_ptr<TS> ts, std::shared_ptr<NBA> nba) {
   std::set<std::string> aps = APIntersection(ts->get_ap(), nba->get_ap());
   std::shared_ptr<TS> prod = std::make_shared<TS>();
   prod->set_ap(std::set<std::string>{"accepting"});
   std::vector<std::vector<TSNodePtr>> nodes(ts->get_node_count());
   int id = 0;
   for (int i = 0; i < ts->get_node_count(); ++i) {
      for (int j = 0; j < nba->get_node_count(); ++j) {
         int is_initial = 0;
         for (int k = 0; k < nba->get_node_count(); ++k) {
            if (nba->get_node(k)->get_is_initial() && 
                nba->get_node(k)->get_transition().find(j) != nba->get_node(k)->get_transition().end() &&
                APEqual(nba->get_node(k)->get_ap(), ts->get_node(i)->get_ap(), aps)) {
               is_initial = 1;
            }
         }
         TSNodePtr node = std::make_shared<TSNode>(id++, 
                      is_initial && ts->get_node(i)->get_is_initial(), 
                      nba->get_node(j)->get_is_accepting() ? std::set<std::string>{"accepting"} : std::set<std::string>{});
         nodes[i].push_back(node);
         prod->add_node(node);
      }
   }
   for (int i1 = 0; i1 < ts->get_node_count(); ++i1) {
      for (int j1 = 0; j1 < nba->get_node_count(); ++j1) {
         for (int i2 = 0; i2 < ts->get_node_count(); ++i2) {
            for (int j2 = 0; j2 < nba->get_node_count(); ++j2) {
               TSNodePtr node1 = nodes[i1][j1];
               TSNodePtr node2 = nodes[i2][j2];
               if (APEqual(nba->get_node(j1)->get_ap(), ts->get_node(i2)->get_ap(), aps) &&
                   ts->get_node(i1)->get_transition().find(i2) != ts->get_node(i1)->get_transition().end() &&
                   nba->get_node(j1)->get_transition().find(j2) != nba->get_node(j1)->get_transition().end()) {
                  prod->add_transition(node1->get_id(), node2->get_id());
               }
            }
         }
      }
   }
   return prod;
}