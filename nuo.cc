#ifndef NUO_CC
#define NUO_CC

// Run code: clang++ -std=c++20 nuo.cc -o build/nuo && ./build/nuo
#include "builtins.cc"
#include "file.cc"

int getLineEnd(StringView text, int lineStart) {
  // Exit early if we are end of file or already see a newline.
  if (lineStart >= text.length() || text[lineStart] == '\n') {
    return lineStart;
  }
  // Find the next newline.
  int lineEnd = lineStart + 1;
  while (lineEnd < text.length() && text[lineEnd] != '\n') {
    lineEnd++;
  }
  return lineEnd;
}

bool isCharacterLine(StringView text, int lineStart, int lineEnd, char c) {
  // Verify line length.
  if (lineEnd - lineStart != 4) {
    return false;
  }
  // Verify all characters are dashes.
  for (int i = lineStart; i < lineEnd; i++) {
    if (text[i] != c) {
      return false;
    }
  }
  return true;
}

bool isDashLine(StringView text, int lineStart, int lineEnd) {
  return isCharacterLine(text, lineStart, lineEnd, '-');
}

bool IsEqualLine(StringView text, int lineStart, int lineEnd) {
  return isCharacterLine(text, lineStart, lineEnd, '=');
}

Vector<StringView> getTests(String& specTestFile) {
  Vector<StringView> tests;
  int testStart = 0;
  int lineStart = 0;
  while (lineStart < specTestFile.length()) {
    // Find the end of the current line.
    int lineEnd = getLineEnd(specTestFile, lineStart);

    // Add test when finding a test break line. Skip equal line right at the
    // beginning of the file.
    if (IsEqualLine(specTestFile, lineStart, lineEnd) &&
        testStart != lineStart) {
      int testSize = lineStart - testStart - 1;
      tests.push_back(StringView(&specTestFile[testStart], testSize));
      testStart = lineEnd + 1;
    }
    // Add final test at end of file that isn't followed by a test break line.
    else if (lineEnd + 1 >= specTestFile.length()) {
      int testSize = lineEnd - testStart;
      tests.push_back(StringView(&specTestFile[testStart], testSize));
    }

    // Move to the next line.
    lineStart = lineEnd + 1;
  }
  return tests;
}

struct TestCase {
  StringView input;
  StringView result;
};

TestCase getTestCase(StringView test) {
  int lineStart = 0;
  int lineEnd = 0;
  while (lineStart < test.length()) {
    // Find the end of the current line.
    lineEnd = getLineEnd(test, lineStart);

    // Find the dash line in this test.
    if (isDashLine(test, lineStart, lineEnd)) {
      break;
    }

    // Move to the next line.
    lineStart = lineEnd + 1;
  }

  int inputStart = 0;
  int inputSize = lineStart == 0 ? 0 : lineStart - inputStart - 1;
  int resultStart = lineEnd == test.length() ? lineEnd : lineEnd + 1;
  int resultSize = test.length() - resultStart;
  return TestCase{.input = StringView(&test[inputStart], inputSize),
                  .result = StringView(&test[resultStart], resultSize)};
}

Vector<TestCase> getTestCases(Vector<StringView>& tests) {
  Vector<TestCase> testCases;
  for (const auto& t : tests) {
    testCases.push_back(getTestCase(t));
  }
  return testCases;
}

int main() {
  String tokenizerTestFile = readFile("tokenizer.test");
  Vector<StringView> tokenizerTests = getTests(tokenizerTestFile);
  Vector<TestCase> tokenizerTestCases = getTestCases(tokenizerTests);
  for (const auto& t : tokenizerTestCases) {
    print("TEST INPUT:");
    print(t.input);
    print("TEST RESULT:");
    print(t.result);
  }
}

#endif  // NUO_CC