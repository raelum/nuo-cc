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

Result<String> getActualResultForTokenizerTest(const TestCase& testCase) {
  StringStream result;
  Tokenizer tokenizer(testCase.input);
  while (true) {
    TRY(Token token, tokenizer.next());
    result << token.toString(testCase.input);
    if (token.type == TokenType::END) {
      break;
    }
    result << '\n';
  }
  return Ok(result.str());
}

Result<String> getActualResultForParserTest(const TestCase& testCase) {
  // Parse code.
  Parser parser(testCase.input);
  TRY(Program program, parser.parse());
  // Generate String from AST.
  AstPrinter astPrinter;
  TRY(String astString, astPrinter.printProgram(program));
  return Ok(astString);
}

Result<String> getActualResultForCompilerTest(const TestCase& testCase) {
  // Parse code.
  Parser parser(testCase.input);
  TRY(Program program, parser.parse());
  // Analyze code.
  Analyzer analyzer;
  TRY(analyzer.analyzeProgram(program));
  // Compile code.
  Compiler compiler;
  TRY(String compiledProgram, compiler.compileProgram(program));
  return Ok(compiledProgram);
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