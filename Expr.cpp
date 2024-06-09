#include <memory>
#include "Expr.hpp"

// Check the equality of two expressions by comparing the syntax tree
bool ExprEqual(ExprPtr expr1, ExprPtr expr2) {
   if (expr1->get_type() != expr2->get_type()) return false;
   if (expr1->get_type() == ExprType::VAR) {
      VarExprPtr var_expr1 = std::dynamic_pointer_cast<VarExpr>(expr1);
      VarExprPtr var_expr2 = std::dynamic_pointer_cast<VarExpr>(expr2);
      return var_expr1->get_var() == var_expr2->get_var();
   } else if (expr1->is_unary()) {
      UnaryExprPtr unary_expr1 = std::dynamic_pointer_cast<UnaryExpr>(expr1);
      UnaryExprPtr unary_expr2 = std::dynamic_pointer_cast<UnaryExpr>(expr2);
      return ExprEqual(unary_expr1->get_expr(), unary_expr2->get_expr());
   } else if (expr1->is_binary()) {
      BinaryExprPtr binary_expr1 = std::dynamic_pointer_cast<BinaryExpr>(expr1);
      BinaryExprPtr binary_expr2 = std::dynamic_pointer_cast<BinaryExpr>(expr2);
      return ExprEqual(binary_expr1->get_left(), binary_expr2->get_left()) && 
             ExprEqual(binary_expr1->get_right(), binary_expr2->get_right());
   }
   return true;
}

// Calculate the negation of an expression(Will eliminate double negation)
ExprPtr ExprCalcNeg(ExprPtr expr) {
   if (expr->get_type() == ExprType::NEG) {
      UnaryExprPtr unary_expr = std::dynamic_pointer_cast<UnaryExpr>(expr);
      return unary_expr->get_expr();
   } else {
      return std::make_shared<UnaryExpr>(ExprType::NEG, expr);
   }
}

// Simplify the expression
// eliminate double negation
// eliminate \/, ->, always, eventually
ExprPtr ExprSimplify(ExprPtr expr) {
   if (expr->get_type() == ExprType::NEG || expr->get_type() == ExprType::ALWAYS || 
            expr->get_type() == ExprType::EVENTUALLY || expr->get_type() == ExprType::NEXT) {
      UnaryExprPtr unary_expr = std::dynamic_pointer_cast<UnaryExpr>(expr);
      unary_expr->set_expr(ExprSimplify(unary_expr->get_expr()));
      if (expr->get_type() == ExprType::NEG) {                                 // !!a = a
         if (unary_expr->get_expr().get()->get_type() == ExprType::NEG) {
            UnaryExprPtr sub_unary_expr = std::dynamic_pointer_cast<UnaryExpr>(unary_expr->get_expr());
            expr = ExprSimplify(sub_unary_expr->get_expr());
         }
      } else if (expr->get_type() == ExprType::ALWAYS) {                       // always P = !eventually !P
         expr = std::make_shared<UnaryExpr>(ExprType::NEG, 
                  std::make_shared<UnaryExpr>(ExprType::EVENTUALLY, 
                     std::make_shared<UnaryExpr>(ExprType::NEG, unary_expr->get_expr())));
         expr = ExprSimplify(expr);
      } else if (expr->get_type() == ExprType::EVENTUALLY) {                   // eventually P = true U P
         expr = std::make_shared<BinaryExpr>(ExprType::UNTIL, 
                     std::make_shared<Expr>(ExprType::TRUE), unary_expr->get_expr());
      }
   } else if (expr->get_type() == ExprType::DISJ || expr->get_type() == ExprType::IMPL || 
              expr->get_type() == ExprType::CONJ || expr->get_type() == ExprType::UNTIL) {
      BinaryExprPtr binary_expr = std::dynamic_pointer_cast<BinaryExpr>(expr);
      binary_expr->set_left(ExprSimplify(binary_expr->get_left()));
      binary_expr->set_right(ExprSimplify(binary_expr->get_right()));
      if (expr->get_type() == ExprType::DISJ) {                                // a \/ b = !(!a /\ !b)
         expr = std::make_shared<UnaryExpr>(ExprType::NEG, 
                  std::make_shared<BinaryExpr>(ExprType::CONJ, 
                     std::make_shared<UnaryExpr>(ExprType::NEG, binary_expr->get_left()), 
                     std::make_shared<UnaryExpr>(ExprType::NEG, binary_expr->get_right())));
         expr = ExprSimplify(expr);
      } else if (expr->get_type() == ExprType::IMPL) {                         // a -> b = !a \/ b
         expr = std::make_shared<BinaryExpr>(ExprType::DISJ, 
                  std::make_shared<UnaryExpr>(ExprType::NEG, binary_expr->get_left()), binary_expr->get_right());
         expr = ExprSimplify(expr);
      }
   }
   return expr;
}

// Build the closure of the expression
void Closure::build_closure(ExprPtr expr) {
   if (!contains(expr)) {
      ExprPtr neg = ExprCalcNeg(expr);
      add(expr, neg);
   }
   if (expr->is_unary()) {
      UnaryExprPtr unary_expr = std::dynamic_pointer_cast<UnaryExpr>(expr);
      build_closure(unary_expr->get_expr());
   } else if (expr->is_binary()) {
      BinaryExprPtr binary_expr = std::dynamic_pointer_cast<BinaryExpr>(expr);
      build_closure(binary_expr->get_left());
      build_closure(binary_expr->get_right());
   }
}

// Check if the a set of expressions is elementary
bool IsElementary(std::shared_ptr<Closure> closure, ExprSet &elementary) {
   // consistent with respect to propositional logic
   for (int i = 0; i < closure->size(); ++i) {
      ExprPtr expr = closure->get_ith(i);
      ExprPtr neg = ExprCalcNeg(expr);
      bool flag1 = elementary.contains(expr);
      bool flag2 = elementary.contains(neg);
      if (!(flag1 ^ flag2)) return false;
   }
   for (int i = 0; i < closure->size(); ++i) {
      ExprPtr expr = closure->get_ith(i);
      if (expr->get_type() == ExprType::CONJ) {
         ExprPtr left = std::dynamic_pointer_cast<BinaryExpr>(expr)->get_left();
         ExprPtr right = std::dynamic_pointer_cast<BinaryExpr>(expr)->get_right();
         bool flag1 = elementary.contains(expr);
         bool flag2 = elementary.contains(left) && elementary.contains(right);
         if (flag1 != flag2) return false;
      }
   }
   {
      ExprPtr true_expr = std::make_shared<Expr>(ExprType::TRUE);
      bool flag1 = elementary.contains(true_expr);
      bool flag2 = closure->contains(true_expr);
      if (flag1 != flag2) return false;
   }
   // local consistency
   for (int i = 0; i < closure->size(); ++i) {
      ExprPtr expr = closure->get_ith(i);
      if (expr->get_type() == ExprType::UNTIL) {
         ExprPtr left = std::dynamic_pointer_cast<BinaryExpr>(expr)->get_left();
         ExprPtr right = std::dynamic_pointer_cast<BinaryExpr>(expr)->get_right();
         if (elementary.contains(right) && !elementary.contains(expr)) {
            return false;
         }
         if (elementary.contains(expr) && !elementary.contains(left) && !elementary.contains(right)) {
            return false;
         }
      }
   }
   return true;
}

void ElementarySet::build_elementary_helper(std::shared_ptr<Closure> closure, int pos, ExprSet &elementary) {
   if (pos == closure->size()) {
      if (IsElementary(closure, elementary)) {
         elementaries.push_back(elementary.copy());
      }
      return;
   }
   ExprPtr expr = closure->get_ith(pos);
   if (!elementary.contains(ExprCalcNeg(expr))) {
      elementary.get_exprs().push_back(expr);
      build_elementary_helper(closure, pos + 1, elementary);
      elementary.get_exprs().pop_back();
   }
   build_elementary_helper(closure, pos + 1, elementary);
}

// calculate the elementary set
void ElementarySet::build_elementary(std::shared_ptr<Closure> closure) {
   ExprSet elementary;
   build_elementary_helper(closure, 0, elementary);
   this->closure = closure;
}

std::ostream &operator<<(std::ostream &os, ExprType type) {
   switch (type) {
      case ExprType::TRUE:
         os << "TRUE";
         break;
      case ExprType::VAR:
         os << "VAR";
         break;
      case ExprType::NEG:
         os << "!";
         break;
      case ExprType::CONJ:
         os << "/\\";
         break;
      case ExprType::DISJ:
         os << "\\/";
         break;
      case ExprType::IMPL:
         os << "->";
         break;
      case ExprType::NEXT:
         os << "X";
         break;
      case ExprType::ALWAYS:
         os << "G";
         break;
      case ExprType::EVENTUALLY:
         os << "F";
         break;
      case ExprType::UNTIL:
         os << "U";
         break;
   }
   return os;
}

std::ostream& operator<<(std::ostream &os, const Expr &expr) {
   return expr.print(os);
}