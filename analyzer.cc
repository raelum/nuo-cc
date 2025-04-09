#ifndef ANALYZER_CC
#define ANALYZER_CC

#include "ast.cc"
#include "builtins.cc"

struct Analyzer {
  Program* program;

  Result<None> analyzeProgram(Program& node) {
    this->program = &node;

    for (size_t i = 0; i < node.functions.size(); i++) {
      TRY(this->analyzeFunctionDeclaration(node.functions[i]));
    }
    return Ok();
  }

  Result<None> analyzeFunctionDeclaration(FunctionDeclaration& node) {
    // Validate main function.
    if (node.name == "main") {
      if (!node.returnType.equals(BaseType::VOID) &&
          !node.returnType.equals(BaseType::INT)) {
        return Error("main function can only return VOID or INT.");
      }
      node.returnType = Type(BaseType::INT);
    }
    TRY(this->analyzeStatementBlock(node.body));
    return Ok();
  }

  Result<None> analyzeStatementBlock(const StatementBlock& node) {
    if (node.statements.size() == 0) {
      return Error("Cannot have an empty statement block.");
    }
    for (size_t i = 0; i < node.statements.size(); i++) {
      TRY(this->analyzeStatement(node.statements[i]));
    }
    return Ok();
  }

  Result<None> analyzeStatement(const Statement& node) {
    if (std::holds_alternative<Unique<FunctionCall>>(node)) {
      TRY(this->analyzeFunctionCall(*std::get<Unique<FunctionCall>>(node)));
    }
    return Ok();
  }

  Result<None> analyzeFunctionCall(const FunctionCall& node) {
    if (node.name == "println") {
      this->addInclude("stdio.h");
    }
    return Ok();
  }

  void addInclude(String include) {
    this->program->includes.push_back(std::move(include));
  }
};

#endif  // ANALYZER_CC