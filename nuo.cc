#ifndef NUO_CC
#define NUO_CC

/*
Run code:
clang++ -Wextra -Werror -std=c++20 nuo.cc -o build/nuo && ./build/nuo
*/
#include "analyzer.cc"
#include "ast.cc"
#include "ast_printer.cc"
#include "builtins.cc"
#include "compiler.cc"
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
  // Parse code.
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

String getActualResultForCompilerTest(const TestCase& testCase) {
  // Parse code.
  Parser parser(testCase.input);
  Result<Program> parseResult = parser.parse();
  if (!parseResult.ok) {
    return parseResult.error;
  }
  // Analyze code.
  Analyzer analyzer;
  Result<None> analyzerResult = analyzer.analyzeProgram(parseResult.value);
  if (!analyzerResult.ok) {
    return analyzerResult.error;
  }
  // Compile code.
  Compiler compiler;
  Result<String> compilerResult = compiler.compileProgram(parseResult.value);
  if (!compilerResult.ok) {
    return compilerResult.error;
  }
  return compilerResult.value;
}

struct FailedTest {
  StringView testFileName;
  Optional<String> error;
};

int main() {
  Vector<SpecTest> tests = {
      SpecTest("tokenizer.test", getActualResultForTokenizerTest),
      SpecTest("parser.test", getActualResultForParserTest),
      SpecTest("compiler.test", getActualResultForCompilerTest),
  };

  Vector<FailedTest> failedTests;
  for (auto& test : tests) {
    Result<bool> testResult = test.run();
    if (!testResult.ok) {
      failedTests.push_back((FailedTest){.testFileName = test.testFileName,
                                         .error = testResult.error});
    } else if (!testResult.value) {
      failedTests.push_back((FailedTest){.testFileName = test.testFileName,
                                         .error = std::nullopt});
    }
  }

  if (failedTests.empty()) {
    print("All tests passed!");
  } else {
    print("The following tests failed:");
    for (const auto& failedTest : failedTests) {
      if (failedTest.error.has_value()) {
        print("{} had an unexpected error:\n{}", failedTest.testFileName,
              failedTest.error.value());
      } else {
        print("{}", failedTest.testFileName);
      }
    }
  }
}

#endif  // NUO_CC