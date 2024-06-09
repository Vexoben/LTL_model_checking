#include <stack>
#include <assert.h>
#include "Utils.hpp"
#include "Parser.hpp"

std::ostream &operator<<(std::ostream &os, const Token &token) {
   switch (token.type) {
      case TOKEN_TYPE::NONE:
         os << "NONE";
         break;
      case TOKEN_TYPE::ENDLINE:
         os << "ENDLINE";
         break;
      case TOKEN_TYPE::LPAREN:
         os << "(";
         break;
      case TOKEN_TYPE::RPAREN:
         os << ")";
         break;
      case TOKEN_TYPE::VAR:
         os << "VAR(" << token.var_name << ")";
         break;
      case TOKEN_TYPE::NUMBER:
         os << "NUMBER(" << token.number << ")";
         break;
      case TOKEN_TYPE::NEG:
         os << "~";
         break;
      case TOKEN_TYPE::CONJ:
         os << "/\\";
         break;
      case TOKEN_TYPE::DISJ:
         os << "\\/";
         break;
      case TOKEN_TYPE::IMPLIES:
         os << "->";
         break;
      case TOKEN_TYPE::ALWAYS:
         os << "G";
         break;
      case TOKEN_TYPE::NEXT:
         os << "X";
         break;
      case TOKEN_TYPE::UNTIL:
         os << "U";
         break;
      case TOKEN_TYPE::EVENTUALLY:
         os << "F";
         break;
      default:
         os << "UNKNOWN";
         break;
   }
   return os;
}

void Parser::init() {
   current = tokenizer();
}

Token Parser::tokenizer() {
   static char c;
   static bool consumed = true;
   if (!consumed) {
      consumed = true;
   } else {
      if ((c = fin.get()) == EOF) c = 0;
   }
   if (!c) return Token(TOKEN_TYPE::NONE);
   else if (c == '(') return Token(TOKEN_TYPE::LPAREN);
   else if (c == ')') return Token(TOKEN_TYPE::RPAREN);
   else if (c == '!') return Token(TOKEN_TYPE::NEG);
   else if (c == 'X') return Token(TOKEN_TYPE::NEXT);
   else if (c == 'U') return Token(TOKEN_TYPE::UNTIL);
   else if (c == 'G') return Token(TOKEN_TYPE::ALWAYS);
   else if (c == 'F') return Token(TOKEN_TYPE::EVENTUALLY);
   else if (c == '\\') {
      fin.get(c);
      assert(c == '/');
      return Token(TOKEN_TYPE::DISJ);
   } else if (c == '/') {
      fin.get(c);
      assert(c == '\\');
      return Token(TOKEN_TYPE::CONJ);
   } else if (c == '-') {
      fin.get(c);
      assert(c == '>');
      return Token(TOKEN_TYPE::IMPLIES);
   } else if ('a' <= c && c <= 'z') {
      std::string var_name;
      while ('a' <= c && c <= 'z') {
         var_name.push_back(c); 
         if ((c = fin.get()) == EOF) break;
      }
      consumed = false;
      return Token(var_name);
   } else if ('0' <= c && c <= '9') {
      int number = 0;
      while ('0' <= c && c <= '9') {
         number = number * 10 + c - '0';
         if ((c = fin.get()) == EOF) break;
      }
      consumed = false;
      return Token(number);
   } else if (c == '\n') {
      return Token(TOKEN_TYPE::ENDLINE);
   }
   return tokenizer();
}

Token Parser::consume() {
#ifdef DEBUG
   std::cerr << "Consume: " << current << std::endl;   
#endif
   Token tmp(current);
   current = tokenizer();
#ifdef DEBUG
   std::cerr << "Get new token: " << current << std::endl;
#endif
   return tmp;
}

void Parser::consume_until_endline() {
   while (current.type != TOKEN_TYPE::ENDLINE && current.type != TOKEN_TYPE::NONE) consume();
   while (current.type == TOKEN_TYPE::ENDLINE) consume();
}

Token Parser::peek() {
   return current;
}

ExprPtr Parser::parse() {
   Token token = consume();
   ExprPtr left = nullptr;
   switch (token.type) {
      case TOKEN_TYPE::VAR: {
         left = std::make_unique<VarExpr>(token.var_name);
         break;
      }
      case TOKEN_TYPE::NEG: {
         left = std::make_unique<UnaryExpr>(ExprType::NEG, parse());
         break;
      }
      case TOKEN_TYPE::NEXT: {
         left = std::make_unique<UnaryExpr>(ExprType::NEXT, parse());
         break;
      }
      case TOKEN_TYPE::ALWAYS: {
         left = std::make_unique<UnaryExpr>(ExprType::ALWAYS, parse());
         break;
      }
      case TOKEN_TYPE::EVENTUALLY: {
         left = std::make_unique<UnaryExpr>(ExprType::EVENTUALLY, parse());
         break;
      }
      case TOKEN_TYPE::LPAREN: {
         left = parse();
         consume();
         break;
      }
      default: {
         failwith("Not a prefix token");
      }
   }
   while (true) {
      token = peek();
      if (!token.is_infix_token()) break;
      consume();
      ExprPtr right = parse();
      switch (token.type) {
         case TOKEN_TYPE::CONJ: {
            left = std::make_unique<BinaryExpr>(ExprType::CONJ, left, right);
            break;
         }
         case TOKEN_TYPE::DISJ: {
            left = std::make_unique<BinaryExpr>(ExprType::DISJ, left, right);
            break;
         }
         case TOKEN_TYPE::IMPLIES: {
            left = std::make_unique<BinaryExpr>(ExprType::IMPL, left, right);
            break;
         }
         case TOKEN_TYPE::UNTIL: {
            left = std::make_unique<BinaryExpr>(ExprType::UNTIL, left, right);
            break;
         }
         default: {
            failwith("Not an infix token");
         }
      }
   }
   return left;
}   