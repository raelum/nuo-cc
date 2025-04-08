#ifndef COMPILER_CC
#define COMPILER_CC

#include "ast.cc"
#include "builtins.cc"

const size_t INDENT_SIZE = 2;

struct Compiler {
  StringStream out;
  size_t indent = 0;

  Result<String> compileProgram(const Program& node) {
    // Empty the output buffer in case this was called before.
    out.str("");

    for (size_t i = 0; i < node.includes.size(); i++) {
      this->out << "#include <" << node.includes[i] << ">\n";
    }
    this->out << "\n";

    for (size_t i = 0; i < node.functions.size(); i++) {
      TRY(this->compileFunctionDeclaration(node.functions[i]));
      if (i < node.functions.size() - 1) {
        this->out << "\n\n";
      }
    }

    return Ok(out.str());
  }

  Result<None> compileFunctionDeclaration(const FunctionDeclaration& node) {
    TRY(this->compileType(node.returnType));
    this->out << " " << node.name;

    this->out << "(";
    for (size_t i = 0; i < node.params.size(); i++) {
      TRY(this->compileType(node.params[i].type));
      this->out << " " << node.params[i].name;
      if (i < node.params.size() - 1) {
        this->out << ", ";
      }
    }
    this->out << ") ";

    TRY(this->compileStatementBlock(node.body));

    return Ok();
  }

  Result<None> compileType(const Type& type) {
    if (std::holds_alternative<BaseType>(type)) {
      TRY(this->compileBaseType(std::get<BaseType>(type)));
      return Ok();
    } else if (std::holds_alternative<ListType>(type)) {
      TRY(this->compileListType(std::get<ListType>(type)));
      return Ok();
    }
    return Error("Unexpected Type with index {} when compiling.", type.index());
  }

  Result<None> compileBaseType(const BaseType& type) {
    if (type == BaseType::VOID) {
      this->out << "void";
      return Ok();
    } else if (type == BaseType::INT) {
      this->out << "int";
      return Ok();
    }
    return Error("Unexpected BaseType {} when compiling.",
                 baseTypeToString(type));
  }

  Result<None> compileListType(const ListType& listType) {
    this->out << "[";
    TRY(this->compileBaseType(listType.elementType));
    this->out << "]";
    return Ok();
  }

  Result<None> compileStatementBlock(const StatementBlock& node) {
    this->out << "{\n";
    this->indent += INDENT_SIZE;
    for (const auto& statement : node.statements) {
      for (size_t i = 0; i < this->indent; i++) {
        this->out << " ";
      }
      TRY(this->compileStatement(statement));
      this->out << ";\n";
    }
    this->indent -= INDENT_SIZE;
    this->out << "}";
    return Ok();
  }

  Result<None> compileStatement(const Statement& node) {
    if (std::holds_alternative<Unique<FunctionCall>>(node)) {
      TRY(this->compileFunctionCall(*std::get<Unique<FunctionCall>>(node)));
      return Ok();
    }
    return Error("Unexpected Statement of index {} when compiling.",
                 node.index());
  }

  Result<None> compileFunctionCall(const FunctionCall& node) {
    this->out << node.name << "(";
    for (size_t i = 0; i < node.args.size(); i++) {
      TRY(this->compileExpression(node.args[i]));
      if (i < node.args.size() - 1) {
        this->out << ", ";
      }
    }
    this->out << ")";
    return Ok();
  }

  Result<None> compileExpression(const Expression& node) {
    if (std::holds_alternative<Unique<FunctionCall>>(node)) {
      TRY(this->compileFunctionCall(*std::get<Unique<FunctionCall>>(node)));
      return Ok();
    } else if (std::holds_alternative<Unique<StringLiteral>>(node)) {
      this->compileStringLiteral(*std::get<Unique<StringLiteral>>(node));
      return Ok();
    }
    return Error("Unexpected Expression of index {} when compiling.",
                 node.index());
  }

  void compileStringLiteral(const StringLiteral& node) {
    this->out << node.value;
  }
};

#endif  // COMPILER_CC