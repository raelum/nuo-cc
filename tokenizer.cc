#ifndef TOKENIZER_CC
#define TOKENIZER_CC

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
  GENERATOR(FLOAT)                    \
  GENERATOR(NUMBER_LITERAL)           \
  GENERATOR(STRING_LITERAL)
enum class TokenType { FOREACH_TOKEN_TYPE(ENUM_GENERATOR) };
static const char* tokenTypeString[] = {FOREACH_TOKEN_TYPE(STRING_GENERATOR)};
StringView tokenTypeToString(TokenType type) {
  return tokenTypeString[static_cast<int>(type)];
}

// Output token from tokenizer.
struct Token {
  TokenType type;
  size_t start;
  size_t end;

  String toString(StringView source) {
    bool showText = false;
    if (this->type == TokenType::IDENTIFIER ||
        this->type == TokenType::NUMBER_LITERAL ||
        this->type == TokenType::STRING_LITERAL) {
      showText = true;
    }

    StringStream output;
    output << tokenTypeToString(this->type);
    if (showText) {
      output << " ";
      for (size_t i = this->start; i < this->end; i++) {
        output << source[i];
      }
    }
    return output.str();
  }
};

// Used when printing error messages.
struct Location {
  int line;
  int col;
};

struct Tokenizer {
  // Nuo code that is being tokenized.
  StringView code;
  // Index to the start of the current token.
  size_t start = 0;
  // Index to the end of the current token.
  size_t end = 0;
  // Number of open parenthesis we see so far.
  size_t openParenCount = 0;

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
      return Ok(this->makeToken(TokenType::NEWLINE));
    } else if (this->isAlpha(c)) {
      // Place end back to beginning of token, since makeIdentifierToken needs
      // to see all characters to decide if it is a keyword.
      this->end--;
      return Ok(this->makeIdentifierToken());
    } else if (this->isDigit(c)) {
      return this->makeNumberToken();
    } else if (c == '"') {
      return this->makeStringToken();
    } else if (c == '(') {
      this->openParenCount++;
      return Ok(this->makeToken(TokenType::LEFT_PAREN));
    } else if (c == ')') {
      this->openParenCount--;
      return Ok(this->makeToken(TokenType::RIGHT_PAREN));
    } else if (c == '{') {
      return Ok(this->makeToken(TokenType::LEFT_BRACE));
    } else if (c == '}') {
      return Ok(this->makeToken(TokenType::RIGHT_BRACE));
    } else if (c == '[') {
      return Ok(this->makeToken(TokenType::LEFT_BRACKET));
    } else if (c == ']') {
      return Ok(this->makeToken(TokenType::RIGHT_BRACKET));
    } else if (c == ',') {
      return Ok(this->makeToken(TokenType::COMMA));
    } else if (c == ':') {
      return Ok(this->makeToken(TokenType::COLON));
    } else if (c == '=') {
      if (this->peekChar() == '=') {
        this->consumeChar();
        return Ok(this->makeToken(TokenType::EQUAL_EQUAL));
      }
      return Ok(this->makeToken(TokenType::EQUAL));
    } else if (c == '!') {
      if (this->peekChar() == '=') {
        this->consumeChar();
        return Ok(this->makeToken(TokenType::BANG_EQUAL));
      }
      return Ok(this->makeToken(TokenType::BANG));
    } else if (c == '<') {
      if (this->peekChar() == '=') {
        this->consumeChar();
        return Ok(this->makeToken(TokenType::LESS_EQUAL));
      }
      return Ok(this->makeToken(TokenType::LESS));
    } else if (c == '>') {
      if (this->peekChar() == '=') {
        this->consumeChar();
        return Ok(this->makeToken(TokenType::GREATER_EQUAL));
      }
      return Ok(this->makeToken(TokenType::GREATER));
    } else if (c == '+') {
      if (this->peekChar() == '=') {
        this->consumeChar();
        return Ok(this->makeToken(TokenType::PLUS_EQUAL));
      }
      return Ok(this->makeToken(TokenType::PLUS));
    } else if (c == '-') {
      if (this->peekChar() == '=') {
        this->consumeChar();
        return Ok(this->makeToken(TokenType::MINUS_EQUAL));
      }
      return Ok(this->makeToken(TokenType::MINUS));
    }

    Location loc = this->getLocation();
    return Error("Ran into an unexpected character '{}' at line {} column {}",
                 c, loc.line, loc.col);
  }

  // Get the location of the given start position in the code. It's easier to do
  // this rather than keeping track of line and column as we tokenize,
  // especially if the only time we need location info is when outputting an
  // error message.
  Location getLocation(int start) {
    int lineNumber = 1;
    int lineStartIndex = 0;
    for (int i = 0; i <= start; i++) {
      if (this->code[i] == '\n') {
        lineNumber += 1;
        lineStartIndex = i + 1;
      }
    }
    return {.line = lineNumber, .col = start - lineStartIndex + 1};
  }

  // Get the location of the token currently being processed.
  Location getLocation() { return this->getLocation(this->start); }

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

  // Consumes the next character if it matches the given one and return true.
  bool matchChar(char c) {
    if (this->peekChar() == c) {
      this->consumeChar();
      return true;
    }
    return false;
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
      } else {
        break;
      }
    }
  }

  Token makeToken(TokenType type) {
    return Token{.type = type, .start = this->start, .end = this->end};
  }

  bool isIdentifierChar() {
    return this->isAlpha(this->peekChar()) || this->isDigit(this->peekChar());
  }

  Token makeIdentifierToken() {
    if (this->matchChar('e')) {
      // token: e
      if (this->matchChar('l')) {
        // token: el
        if (this->matchChar('i')) {
          // token: eli
          if (this->matchChar('f')) {
            // token: elif
            if (!this->isIdentifierChar()) {
              // token: elif<end>
              return this->makeToken(TokenType::ELIF);
            }
          }
        } else if (this->matchChar('s')) {
          // token: els
          if (this->matchChar('e')) {
            // token: else
            if (!this->isIdentifierChar()) {
              // token: else<end>
              return this->makeToken(TokenType::ELSE);
            }
          }
        }
      }
    } else if (this->matchChar('f')) {
      // token: f
      if (this->matchChar('l')) {
        // token: fl
        if (this->matchChar('o')) {
          // token: flo
          if (this->matchChar('a')) {
            // token: floa
            if (this->matchChar('t')) {
              // token: float
              if (!this->isIdentifierChar()) {
                // token: float<end>
                return this->makeToken(TokenType::FLOAT);
              }
            }
          }
        }
      } else if (this->matchChar('n')) {
        // token: fn
        if (!this->isIdentifierChar()) {
          // token: fn<end>
          return this->makeToken(TokenType::FN);
        }
      } else if (this->matchChar('o')) {
        // token: fo
        if (this->matchChar('r')) {
          // token: for
          if (!this->isIdentifierChar()) {
            // token: for<end>
            return this->makeToken(TokenType::FOR);
          }
        }
      }
    } else if (this->matchChar('i')) {
      // token: i
      if (this->matchChar('f')) {
        // token: if
        if (!this->isIdentifierChar()) {
          // token: if<end>
          return this->makeToken(TokenType::IF);
        }
      } else if (this->matchChar('n')) {
        // token: in
        if (this->matchChar('t')) {
          // token: int
          if (!this->isIdentifierChar()) {
            // token: int<end>
            return this->makeToken(TokenType::INT);
          }
        }
      }
    } else if (this->matchChar('r')) {
      // token: r
      if (this->matchChar('e')) {
        // token: re
        if (this->matchChar('t')) {
          // token: ret
          if (this->matchChar('u')) {
            // token: retu
            if (this->matchChar('r')) {
              // token: retur
              if (this->matchChar('n')) {
                // token: return
                if (!this->isIdentifierChar()) {
                  // token: return<end>
                  return this->makeToken(TokenType::RETURN);
                }
              }
            }
          }
        }
      }
    }
    // Since we didn't match any keywords, we have a user-defined identifier.
    // Consume all remaining alphabet or digit tokens.
    while (!this->isAtEnd() && this->isIdentifierChar()) {
      this->consumeChar();
    }
    return this->makeToken(TokenType::IDENTIFIER);
  }

  Result<Token> makeNumberToken() {
    // Consume the first part of the number.
    this->consumeNumberChars();
    // Consume the second part of the number after decimal place.
    if (this->peekChar() == '.') {
      this->consumeChar();
      if (!this->isDigit(this->peekChar())) {
        Location loc = this->getLocation();
        return Error(
            "Unexpected character '{}' after number decimal at line {} column "
            "{}",
            this->peekChar(), loc.line, this->end + 1);
      }
      consumeNumberChars();
    }
    return Ok(this->makeToken(TokenType::NUMBER_LITERAL));
  }

  void consumeNumberChars() {
    while (!this->isAtEnd() && this->isDigit(this->peekChar())) {
      this->consumeChar();
    }
  }

  Result<Token> makeStringToken() {
    // Consume characters until we reach the end of the string or the end of the
    // file.
    while (!this->isAtEnd() && this->peekChar() != '"') {
      this->consumeChar();
    }
    // Return string token only if we've truly reached the end of the string.
    if (!this->isAtEnd() && this->peekChar() == '"') {
      this->consumeChar();
      Token stringToken = this->makeToken(TokenType::STRING_LITERAL);
      return Ok(stringToken);
    }
    Location loc = this->getLocation();
    return Error("Unterminated string that started at line {} column {}.",
                 loc.line, loc.col);
  }
};

#endif  // TOKENIZER_CC