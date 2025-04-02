#ifndef NUO_CC
#define NUO_CC

/*
Run code:
clang++ -Wextra -Werror -std=c++20 nuo.cc -o build/nuo && ./build/nuo
*/
#include "ast.cc"
#include "ast_printer.cc"
#include "builtins.cc"
#include "file.cc"
#include "parser.cc"
#include "spec_test.cc"
#include "tokenizer.cc"

String getActualResultForTokenizerTest(const TestCase& testCase) {
  StringStream result;
  Tokenizer tokenizer(testCase.input);
  while (true) {
    Result<Token> token = tokenizer.next();
    if (!token.ok) {
      return token.error;
    }
    result << token.value.toString(testCase.input);
    if (token.value.type == TokenType::END) {
      break;
    }
    result << '\n';
  }
  return result.str();
}

String getActualResultForParserTest(const TestCase& testCase) {
  Parser parser(testCase.input);
  Result<Program> program = parser.parse();
  if (!program.ok) {
    return program.error;
  }
  // Generate String from AST.
  AstPrinter astPrinter;
  Result<String> programString = astPrinter.printProgram(program.value);
  if (!programString.ok) {
    return programString.error;
  }
  return programString.value;
}

int main() {
  Vector<SpecTest> tests = {
      SpecTest("tokenizer.test", getActualResultForTokenizerTest),
      SpecTest("parser.test", getActualResultForParserTest),
  };
  for (auto& test : tests) {
    Result<None> testResult = test.run();
    if (!testResult.ok) {
      print("ERROR: {}", testResult.error);
      break;
    }
  }
}

#endif  // NUO_CC