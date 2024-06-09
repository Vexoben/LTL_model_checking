#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <iostream>
#include "Expr.hpp"

enum class TOKEN_TYPE {
   NONE, ENDLINE,
   LPAREN, RPAREN,
   VAR, NUMBER,
   NEG,
   CONJ, DISJ, IMPLIES,
   ALWAYS, NEXT, UNTIL, EVENTUALLY
};

class Token {
 public:
   TOKEN_TYPE type;
   int number;
   std::string var_name;
   Token() : type(TOKEN_TYPE::NONE), number(0), var_name("") {}
   Token(TOKEN_TYPE type) : type(type), number(0), var_name("") {}
   Token(int number) : type(TOKEN_TYPE::NUMBER), number(number) {}
   Token(std::string var_name) : type(TOKEN_TYPE::VAR), var_name(var_name) {}
   Token(const Token &obj)
      : type(obj.type), number(obj.number), var_name(obj.var_name) {}
   Token& operator = (const Token &obj) {
      type = obj.type;
      var_name = obj.var_name;
      number = obj.number;
      return *this;
   }
   bool is_infix_token() {
      return type == TOKEN_TYPE::CONJ || type == TOKEN_TYPE::DISJ || 
             type == TOKEN_TYPE::IMPLIES || type == TOKEN_TYPE::UNTIL;
   }
   friend std::ostream &operator<<(std::ostream &os, const Token &token);
};

class Parser {
 private:
   std::istream &fin;
   Token current;
   Token tokenizer();
 public:
   void init();
   Parser(std::istream &fin) : fin(fin) { init(); }
   Token consume();
   void consume_until_endline();
   Token peek();
   ExprPtr parse();
};

#endif