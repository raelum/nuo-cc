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

  Result<None> printStatement(const Statement& node, int level) {
    if (std::holds_alternative<Unique<FunctionCall>>(node)) {
      TRY(this->printFunctionCall(*std::get<Unique<FunctionCall>>(node),
                                  level));
      return Ok();
    }
    if (std::holds_alternative<Unique<Return>>(node)) {
      TRY(this->printReturn(*std::get<Unique<Return>>(node), level));
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
    }
    if (std::holds_alternative<Unique<NumberLiteral>>(node)) {
      TRY(this->printNumberLiteral(*std::get<Unique<NumberLiteral>>(node),
                                   level));
      return Ok();
    }
    if (std::holds_alternative<Unique<StringLiteral>>(node)) {
      TRY(this->printStringLiteral(*std::get<Unique<StringLiteral>>(node),
                                   level));
      return Ok();
    }
    return Error("Unexpected Expression of index {} when converting to String.",
                 node.index());
  }

  Result<None> printType(const Type& type) {
    if (type.isBaseType()) {
      TRY(this->printBaseType(type.getBaseType()));
      return Ok();
    } else if (type.isListType()) {
      TRY(this->printListType(type.getListType()));
      return Ok();
    }
    return Error("Unexpected Type with index {} when printing AST.",
                 type.typeVariant.index());
  }

  Result<None> printListType(const ListType& listType) {
    this->out << "[";
    TRY(this->printBaseType(listType.elementType));
    this->out << "]";
    return Ok();
  }

  Result<None> printBaseType(const BaseType& type) {
    this->out << baseTypeToString(type);
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

  Result<None> printNumberLiteral(const NumberLiteral& node, int level) {
    this->indent(level);
    this->out << node.value;
    return Ok();
  }

  Result<None> printStringLiteral(const StringLiteral& node, int level) {
    this->indent(level);
    this->out << node.value;
    return Ok();
  }

  Result<None> printReturn(const Return& node, int level) {
    this->indent(level);
    this->out << "Return:\n";
    if (node.expression.has_value()) {
      TRY(this->printExpression(node.expression.value(), level + 1));
    } else {
      this->indent(level + 1);
      this->out << "VOID";
    }
    return Ok();
  }
};

#endif  // AST_PRINTER_CC