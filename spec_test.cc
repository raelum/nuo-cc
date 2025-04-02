#ifndef SPEC_TEST_CC
#define SPEC_TEST_CC

#include "builtins.cc"
#include "file.cc"

struct TestCase {
  StringView description;
  StringView input;
  StringView result;
};

struct SpecTest {
  StringView testFileName;
  String (*getActualResult)(const TestCase& testCase);

  SpecTest(StringView testFileName,
           String (*getActualResult)(const TestCase& testCase))
      : testFileName(testFileName), getActualResult(getActualResult) {}

  Result<bool> run() {
    TRY(String testFile, readFile(testFileName));
    Vector<StringView> tests = this->getTests(testFile);
    TRY(Vector<TestCase> testCases, this->getTestCases(tests));
    Vector<String> actualResults = this->getActualResults(testCases);

    // Write updated spec tests.
    String actualSpecTests = this->generateSpecTests(testCases, actualResults);
    TRY(writeFile(String("build/") + String(this->testFileName),
                  actualSpecTests));

    // Check if the actual results match expected ones.
    bool testsPassed = true;
    for (size_t i = 0; i < testCases.size(); i++) {
      if (testCases[i].result != actualResults[i]) {
        testsPassed = false;
        break;
      }
    }
    return Ok(testsPassed);
  }

  int getLineEnd(StringView text, size_t lineStart) {
    // Exit early if we are end of file or already see a newline.
    if (lineStart >= text.length() || text[lineStart] == '\n') {
      return lineStart;
    }
    // Find the next newline.
    size_t lineEnd = lineStart + 1;
    while (lineEnd < text.length() && text[lineEnd] != '\n') {
      lineEnd++;
    }
    return lineEnd;
  }

  bool isCharacterLine(StringView text, size_t lineStart, size_t lineEnd,
                       char c) {
    // Verify line length.
    if (lineEnd - lineStart != 4) {
      return false;
    }
    // Verify all characters are dashes.
    for (size_t i = lineStart; i < lineEnd; i++) {
      if (text[i] != c) {
        return false;
      }
    }
    return true;
  }

  bool isTickLine(StringView text, size_t lineStart, size_t lineEnd) {
    return this->isCharacterLine(text, lineStart, lineEnd, '`');
  }

  bool isDashLine(StringView text, size_t lineStart, size_t lineEnd) {
    return this->isCharacterLine(text, lineStart, lineEnd, '-');
  }

  bool isEqualLine(StringView text, size_t lineStart, size_t lineEnd) {
    return this->isCharacterLine(text, lineStart, lineEnd, '=');
  }

  Result<Vector<TestCase>> getTestCases(Vector<StringView>& tests) {
    Vector<TestCase> testCases;
    for (const auto& t : tests) {
      TRY(TestCase testCase, this->getTestCase(t));
      testCases.push_back(testCase);
    }
    return Ok(testCases);
  }

  Vector<StringView> getTests(String& specTestFile) {
    Vector<StringView> tests;
    size_t testStart = 0;
    size_t lineStart = 0;
    while (lineStart < specTestFile.length()) {
      // Find the end of the current line.
      size_t lineEnd = this->getLineEnd(specTestFile, lineStart);

      // Add test when finding a test break line. Skip equal line right at the
      // beginning of the file.
      if (this->isEqualLine(specTestFile, lineStart, lineEnd) &&
          testStart != lineStart) {
        int testSize = lineStart - testStart - 1;
        tests.push_back(StringView(&specTestFile[testStart], testSize));
        testStart = lineEnd + 1;
      }
      // Add final test at end of file that isn't followed by a test break line.
      else if (lineEnd + 1 >= specTestFile.length()) {
        size_t testSize = lineEnd - testStart;
        tests.push_back(StringView(&specTestFile[testStart], testSize));
      }

      // Move to the next line.
      lineStart = lineEnd + 1;
    }
    return tests;
  }

  Result<TestCase> getTestCase(StringView test) {
    size_t lineStart = 0;
    size_t lineEnd = 0;

    // Consume any preceding newlines.
    while (test[lineStart] == '\n') {
      lineStart += 1;
    }

    // Ensure we find the description block opener.
    lineEnd = this->getLineEnd(test, lineStart);
    if (!this->isTickLine(test, lineStart, lineEnd)) {
      return Error("Didn't find beginning of description in test:\n{}", test);
    }

    // Find end of description block.
    size_t descriptionStart = lineEnd + 1;
    lineStart = descriptionStart;
    while (lineStart < test.length()) {
      lineEnd = this->getLineEnd(test, lineStart);
      if (this->isTickLine(test, lineStart, lineEnd)) {
        break;
      }
      lineStart = lineEnd + 1;
    }

    // Ensure we found end of description.
    if (lineStart >= test.length()) {
      return Error("Didn't find end of description in test:\n{}", test);
    }

    size_t descriptionSize = lineStart - descriptionStart - 1;

    // Find the end of test input.
    size_t inputStart = lineEnd + 1;
    lineStart = inputStart;
    while (lineStart < test.length()) {
      lineEnd = this->getLineEnd(test, lineStart);
      if (this->isDashLine(test, lineStart, lineEnd)) {
        break;
      }
      lineStart = lineEnd + 1;
    }

    // Ensure we found end of input.
    if (lineStart >= test.length()) {
      return Error("Didn't find end of input in test:\n{}", test);
    }

    int inputSize = lineStart - inputStart - 1;
    int resultStart = lineEnd == test.length() ? lineEnd : lineEnd + 1;
    int resultSize = test.length() - resultStart;
    return Ok(TestCase{
        .description = StringView(&test[descriptionStart], descriptionSize),
        .input = StringView(&test[inputStart], inputSize),
        .result = StringView(&test[resultStart], resultSize)});
  }

  Vector<String> getActualResults(Vector<TestCase>& testCases) {
    Vector<String> results;
    for (const auto& testCase : testCases) {
      results.push_back(this->getActualResult(testCase));
    }
    return results;
  }

  String generateSpecTests(Vector<TestCase>& testCases,
                           Vector<String>& actualResults) {
    StringStream specTests;
    for (size_t i = 0; i < testCases.size(); i++) {
      specTests << "````\n";
      specTests << testCases[i].description << '\n';
      specTests << "````\n";
      specTests << testCases[i].input << '\n';
      specTests << "----\n";
      specTests << actualResults[i] << '\n';
      specTests << "====";

      // Only add new lines if this isn't the last test.
      if (i < testCases.size() - 1) {
        specTests << "\n\n";
      }
    }
    return specTests.str();
  }
};

#endif  // SPEC_TEST_CC