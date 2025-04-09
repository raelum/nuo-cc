#ifndef AST_CC
#define AST_CC

#include "builtins.cc"

// Base Type enum and their string names for debugging.
#define FOREACH_BASE_TYPE(GENERATOR) \
  GENERATOR(VOID)                    \
  GENERATOR(INT)                     \
  GENERATOR(FLOAT)
enum class BaseType { FOREACH_BASE_TYPE(ENUM_GENERATOR) };
static const char* baseTypeString[] = {FOREACH_BASE_TYPE(STRING_GENERATOR)};
String baseTypeToString(BaseType type) {
  return baseTypeString[static_cast<int>(type)];
}

struct ListType {
  BaseType elementType;
};

struct Type {
  Variant<BaseType, ListType> typeVariant;

  bool isBaseType() const {
    return std::holds_alternative<BaseType>(this->typeVariant);
  }

  BaseType getBaseType() const { return std::get<BaseType>(this->typeVariant); }

  bool isListType() const {
    return std::holds_alternative<ListType>(this->typeVariant);
  }

  ListType getListType() const { return std::get<ListType>(this->typeVariant); }

  bool equals(BaseType baseType) {
    return this->isBaseType() && this->getBaseType() == baseType;
  }

  bool equals(ListType listType) {
    return this->isListType() &&
           this->getListType().elementType == listType.elementType;
  }
};

// Forward declare Ast Nodes that are used in Statement or Expression variants.
struct VariableDeclaration;
struct VariableReference;
struct FunctionCall;
struct NumberLiteral;
struct StringLiteral;
struct Return;

using Statement =
    Variant<Unique<VariableDeclaration>, Unique<FunctionCall>, Unique<Return>>;

using Expression = Variant<Unique<VariableReference>, Unique<FunctionCall>,
                           Unique<StringLiteral>, Unique<NumberLiteral>>;

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

struct NumberLiteral {
  StringView value;

  static Expression make(StringView value) {
    return Unique<NumberLiteral>(new NumberLiteral{.value = std::move(value)});
  }
};

struct StringLiteral {
  StringView value;

  static Expression make(StringView value) {
    return Unique<StringLiteral>(new StringLiteral{.value = std::move(value)});
  }
};

struct Return {
  Optional<Expression> expression;

  static Statement makeStatement(Optional<Expression> expression) {
    return Unique<Return>(new Return{.expression = std::move(expression)});
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
  Vector<String> includes;
  Vector<FunctionDeclaration> functions;
};

#endif  // AST_CC