#ifndef AST_CC
#define AST_CC

#include "builtins.cc"

// Base Type enum and their string names for debugging.
#define FOREACH_BASE_TYPE(GENERATOR) \
  GENERATOR(VOID)                    \
  GENERATOR(INT)
enum class BaseType { FOREACH_BASE_TYPE(ENUM_GENERATOR) };
static const char* baseTypeString[] = {FOREACH_BASE_TYPE(STRING_GENERATOR)};
String baseTypeToString(BaseType type) {
  return baseTypeString[static_cast<int>(type)];
}

struct ListType {
  BaseType elementType;

  String toString() { return "[" + baseTypeToString(elementType) + "]"; }
};

using Type = Variant<BaseType, ListType>;
String typeToString(Type type) {
  if (std::holds_alternative<BaseType>(type)) {
    return baseTypeToString(std::get<BaseType>(type));
  } else if (std::holds_alternative<ListType>(type)) {
    return std::get<ListType>(type).toString();
  }
  return "Unexpected Type when converting to string.";
}

void indent(StringStream& buffer, int level) {
  for (int i = 0; i < level; i++) {
    buffer << "  ";
  }
}

// Forward declare Ast Nodes that are used in Statement or Expression variants.
struct VariableDeclaration;
struct VariableReference;
struct FunctionCall;
struct StringLiteral;

using Statement = Variant<Unique<VariableDeclaration>, Unique<FunctionCall>>;
String statementToString(Statement& statement, int level);

using Expression = Variant<Unique<VariableReference>, Unique<FunctionCall>,
                           Unique<StringLiteral>>;
String expressionToString(Expression& expression, int level);

struct FunctionParameter {
  StringView name;
  Type type;
};

struct VariableDeclaration {
  StringView name;
  Type type;
  Expression expression;
};

struct VariableReference {
  StringView name;

  static Expression make(StringView name) {
    return Unique<VariableReference>(
        new VariableReference{.name = std::move(name)});
  }
};

struct FunctionCall {
  StringView name;
  Vector<Expression> args;

  static Statement makeStatement(StringView name, Vector<Expression> args) {
    return Unique<FunctionCall>(
        new FunctionCall{.name = std::move(name), .args = std::move(args)});
  }

  static Expression makeExpression(StringView name, Vector<Expression> args) {
    return Unique<FunctionCall>(
        new FunctionCall{.name = std::move(name), .args = std::move(args)});
  }

  String toString(int level) {
    StringStream buffer;

    indent(buffer, level);
    buffer << "FunctionCall: " << this->name << "\n";

    for (size_t i = 0; i < this->args.size(); i++) {
      buffer << expressionToString(this->args[i], level + 1);
    }
    return buffer.str();
  }
};

struct StringLiteral {
  StringView value;

  static Expression make(StringView value) {
    return Unique<StringLiteral>(new StringLiteral{.value = std::move(value)});
  }

  String toString(int level) {
    StringStream buffer;
    indent(buffer, level);
    buffer << this->value;
    return buffer.str();
  }
};

struct StatementBlock {
  Vector<Statement> statements;

  String toString(int level) {
    StringStream buffer;
    for (size_t i = 0; i < this->statements.size(); i++) {
      buffer << statementToString(this->statements[i], level);
    }
    return buffer.str();
  }
};

struct FunctionDeclaration {
  StringView name;
  Vector<FunctionParameter> params;
  Type returnType;
  StatementBlock body;

  String toString(int level) {
    StringStream buffer;

    indent(buffer, level);
    buffer << "FunctionDeclaration: " << this->name << "\n";

    indent(buffer, level + 1);
    buffer << "params:\n";

    for (size_t i = 0; i < this->params.size(); i++) {
      indent(buffer, level + 2);
      buffer << this->params[i].name << ": "
             << typeToString(this->params[i].type) << "\n";
    }

    indent(buffer, level + 1);
    buffer << "returnType: " << typeToString(returnType) << "\n";

    indent(buffer, level + 1);
    buffer << "body:\n";
    buffer << this->body.toString(level + 2);

    return buffer.str();
  }
};

struct Program {
  Vector<FunctionDeclaration> functions;

  String toString() {
    StringStream buffer;
    for (size_t i = 0; i < this->functions.size(); i++) {
      buffer << this->functions[i].toString(0);
      if (i < this->functions.size() - 1) {
        buffer << "\n";
      }
    }
    return buffer.str();
  }
};

String statementToString(Statement& statement, int level) {
  if (std::holds_alternative<Unique<FunctionCall>>(statement)) {
    return std::get<Unique<FunctionCall>>(statement)->toString(level);
  }
  return "Unexpected Statement when converting to String.";
}

String expressionToString(Expression& expression, int level) {
  if (std::holds_alternative<Unique<FunctionCall>>(expression)) {
    return std::get<Unique<FunctionCall>>(expression)->toString(level);
  } else if (std::holds_alternative<Unique<StringLiteral>>(expression)) {
    return std::get<Unique<StringLiteral>>(expression)->toString(level);
  }
  return "Unexpected Expression when converting to String.";
}

#endif  // AST_CC