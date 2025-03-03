#include "builtins.cc"

// Token Type enum and their string names for debugging.
#define FOREACH_TOKEN_TYPE(GENERATOR) \
  GENERATOR(END)                      \
  GENERATOR(NEWLINE)                  \
  GENERATOR(LEFT_PAREN)               \
  GENERATOR(RIGHT_PAREN)              \
  GENERATOR(LEFT_BRACE)               \
  GENERATOR(RIGHT_BRACE)              \
  GENERATOR(LEFT_BRACKET)             \
  GENERATOR(RIGHT_BRACKET)            \
  GENERATOR(COLON)                    \
  GENERATOR(COMMA)                    \
  GENERATOR(PLUS)                     \
  GENERATOR(PLUS_EQUAL)               \
  GENERATOR(MINUS)                    \
  GENERATOR(MINUS_EQUAL)              \
  GENERATOR(BANG)                     \
  GENERATOR(BANG_EQUAL)               \
  GENERATOR(EQUAL)                    \
  GENERATOR(EQUAL_EQUAL)              \
  GENERATOR(GREATER)                  \
  GENERATOR(GREATER_EQUAL)            \
  GENERATOR(LESS)                     \
  GENERATOR(LESS_EQUAL)               \
  GENERATOR(FN)                       \
  GENERATOR(RETURN)                   \
  GENERATOR(IF)                       \
  GENERATOR(ELIF)                     \
  GENERATOR(ELSE)                     \
  GENERATOR(FOR)                      \
  GENERATOR(IDENTIFIER)               \
  GENERATOR(INT)                      \
  GENERATOR(NUMBER)                   \
  GENERATOR(STRING)
enum class TokenType { FOREACH_TOKEN_TYPE(ENUM_GENERATOR) };
static const char* tokenTypeString[] = {FOREACH_TOKEN_TYPE(STRING_GENERATOR)};
String tokenTypeToString(TokenType type) {
  return tokenTypeString[static_cast<int>(type)];
}

struct Token {
  TokenType type;
  StringView value;
  int line;

  String toString() { return tokenTypeToString(this->type); }
};

struct Tokenizer {
  // Nuo code that is being tokenized.
  StringView code;
  // Index to the start of the current token.
  int start = 0;
  // Index to the end of the current token.
  int end = 0;
  // Number of open parenthesis we see so far.
  int openParenCount = 0;
  // Line number we are currently on.
  int line = 1;

  Tokenizer(StringView code) : code(code) {}

  Result<Token> next() {
    this->skipWhitespace();

    // Move start of token to end of whitespace.
    this->start = this->end;

    // Return early if we reached the end of the file.
    if (this->isAtEnd()) {
      return Ok(this->makeToken(TokenType::END));
    }

    char c = this->getChar();
    if (c == '\n') {
      this->line++;
      return Ok(this->makeToken(TokenType::NEWLINE));
    } else if (this->isAlpha(c)) {
      return Ok(this->makeIdentifierToken());
    } else if (this->isDigit(c)) {
      return this->makeNumberToken();
    } else if (c == '"') {
      return this->makeStringToken();
    } else if (c == '(') {
      return Ok(this->makeToken(TokenType::LEFT_PAREN));
    } else if (c == ')') {
      return Ok(this->makeToken(TokenType::RIGHT_PAREN));
    } else if (c == '{') {
      return Ok(this->makeToken(TokenType::LEFT_BRACE));
    } else if (c == '}') {
      return Ok(this->makeToken(TokenType::RIGHT_BRACE));
    } else if (c == ',') {
      return Ok(this->makeToken(TokenType::COMMA));
    } else if (c == ':') {
      return Ok(this->makeToken(TokenType::COLON));
    } else if (c == '=') {
      return Ok(this->makeToken(TokenType::EQUAL));
    } else if (c == '<') {
      return Ok(this->makeToken(TokenType::LESS));
    } else if (c == '>') {
      return Ok(this->makeToken(TokenType::GREATER));
    } else if (c == '+') {
      return Ok(this->makeToken(TokenType::PLUS));
    } else if (c == '-') {
      return Ok(this->makeToken(TokenType::MINUS));
    }

    return Error("Ran into an unexpected character '{}'", c);
  }

  // Peeks at the next character without consuming it.
  char peekChar() { return this->code[this->end]; }

  // Consumes the next character.
  void consumeChar() { this->end++; }

  // Consumes the next character and returns it.
  char getChar() {
    int currentEnd = this->end;
    this->end++;
    return this->code[currentEnd];
  }

  // Whether or not we've reached the end of the code.
  bool isAtEnd() { return this->end >= this->code.length(); }

  bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }

  bool isDigit(char c) { return c >= '0' && c <= '9'; }

  void skipWhitespace() {
    while (!this->isAtEnd()) {
      char c = this->peekChar();
      // Consume all whitespace characters.
      if (c == ' ' || c == '\t' || c == '\r') {
        this->consumeChar();
      }
      // Consume any newline characters if we are within a parenthesis.
      else if (c == '\n' && this->openParenCount > 0) {
        this->consumeChar();
        this->line++;
      } else {
        break;
      }
    }
  }

  Token makeToken(TokenType type) {
    return Token{
        .type = type, .value = this->makeTokenValue(), .line = this->line};
  }

  StringView makeTokenValue() {
    return StringView(&this->code[this->start], this->end - this->start);
  }

  Token makeIdentifierToken() {
    // Consume all alphabet or digit tokens. The first character was already
    // verified to be an alphabet before this function was called.
    while (!this->isAtEnd() && (this->isAlpha(this->peekChar()) ||
                                this->isDigit(this->peekChar()))) {
      this->consumeChar();
    }
    return this->makeToken(this->getIdentifierTokenType());
  }

  TokenType getIdentifierTokenType() {
    StringView value = this->makeTokenValue();
    if (value == "fn") {
      return TokenType::FN;
    } else if (value == "if") {
      return TokenType::IF;
    } else if (value == "elif") {
      return TokenType::ELIF;
    } else if (value == "else") {
      return TokenType::ELSE;
    } else if (value == "for") {
      return TokenType::FOR;
    } else if (value == "return") {
      return TokenType::RETURN;
    } else if (value == "int") {
      return TokenType::INT;
    } else {
      return TokenType::IDENTIFIER;
    }
  }

  Result<Token> makeNumberToken() {
    // Consume the first part of the number.
    this->consumeNumberChars();
    // Consume the second part of the number after decimal place.
    if (this->peekChar() == '.') {
      this->consumeChar();
      if (!this->isDigit(this->peekChar())) {
        Error("Unexpected character after decimal . in number: '{}'",
              this->peekChar());
      }
      consumeNumberChars();
    }
    return Ok(this->makeToken(TokenType::NUMBER));
  }

  void consumeNumberChars() {
    while (!this->isAtEnd() && this->isDigit(this->peekChar())) {
      this->consumeChar();
    }
  }

  Result<Token> makeStringToken() {
    int startLine = this->line;
    // Consume characters until we reach the end of the string or the end of the
    // file.
    while (!this->isAtEnd() && this->peekChar() != '"') {
      // We allow multi-line strings so increment line number here.
      if (this->peekChar() == '\n') {
        this->line++;
      }
      this->consumeChar();
    }
    // Return string token only if we've truly reached the end of the string.
    if (!this->isAtEnd() && this->peekChar() == '"') {
      this->consumeChar();
      return Ok(this->makeToken(TokenType::STRING));
    }
    return Error("Unterminated string that started on line {}.", startLine);
  }
};