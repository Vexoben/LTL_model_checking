#include "TS.hpp"
#include "NBA.hpp"
#include "Expr.hpp"
#include "Parser.hpp"
#include "Product.hpp"
#include "SCCProcessor.hpp"
#include <assert.h>
#include <fstream>
#include <iostream>

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

int read_number(Parser &parser) {
   Token token = parser.consume();
   assert(token.type == TOKEN_TYPE::NUMBER);
   return token.number;
}

// check if the TS satisfies the LTL formula
int CheckLTL(std::shared_ptr<TS> ts, std::shared_ptr<NBA> nba) {
   std::shared_ptr<TS> prod = ProductTSWithNBA(ts, nba);
   std::shared_ptr<SCCProcessor> scc = std::make_shared<SCCProcessor>(prod->get_node_count());
   for (int i = 0; i < prod->get_node_count(); ++i) {
      for (auto &to : prod->get_node(i)->get_transition()) {
         scc->add_edge(i, to);
      }
   }
   scc->calc_scc();
   std::vector<int> reachable = scc->reachable_from(prod->get_initial());
   for (auto &id : reachable) {
      TSNodePtr node = prod->get_node(id);
      if (node->get_ap().find("accepting") != node->get_ap().end()) {
         int belong = scc->get_scc_belong(id);
         if (scc->scc_contains_circle(belong)) {
            return 0;
         }
      }
   }
   return 1;
}

// read TS from input stream
std::shared_ptr<TS> InputTS(std::istream &fin) {
   std::shared_ptr<TS> ts = std::make_shared<TS>();
   int n, m;
   Parser parser(fin);
   Token token;
   n = read_number(parser);
   m = read_number(parser);
   parser.consume_until_endline();
   std::set<int> initials;
   while (1) {
      token = parser.consume();
      if (token.type == TOKEN_TYPE::ENDLINE || token.type == TOKEN_TYPE::NONE) break;
      assert(token.type == TOKEN_TYPE::NUMBER);
      initials.insert(token.number);
   }
   parser.consume_until_endline();
   std::vector<std::string> aps;
   while (1) {
      token = parser.consume();
      if (token.type == TOKEN_TYPE::ENDLINE || token.type == TOKEN_TYPE::NONE) break;
      assert(token.type == TOKEN_TYPE::VAR);
      aps.push_back(token.var_name);
   }
   ts->set_ap(std::set<std::string>(aps.begin(), aps.end()));
   std::vector<std::vector<int>> transition(n);
   for (int i = 0; i < m; ++i) {
      int from, to;
      from = read_number(parser);
      read_number(parser);                   // ignore action
      to = read_number(parser);
      parser.consume_until_endline();
      transition[from].push_back(to);
   }
   for (int i = 0; i < n; ++i) {
      std::set<std::string> ap;
      while (1) {
         token = parser.consume();
         if (token.type == TOKEN_TYPE::ENDLINE || token.type == TOKEN_TYPE::NONE) break;
         assert(token.type == TOKEN_TYPE::NUMBER);
         if (token.number > -1) ap.insert(aps[token.number]);
      }
      TSNodePtr node = std::make_shared<TSNode>(i, initials.find(i) != initials.end(), ap);
      ts->add_node(node);
   }
   for (int i = 0; i < n; ++i) {
      for (auto &to : transition[i]) {
         ts->add_transition(i, to);
      }
   }
   return ts;
}

// read LTL expression and transform it to NBA
std::shared_ptr<NBA> ParseExprAndTrans(Parser & parser) {
   ExprPtr expr = parser.parse();
   expr = ExprSimplify(std::make_shared<UnaryExpr>(ExprType::NEG, expr));
   std::shared_ptr<Closure> closure = std::make_shared<Closure>(expr);
   ElementarySet elementaries(closure);
   std::shared_ptr<GNBA> gnba = LTL_to_GNBA(std::make_shared<ElementarySet>(elementaries));
   std::shared_ptr<NBA> nba = GNBA_to_NBA(gnba);
   parser.consume_until_endline();
   return nba;
}

void InputLTL(std::shared_ptr<TS> ts, std::istream &fin) {
   int n, m;
   Parser parser(fin);
   Token token;
   n = read_number(parser);
   m = read_number(parser);
   parser.consume_until_endline();
   for (int i = 1; i <= n; ++i) {
      std::shared_ptr<NBA> nba = ParseExprAndTrans(parser);
      std::cout << CheckLTL(ts, nba) << std::endl;
   }
   for (int i = 1; i <= m; ++i) {
      token = parser.consume();
      assert(token.type == TOKEN_TYPE::NUMBER);
      int id = token.number;
      std::shared_ptr<NBA> nba = ParseExprAndTrans(parser);
      std::shared_ptr<TS> new_ts = ts->adjust_initial(id);
      std::cout << CheckLTL(new_ts, nba) << std::endl;
   }
}

int main() {
   std::string project_root_dir = std::string(STR(PROJECT_ROOT_DIR));
   project_root_dir = project_root_dir.substr(1, project_root_dir.size() - 2);
   std::string ts_in_path = project_root_dir + "/testcases/TS.txt";
   std::string ltl_in_path = project_root_dir + "/testcases/benchmark1.txt";
   std::ifstream ts_in(ts_in_path);
   if (!ts_in.is_open()) {
      std::cerr << "Cannot open file " << ts_in_path << std::endl;
      return 1;
   }
   std::ifstream ltl_in(ltl_in_path);
   if (!ltl_in.is_open()) {
      std::cerr << "Cannot open file " << ltl_in_path << std::endl;
      return 1;
   }
   std::shared_ptr<TS> ts = InputTS(ts_in);
   InputLTL(ts, ltl_in);
   return 0;
}

#undef QUOTE
#undef STR