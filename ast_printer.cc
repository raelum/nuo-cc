#ifndef AST_PRINTER_CC
#define AST_PRINTER_CC

#include "ast.cc"
#include "builtins.cc"

struct AstPrinter {
  StringStream out;

  Result<String> print(const Program& program) {
    // Empty the output buffer in case this was called before.
    out.str("");
    for (size_t i = 0; i < program.functions.size(); i++) {
      TRY(this->print(program.functions[i], 0));
      if (i < program.functions.size() - 1) {
        out << "\n";
      }
    }
    return Ok(out.str());
  }

  void indent(int level) {
    for (int i = 0; i < level; i++) {
      this->out << "  ";
    }
  }

  Result<None> print(const FunctionDeclaration& funcDecl, int level) {
    this->indent(level);
    this->out << "FunctionDeclaration: " << funcDecl.name << "\n";

    this->indent(level + 1);
    this->out << "params:\n";

    for (size_t i = 0; i < funcDecl.params.size(); i++) {
      this->indent(level + 2);
      this->out << funcDecl.params[i].name << ": ";
      TRY(this->print(funcDecl.params[i].type));
      this->out << "\n";
    }

    this->indent(level + 1);
    this->out << "returnType: ";
    TRY(this->print(funcDecl.returnType));
    this->out << "\n";

    this->indent(level + 1);
    this->out << "body:\n";
    TRY(print(funcDecl.body, level + 2));

    return Ok();
  }

  Result<None> print(const StatementBlock& block, int level) {
    for (size_t i = 0; i < block.statements.size(); i++) {
      TRY(this->print(block.statements[i], level));
    }
    return Ok();
  }

  Result<None> print(const FunctionCall& functionCall, int level) {
    this->indent(level);
    this->out << "FunctionCall: " << functionCall.name << "\n";

    for (size_t i = 0; i < functionCall.args.size(); i++) {
      TRY(this->print(functionCall.args[i], level + 1));
    }
    return Ok();
  }

  void print(const StringLiteral& stringLiteral, int level) {
    this->indent(level);
    this->out << stringLiteral.value;
  }

  Result<None> print(const Type& type) {
    if (std::holds_alternative<BaseType>(type)) {
      this->print(std::get<BaseType>(type));
      return Ok();
    } else if (std::holds_alternative<ListType>(type)) {
      this->print(std::get<ListType>(type));
      return Ok();
    }
    return Error("Unexpected Type with index {} when printing AST.",
                 type.index());
  }

  void print(const ListType& listType) {
    this->out << "[";
    this->print(listType.elementType);
    this->out << "]";
  }

  void print(const BaseType& type) { this->out << baseTypeToString(type); }

  Result<None> print(const Statement& statement, int level) {
    if (std::holds_alternative<Unique<FunctionCall>>(statement)) {
      TRY(this->print(*std::get<Unique<FunctionCall>>(statement).get(), level));
      return Ok();
    }
    return Error("Unexpected Statement of index {} when converting to String.",
                 statement.index());
  }

  Result<None> print(const Expression& expression, int level) {
    if (std::holds_alternative<Unique<FunctionCall>>(expression)) {
      TRY(this->print(*std::get<Unique<FunctionCall>>(expression).get(),
                      level));
      return Ok();
    } else if (std::holds_alternative<Unique<StringLiteral>>(expression)) {
      this->print(*std::get<Unique<StringLiteral>>(expression).get(), level);
      return Ok();
    }
    return Error("Unexpected Expression of index {} when converting to String.",
                 expression.index());
  }
};

#endif  // AST_PRINTER_CC