#include "NBA.hpp"

// Convert an LTL formula to a GNBA
std::shared_ptr<GNBA> LTL_to_GNBA(std::shared_ptr<ElementarySet> elementaries) {
   std::shared_ptr<GNBA> gnba = std::make_shared<GNBA>();
   ExprPtr phi = elementaries->get_closure()->get_primary();
   int id = 0;
   for (auto &e : elementaries->get_elementaries()) {
      int initial = e.contains(phi) ? 1 : 0;
      gnba->add_node(std::make_shared<NBANode>(id++, initial, e.get_ap()));
   }
   for (std::vector<Elementary>::size_type i = 0; i < elementaries->get_elementaries().size(); ++i) {
      for (std::vector<Elementary>::size_type j = 0; j < elementaries->get_elementaries().size(); ++j) {
         bool flag = true;
         NBANodePtr from = gnba->get_node(i);
         NBANodePtr to = gnba->get_node(j);
         for (auto &expr : elementaries->get_closure()->get_exprs()) {
            if (expr->get_type() == ExprType::NEXT) {
               ExprPtr next = std::dynamic_pointer_cast<UnaryExpr>(expr)->get_expr();
               bool flag1 = elementaries->get_elementaries()[i].contains(expr);
               bool flag2 = elementaries->get_elementaries()[j].contains(next);
               if (flag1 != flag2) {
                  flag = false;
                  break;
               }
            }
         }
         if (!flag) continue;
         for (auto &expr : elementaries->get_closure()->get_exprs()) {
            if (expr->get_type() == ExprType::UNTIL) {
               ExprPtr left = std::dynamic_pointer_cast<BinaryExpr>(expr)->get_left();
               ExprPtr right = std::dynamic_pointer_cast<BinaryExpr>(expr)->get_right();
               bool flag1 = elementaries->get_elementaries()[i].contains(expr);
               bool flag2 = elementaries->get_elementaries()[i].contains(right) || 
                            (elementaries->get_elementaries()[i].contains(left) && elementaries->get_elementaries()[j].contains(expr));
               if (flag1 != flag2) {
                  flag = false;
                  break;
               }
            }
         }
         if (flag) {
            gnba->add_transition(i, j);
         }
      }
   }
   for (auto &expr : elementaries->get_closure()->get_exprs()) {
      if (expr->get_type() == ExprType::UNTIL) {
         ExprPtr left = std::dynamic_pointer_cast<BinaryExpr>(expr)->get_left();
         ExprPtr right = std::dynamic_pointer_cast<BinaryExpr>(expr)->get_right();
         std::set<int> accepting;
         for (std::vector<Elementary>::size_type i = 0; i < elementaries->get_elementaries().size(); ++i) {
            if (elementaries->get_elementaries()[i].contains(right) || !elementaries->get_elementaries()[i].contains(expr)) {
               accepting.insert(i);
            }
         }
         gnba->add_accepting(accepting);
      }
   }
   if (gnba->get_accepting().empty()) {
      std::set<int> accepting;
      for (int i = 0; i < id; ++i) {
         accepting.insert(i);
      }
      gnba->add_accepting(accepting);
   }
   return gnba;
}

// Convert a GNBA to an NBA
std::shared_ptr<NBA> GNBA_to_NBA(std::shared_ptr<GNBA> gnba) {
   std::shared_ptr<NBA> nba = std::make_shared<NBA>();
   std::vector<std::vector<NBANodePtr>> nodes(gnba->get_node_count());
   int id = 0;
   for (int i = 0; i < gnba->get_node_count(); ++i) {
      for (std::vector<std::set<int>>::size_type j = 0; j < gnba->get_accepting().size(); ++j) {
         NBANodePtr node = std::make_shared<NBANode>(id++, 
                                                     (j == 0) && gnba->get_node(i)->get_is_initial(), 
                                                     gnba->get_node(i)->get_ap());
         nodes[i].push_back(node);
         nba->add_node(node);
      }
   }
   for (auto &node : gnba->get_accepting()[0]) {
      nba->add_accepting(nodes[node][0]->get_id());
   }
   for (int i = 0; i < gnba->get_node_count(); ++i) {
      for (std::vector<std::set<int>>::size_type j = 0; j < gnba->get_accepting().size(); ++j) {
         for (auto & e : gnba->get_node(i)->get_transition()) {
            if (gnba->get_accepting()[j].find(e) == gnba->get_accepting()[j].end()) {
               nba->add_transition(nodes[i][j]->get_id(), nodes[e][j]->get_id());
            } else {
               nba->add_transition(nodes[i][j]->get_id(), nodes[e][(j + 1) % gnba->get_accepting().size()]->get_id());
            }
         }
      }
   }
   return nba;
}
