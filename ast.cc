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
};

using Type = Variant<BaseType, ListType>;

// Forward declare Ast Nodes that are used in Statement or Expression variants.
struct VariableDeclaration;
struct VariableReference;
struct FunctionCall;
struct StringLiteral;

using Statement = Variant<Unique<VariableDeclaration>, Unique<FunctionCall>>;

using Expression = Variant<Unique<VariableReference>, Unique<FunctionCall>,
                           Unique<StringLiteral>>;

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
};

struct StringLiteral {
  StringView value;

  static Expression make(StringView value) {
    return Unique<StringLiteral>(new StringLiteral{.value = std::move(value)});
  }
};

struct StatementBlock {
  Vector<Statement> statements;
};

struct FunctionDeclaration {
  StringView name;
  Vector<FunctionParameter> params;
  Type returnType;
  StatementBlock body;
};

struct Program {
  Vector<FunctionDeclaration> functions;
};

#endif  // AST_CC