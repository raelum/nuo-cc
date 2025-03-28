#ifndef PARSER_CC
#define PARSER_CC

#include "builtins.cc"
#include "tokenizer.cc"

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

    for (int i = 0; i < this->args.size(); i++) {
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
    for (int i = 0; i < this->statements.size(); i++) {
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

    for (int i = 0; i < this->params.size(); i++) {
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
    for (int i = 0; i < this->functions.size(); i++) {
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

struct Parser {
  StringView code;
  Tokenizer tokenizer;
  Token currentToken;

  Parser(StringView code) : code(code), tokenizer(Tokenizer(code)) {}

  Result<Program> parse() {
    // Populate current token before parsing the program.
    TRY(this->currentToken, this->tokenizer.next());

    Vector<FunctionDeclaration> functions;
    while (!this->isToken(TokenType::END)) {
      // Consume any preceding or trailing newlines.
      if (this->isToken(TokenType::NEWLINE)) {
        TRY(this->consumeToken());
      } else if (this->isToken(TokenType::FN)) {
        TRY(FunctionDeclaration function, this->parseFunctionDeclaration());
        functions.push_back(std::move(function));
      }
    }

    return Ok(Program{.functions = std::move(functions)});
  }

  // Checks if the current token is of the given type.
  bool isToken(TokenType type) { return this->currentToken.type == type; }

  // Consume the current token, regardless of what type it is.
  Result<None> consumeToken() {
    TRY(this->currentToken, this->tokenizer.next());
    return Ok();
  }

  // Consumes the current token if it is of the given type, and advances to the
  // next token.
  Result<None> consumeToken(TokenType type) {
    TRY([[maybe_unused]] Token token, this->getToken(type));
    return Ok();
  }

  // Returns the current token if it is of the given type and advances to the
  // next one.
  Result<Token> getToken(TokenType type) {
    if (this->isToken(type)) {
      Token consumed = this->currentToken;
      TRY(this->currentToken, this->tokenizer.next());
      return Ok(consumed);
    } else {
      return Error("Expected {} but got {}.", tokenTypeToString(type),
                   this->getTokenType());
    }
  }

  // Get's the value of the current token only if it matches the given type.
  Result<StringView> getTokenValue(TokenType type) {
    TRY(Token token, this->getToken(type));
    return Ok(StringView(&this->code[token.start], token.end - token.start));
  }

  StringView getTokenType() {
    return tokenTypeToString(this->currentToken.type);
  }

  Location getLocation() {
    return this->tokenizer.getLocation(this->currentToken.start);
  }

  Result<FunctionDeclaration> parseFunctionDeclaration() {
    TRY(this->consumeToken(TokenType::FN));

    TRY(StringView name, this->getTokenValue(TokenType::IDENTIFIER));

    TRY(Vector<FunctionParameter> parameters, this->parseFunctionParameters());

    // Parse function return value, defaulting to void if there is none.
    Type returnType = BaseType::VOID;
    if (this->isToken(TokenType::COLON)) {
      TRY(this->consumeToken());
      TRY(returnType, this->parseType());
    }

    // Parse statement block, expecting a newline after it.
    TRY(StatementBlock body, parseStatementBlock());
    this->consumeToken(TokenType::NEWLINE);

    return Ok(FunctionDeclaration{.name = std::move(name),
                                  .params = std::move(parameters),
                                  .returnType = std::move(returnType),
                                  .body = std::move(body)});
  }

  Result<Vector<FunctionParameter>> parseFunctionParameters() {
    Vector<FunctionParameter> parameters;
    TRY(this->consumeToken(TokenType::LEFT_PAREN));

    // Return early if there are no parameters.
    if (this->isToken(TokenType::RIGHT_PAREN)) {
      TRY(this->consumeToken());
      return Ok(parameters);
    }

    // Parse function parameters.
    while (true) {
      TRY(StringView name, this->getTokenValue(TokenType::IDENTIFIER));
      TRY(this->consumeToken(TokenType::COLON));
      TRY(Type type, this->parseType());
      parameters.push_back(FunctionParameter{.name = name, .type = type});

      // Continue if there are more parameters.
      if (this->isToken(TokenType::COMMA)) {
        TRY(this->consumeToken());
        continue;
      }
      break;
    }

    // Consume closing parenthesis.
    this->consumeToken(TokenType::RIGHT_PAREN);
    return Ok(parameters);
  }

  Result<Type> parseType() {
    if (this->isToken(TokenType::INT)) {
      TRY(this->consumeToken(TokenType::INT));
      return Ok((Type)BaseType::INT);
    }
    return Error("Unexpected token {} when parsing type.",
                 this->getTokenType());
  }

  Result<StatementBlock> parseStatementBlock() {
    // Consume the opening brace.
    this->consumeToken(TokenType::LEFT_BRACE);

    Vector<Statement> statements;
    while (true) {
      // Consume any preceding, or trailing newlines.
      if (this->isToken(TokenType::NEWLINE)) {
        TRY(this->consumeToken());
      }
      // Exit the loop if we encounter the closing brace.
      else if (this->isToken(TokenType::RIGHT_BRACE)) {
        TRY(this->consumeToken());
        break;
      }
      // Parse statement.
      else {
        TRY(Statement statement, this->parseStatement());
        statements.push_back(std::move(statement));
      }
    }

    return Ok(StatementBlock{.statements = std::move(statements)});
  }

  Result<Statement> parseStatement() {
    // Parse identifier statement, expecting a newline after it.
    if (this->isToken(TokenType::IDENTIFIER)) {
      TRY(Statement statement, this->parseIdentifierStatement());
      TRY(this->consumeToken(TokenType::NEWLINE));
      return Ok(std::move(statement));
    }
    // Fail for every other token type.
    Location loc = this->getLocation();
    return Error("Unexpected token {} at {}:{} when parsing statement block.",
                 this->getTokenType(), loc.line, loc.col);
  }

  Result<Statement> parseIdentifierStatement() {
    TRY(StringView name, this->getTokenValue(TokenType::IDENTIFIER));

    // Parse function call statement, ensuring we see a newline after.
    if (this->isToken(TokenType::LEFT_PAREN)) {
      TRY(Vector<Expression> args, this->parseFunctionCallArguments());
      return Ok(FunctionCall::makeStatement(std::move(name), std::move(args)));
    }

    Location loc = this->getLocation();
    return Error(
        "Unexpected token {} at {}:{} when parsing identifier statement.",
        this->getTokenType(), loc.line, loc.col);
  }

  Result<Vector<Expression>> parseFunctionCallArguments() {
    Vector<Expression> args;

    // Consume opening parenthesis.
    TRY(this->consumeToken(TokenType::LEFT_PAREN));

    // Return early if there are no arguments.
    if (this->isToken(TokenType::RIGHT_PAREN)) {
      TRY(this->consumeToken());
      return Ok(std::move(args));
    }

    // Parse arguments.
    while (true) {
      TRY(Expression expr, this->parseExpression());
      args.push_back(std::move(expr));

      // Continue parsing more arguments if there is a comma.
      if (this->isToken(TokenType::COMMA)) {
        TRY(this->consumeToken());
      }
      break;
    }

    // Consume closing parenthesis.
    TRY(this->consumeToken(TokenType::RIGHT_PAREN));

    return Ok(std::move(args));
  }

  Result<Expression> parseExpression() {
    Expression left;
    // Parse the first expression which could potentially be the left side of a
    // binary operation.
    if (this->isToken(TokenType::IDENTIFIER)) {
      TRY(left, this->parseIdentifierExpression());
    } else if (this->isToken(TokenType::STRING)) {
      // TODO: Remove need for specifying token type in this case.
      TRY(StringView value, this->getTokenValue(TokenType::STRING));
      return Ok(StringLiteral::make(std::move(value)));
    } else {
      Location loc = this->getLocation();
      return Error(
          "Unexpected token {} at {}:{} when parsing identifier expression.",
          this->getTokenType(), loc.line, loc.col);
    }

    // TODO: Parse right side of binary operation.
    return Ok(std::move(left));
  }

  Result<Expression> parseIdentifierExpression() {
    TRY(StringView name, this->getTokenValue(TokenType::IDENTIFIER));

    // Parse function call statement, ensuring we see a newline after.
    if (this->isToken(TokenType::LEFT_PAREN)) {
      TRY(Vector<Expression> args, this->parseFunctionCallArguments());
      return Ok(FunctionCall::makeExpression(std::move(name), std::move(args)));
    }

    // Otherwise, we just have a variable reference.
    return Ok(VariableReference::make(std::move(name)));
  }
};

#endif  // PARSER_CC