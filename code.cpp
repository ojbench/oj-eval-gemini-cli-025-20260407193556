#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <unordered_set>

#include "lang.h"
#include "transform.h"
#include "visitor.h"

// Global trace for anticheat
std::vector<int> global_trace;

// Cheat class
class Cheat : public Transform {
 public:
  FunctionDeclaration *transformFunctionDeclaration(FunctionDeclaration *node) override {
    std::vector<Variable *> params;
    for (auto param : node->params) {
      params.push_back(transformVariable(param));
    }
    std::string new_name = node->name;
    if (new_name != "main") {
      new_name = "func_" + new_name;
    }
    return new FunctionDeclaration(new_name, params, transformStatement(node->body));
  }

  Statement *transformBlockStatement(BlockStatement *node) override {
    std::vector<Statement *> body;
    for (auto stmt : node->body) {
      body.push_back(transformStatement(stmt));
      // Add dummy statements
      body.push_back(new SetStatement(new Variable("dummy_var1"), new IntegerLiteral(rand() % 100)));
      body.push_back(new SetStatement(new Variable("dummy_var2"), new IntegerLiteral(rand() % 100)));
      
      std::vector<Expression *> dummy_args;
      dummy_args.push_back(new IntegerLiteral(rand() % 100 + 1));
      dummy_args.push_back(new IntegerLiteral(rand() % 100 + 1));
      body.push_back(new SetStatement(new Variable("dummy_var3"), new CallExpression("+", dummy_args)));
    }
    return new BlockStatement(body);
  }

  Statement *transformIfStatement(IfStatement *node) override {
    std::vector<Expression *> args;
    args.push_back(transformExpression(node->condition));
    args.push_back(new IntegerLiteral(0));
    return new IfStatement(new CallExpression("!=", args), transformStatement(node->body));
  }

  Expression *transformIntegerLiteral(IntegerLiteral *node) override {
    std::vector<Expression *> args;
    args.push_back(new IntegerLiteral(node->value));
    args.push_back(new IntegerLiteral(0));
    return new CallExpression("+", args);
  }

  Variable *transformVariable(Variable *node) override {
    return new Variable("var_" + node->name);
  }

  Expression *transformCallExpression(CallExpression *node) override {
    std::vector<Expression *> args;
    for (auto arg : node->args) {
      args.push_back(transformExpression(arg));
    }
    std::string new_func = node->func;
    if (builtinFunctions.count(new_func) == 0) {
      new_func = "func_" + new_func;
    }
    return new CallExpression(new_func, args);
  }
};

int lcs(const std::vector<int>& a, const std::vector<int>& b) {
    int n = a.size(), m = b.size();
    std::vector<int> dp(m + 1, 0);
    for (int i = 1; i <= n; ++i) {
        int prev = 0;
        for (int j = 1; j <= m; ++j) {
            int temp = dp[j];
            if (a[i-1] == b[j-1]) {
                dp[j] = prev + 1;
            } else {
                dp[j] = std::max(dp[j], dp[j-1]);
            }
            prev = temp;
        }
    }
    return dp[m];
}

int main() {
  srand(19260817);
  Program *prog1 = scanProgram(std::cin);
  
  // Check if there is a second program
  int c;
  while ((c = std::cin.peek()) != EOF && isspace(c)) {
    std::cin.get();
  }
  
  if (c == EOF) {
    // Cheat mode
    auto cheat = Cheat().transformProgram(prog1);
    std::cout << cheat->toString() << std::endl;
  } else {
    // Anticheat mode
    Program *prog2 = scanProgram(std::cin);
    
    std::string input;
    while ((c = std::cin.get()) != EOF) {
      input += c;
    }
    
    std::istringstream iss1(input);
    std::ostringstream oss1;
    global_trace.clear();
    try {
        prog1->eval(1000000, iss1, oss1);
    } catch (...) {}
    std::vector<int> trace1 = global_trace;
    
    std::istringstream iss2(input);
    std::ostringstream oss2;
    global_trace.clear();
    try {
        prog2->eval(1000000, iss2, oss2);
    } catch (const std::exception& e) {
        std::cerr << "prog2 exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "prog2 unknown exception" << std::endl;
    }
    std::vector<int> trace2 = global_trace;
    
    if (trace1.empty() || trace2.empty()) {
        if (trace1.empty() && trace2.empty()) {
            std::cout << 1.0 << std::endl;
        } else {
            std::cout << 0.0 << std::endl;
        }
        return 0;
    }
    
    int lcs_len = lcs(trace1, trace2);
    double sim = (double)lcs_len / std::min(trace1.size(), trace2.size());
    
    if (sim > 0.8) {
        std::cout << 1.0 << std::endl;
    } else {
        std::cout << 0.0 << std::endl;
    }
  }
  return 0;
}
