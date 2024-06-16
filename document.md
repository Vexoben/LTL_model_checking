## LTL Algorithm

### Code Structures

The codes consists of the following files:

- `Expr.cpp` : The definition of the expression tree. Also contains the definition of closure and elementary sets. Some conversion functions are also defined here.

- `Parser.cpp` : Use Pratt Parsing to parse the LTL formula. It is assumed that the input formula has enough parentheses to make the parsing unambiguous, so the parser does not need to handle the precedence of operators.

- `NBA.cpp` : The definition of the NBA(non-deterministic Buchi automaton) and GNBA(generalized NBA). The formula will first be converted to a GNBA and then to a NBA.

- `TS.hpp` : The definition of the transition system.

- `Product.cpp` : Calculate the product of NBA and TS.

- `NestedDFS.cpp` : The nested DFS algorithm.

- `SCCProcessor.cpp` : Calculate the strongly connected components of the product by Tarjan's algorithm.

- `Utils.hpp` : Defines the `failwith` macro for debugging.

- `main.cpp` : The main function of the program.

### Algorithm

This code implements the LTL model checking algorithm which can be found in the book "Principles of Model Checking".

It first parses the LTL formula $\varphi$ and converts $\neg\varphi$ into GNBA, and later converts into NBA $\mathcal A$ such that $L(\mathcal A) = L(\neg\varphi)$. 

Then it calculates the product of the NBA and the transition system.

In the end, it uses the nested DFS algorithm to check whether a node in the accepting set is reachable from the initial states and is contained in a circle.

An alternating algorithm of nested DFS is Tarjan's algorithm. If a node in accepting set is reachable from the initial states and is in a SCC(strongly connected components) that contains a circle(that means the size of SCC >= 2, or the SCC contains a self-loop), then the formula is not satisfied. Both nested DFS and Tarjan's algorithm are implemented.  

### Data Structures

#### Expression Tree

Expr is a abstract class that represents the expression tree. The VarExpr, UnaryExpr, BinaryExpr class are derived from Expr. The VarExpr represents the atomic proposition, UnaryExpr represents the unary operator, and BinaryExpr represents the binary operator. 

```cpp
// Some code is omitted for brevity
enum class ExprType {
   TRUE, VAR, NEG, CONJ, DISJ, IMPL, NEXT, ALWAYS, EVENTUALLY, UNTIL
};

class Expr {
 protected:
   ExprType type;
};

typedef std::shared_ptr<Expr> ExprPtr;

class VarExpr : public Expr {
 private:
   std::string var;
};

class UnaryExpr : public Expr {
 private:
   ExprPtr expr;
};

class BinaryExpr : public Expr {
 private:
   ExprPtr left;
   ExprPtr right;
};
```

#### Closure and Elementary Sets

ExprSet contains a vector of ExprPtr. It offers the functions to check if an expression is in the set(by comparing the syntax tree). 

Closure is inherited from ExprSet. It represents a ExprSet that is closed under the negation operator. It also contains the primary expression and a map that maps evry expression to its negation.

Elemetary is also inherited from ExprSet. It represents a ExprSet that is elementary.

ElemetarySet is a class that contains a vector of Elementary.

```cpp
// Some code is omitted for brevity
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

class Closure : public ExprSet {
 private:
   ExprPtr primary;
   std::map<ExprPtr, ExprPtr> negation;
};

class Elementary : public ExprSet {};

class ElementarySet {
 private:
   std::shared_ptr<Closure> closure;
   std::vector<Elementary> elementaries;
};
```

#### NBA and GNBA

NBANode is the class for node in both NBA and GNBA. Since the NBA in this algorithm only contains the transitions that are labeled with the atomic propositions of elementary set, the node contains a set of atomic propositions.

NBA_Base is an abstract class that represents the NBA and GNBA. NBA and GNBA are derived from NBA_Base. The only difference between NBA and GNBA is the acceptance condition.

```cpp
// Some code is omitted for brevity
class NBANode {
 private:
   int id;
   int is_initial;
   int is_accepting;
   std::set<std::string> ap;
   std::set<int> transition;
};

class NBA_base {
 protected:
   int node_count;
   std::vector<int> initial;
   std::set<std::string> aps;
   std::vector<NBANodePtr> nodes;
   std::map<int, NBANodePtr> node_map;
};

class NBA : public NBA_base {
 private:
   std::set<int> accepting;
};

class GNBA : public NBA_base {
 private:
   std::vector<std::set<int>> accepting;
};
```

#### Transition System

Actions are not recorded in the transition system since the algorithm only needs to know the set of atomic propositions of the state.

```cpp
// Some code is omitted for brevity
class TSNode {
 private:
   int id;
   int is_initial;
   std::set<std::string> ap;
   std::set<int> transition;
};

typedef std::shared_ptr<TSNode> TSNodePtr;

class TS {
 private:
   int node_count;
   std::vector<int> initial;
   std::vector<TSNodePtr> nodes;
   std::set<std::string> ap;
};
```

### Running the Code

The code has a `CMakeLists.txt` file. You can build and run the code by running the following commands:

```bash
mkdir build
cd build
cmake ..
make
make run
```

You can change the input file by changing the `ts_in_path` and `ltl_in_path` defined in the main function.