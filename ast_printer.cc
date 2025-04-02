#ifndef AST_PRINTER_CC
#define AST_PRINTER_CC

#include "ast.cc"
#include "builtins.cc"

struct AstPrinter {
  StringStream out;

  Result<String> printProgram(const Program& node) {
    // Empty the output buffer in case this was called before.
    out.str("");
    for (size_t i = 0; i < node.functions.size(); i++) {
      TRY(this->printFunctionDeclaration(node.functions[i], 0));
      if (i < node.functions.size() - 1) {
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

  Result<None> printFunctionDeclaration(const FunctionDeclaration& node,
                                        int level) {
    this->indent(level);
    this->out << "FunctionDeclaration: " << node.name << "\n";

    this->indent(level + 1);
    this->out << "params:\n";
    for (size_t i = 0; i < node.params.size(); i++) {
      this->indent(level + 2);
      this->out << node.params[i].name << ": ";
      TRY(this->printType(node.params[i].type));
      this->out << "\n";
    }

    this->indent(level + 1);
    this->out << "returnType: ";
    TRY(this->printType(node.returnType));
    this->out << "\n";

    this->indent(level + 1);
    this->out << "body:\n";
    TRY(this->printStatementBlock(node.body, level + 2));

    return Ok();
  }

  Result<None> printStatementBlock(const StatementBlock& node, int level) {
    for (size_t i = 0; i < node.statements.size(); i++) {
      TRY(this->printStatement(node.statements[i], level));
    }
    return Ok();
  }

  Result<None> printFunctionCall(const FunctionCall& node, int level) {
    this->indent(level);
    this->out << "FunctionCall: " << node.name << "\n";

    for (size_t i = 0; i < node.args.size(); i++) {
      TRY(this->printExpression(node.args[i], level + 1));
    }
    return Ok();
  }

  void printStringLiteral(const StringLiteral& node, int level) {
    this->indent(level);
    this->out << node.value;
  }

  Result<None> printType(const Type& type) {
    if (std::holds_alternative<BaseType>(type)) {
      this->printBaseType(std::get<BaseType>(type));
      return Ok();
    } else if (std::holds_alternative<ListType>(type)) {
      this->printListType(std::get<ListType>(type));
      return Ok();
    }
    return Error("Unexpected Type with index {} when printing AST.",
                 type.index());
  }

  void printListType(const ListType& listType) {
    this->out << "[";
    this->printBaseType(listType.elementType);
    this->out << "]";
  }

  void printBaseType(const BaseType& type) {
    this->out << baseTypeToString(type);
  }

  Result<None> printStatement(const Statement& node, int level) {
    if (std::holds_alternative<Unique<FunctionCall>>(node)) {
      TRY(this->printFunctionCall(*std::get<Unique<FunctionCall>>(node),
                                  level));
      return Ok();
    }
    return Error("Unexpected Statement of index {} when converting to String.",
                 node.index());
  }

  Result<None> printExpression(const Expression& node, int level) {
    if (std::holds_alternative<Unique<FunctionCall>>(node)) {
      TRY(this->printFunctionCall(*std::get<Unique<FunctionCall>>(node),
                                  level));
      return Ok();
    } else if (std::holds_alternative<Unique<StringLiteral>>(node)) {
      this->printStringLiteral(*std::get<Unique<StringLiteral>>(node), level);
      return Ok();
    }
    return Error("Unexpected Expression of index {} when converting to String.",
                 node.index());
  }
};

#endif  // AST_PRINTER_CC