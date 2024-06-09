#ifndef EXPR_H
#define EXPR_H

#include <set>
#include <map>
#include <vector>
#include <memory>
#include <iostream>

enum class ExprType {
   TRUE, VAR, NEG, CONJ, DISJ, IMPL, NEXT, ALWAYS, EVENTUALLY, UNTIL
};

std::ostream &operator<<(std::ostream &os, ExprType type);

class Expr {
 protected:
   ExprType type;
   virtual std::ostream& print(std::ostream &os) const {
      return os << type;
   }
 public: 
   Expr(ExprType type) : type(type) {}
   Expr(const Expr &expr) : type(expr.type) {}
   bool is_unary() const {
      return type == ExprType::NEG || type == ExprType::NEXT || 
             type == ExprType::ALWAYS || type == ExprType::EVENTUALLY;
   }
   bool is_binary() const {
      return type == ExprType::CONJ || type == ExprType::DISJ || type == ExprType::IMPL || type == ExprType::UNTIL;
   }
   virtual ~Expr() {}
   ExprType get_type() const { return type; }
   friend std::ostream& operator<<(std::ostream &os, const Expr &expr);
};

typedef std::shared_ptr<Expr> ExprPtr;
typedef std::shared_ptr<const Expr> ConstExprPtr;

class VarExpr : public Expr {
 private:
   std::string var;
 public:
   VarExpr(std::string var) : Expr(ExprType::VAR), var(var) {}
   VarExpr(const VarExpr &expr) : Expr(expr), var(expr.var) {}
   std::string get_var() { return var; }
   std::ostream& print(std::ostream &os) const override {
      return os << var;
   }
};

class UnaryExpr : public Expr {
 private:
   ExprPtr expr;
 public:
   UnaryExpr(ExprType type, ExprPtr expr) : Expr(type) {
      this->expr = std::move(expr);
   }
   UnaryExpr(const UnaryExpr &expr) : Expr(expr), expr(expr.expr) {}
   ~UnaryExpr() {}
   void set_expr(ExprPtr expr) { this->expr = std::move(expr); }
   ExprPtr get_expr() { return expr; }
   std::ostream& print(std::ostream &os) const override {
      return os << get_type() << "(" << *expr << ")";
   }
};

class BinaryExpr : public Expr {
 private:
   ExprPtr left;
   ExprPtr right;
 public:
   BinaryExpr(ExprType type, ExprPtr left, ExprPtr right) : Expr(type) {
      this->left = std::move(left);
      this->right = std::move(right);
   }
   BinaryExpr(const BinaryExpr &expr) : Expr(expr), left(expr.left), right(expr.right) {}
   ~BinaryExpr() {}
   void set_left(ExprPtr left) { this->left = std::move(left); }
   void set_right(ExprPtr right) { this->right = std::move(right); }
   ExprPtr get_left() { return left; }
   ExprPtr get_right() { return right; }
   std::ostream& print(std::ostream &os) const override {
      return os << "(" << *left << " " << get_type() << " " << *right << ")";
   }
};

typedef std::shared_ptr<VarExpr> VarExprPtr;
typedef std::shared_ptr<UnaryExpr> UnaryExprPtr;
typedef std::shared_ptr<BinaryExpr> BinaryExprPtr;

bool ExprEqual(ExprPtr expr1, ExprPtr expr2);
ExprPtr ExprCalcNeg(ExprPtr expr);
ExprPtr ExprSimplify(ExprPtr expr);

class ExprSet {
 protected :
   std::vector<ExprPtr> exprs;
 public:
   ExprSet() {}
   ExprSet(const ExprSet &exprset) : exprs(exprset.exprs) {}
   ExprSet copy() { return ExprSet(*this); }
   std::vector<ExprPtr>& get_exprs() { return exprs; }
   bool contains(ExprPtr expr) {
      for (auto &e : exprs) {
         if (ExprEqual(e, expr)) return true;
      }
      return false;
   }
   void add(ExprPtr expr) { exprs.push_back(expr); }
};

class Elementary : public ExprSet {
 public:
   Elementary() {}
   Elementary(const Elementary &elementary) : ExprSet(elementary) {}
   Elementary(const ExprSet &exprset) : ExprSet(exprset) {}
   std::set<std::string> get_ap() {
      std::set<std::string> ap;
      for (auto &e : exprs) {
         if (e->get_type() == ExprType::VAR) {
            VarExprPtr var_expr = std::dynamic_pointer_cast<VarExpr>(e);
            ap.insert(var_expr->get_var());
         }
      }
      return ap;
   }
};

class Closure : public ExprSet {
 private:
   ExprPtr primary;
   std::map<ExprPtr, ExprPtr> negation;
   void build_closure(ExprPtr expr);
      
 public:
   Closure(ExprPtr primary) : primary(primary) { build_closure(primary); }
   int size() { return get_exprs().size(); }
   ExprPtr get_ith(int i) { return get_exprs()[i]; }
   ExprPtr get_negation(ExprPtr expr) { return negation[expr]; }
   int get_id(ExprPtr expr) { 
      for (std::vector<ExprPtr>::size_type i = 0; i < get_exprs().size(); ++i) {
         if (ExprEqual(get_exprs()[i], expr)) return i;
      }
      return -1;
   }
   bool contains(ExprPtr expr) {
      for (auto &e : get_exprs()) {
         if (ExprEqual(e, expr)) return true;
      }
      return false;
   }
   void add(ExprPtr expr, ExprPtr neg) {  
      ExprSet::add(expr);
      negation[expr] = neg;
      ExprSet::add(neg);
      negation[neg] = expr;
   }
   ExprPtr& get_primary() { return primary; }
   void print_closure() {
      for (auto &e : get_exprs()) {
         std::cout << *e << ", with negation: " << *negation[e] << std::endl;
      }
   }
};

class ElementarySet {
 private:
   std::shared_ptr<Closure> closure;
   std::vector<Elementary> elementaries;

   void build_elementary_helper(std::shared_ptr<Closure> closure, int pos, ExprSet &elementary);
   void build_elementary(std::shared_ptr<Closure> closure);

 public:
   ElementarySet(std::shared_ptr<Closure> closure) { build_elementary(closure); }
   std::shared_ptr<Closure> get_closure() { return closure; }
   std::vector<Elementary>& get_elementaries() { return elementaries; }
   void print_elementaries() {
      std::cout << "----------------------------------------\n";
      std::cout << "Elementaries: \n";
      for (auto &e : elementaries) {
         std::cout << "{";
         for (auto &expr : e.get_exprs()) {
            std::cout << *expr;
            if (expr != e.get_exprs().back()) std::cout << ", ";
            else std::cout << "}\n";
         }
      }
      std::cout << "----------------------------------------\n";
   }
};

#endif // EXPR_H