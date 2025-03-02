// Run code: clang++ -std=c++20 nuo.cc -o build/nuo && ./build/nuo
#include <iostream>
#include <string>

#include "file.cc"

int get_line_end(string& text, int line_start) {
  // Exit early if we are end of file or already see a newline.
  if (line_start >= text.length() || text[line_start] == '\n') {
    return line_start;
  }
  // Find the next newline.
  int line_end = line_start + 1;
  while (line_end < text.length() && text[line_end] != '\n') {
    line_end++;
  }
  return line_end;
}

bool is_character_line(string& text, int line_start, int line_end, char c) {
  // Verify line length.
  if (line_end - line_start != 4) {
    return false;
  }
  // Verify all characters are dashes.
  for (int i = line_start; i < line_end; i++) {
    if (text[i] != c) {
      return false;
    }
  }
  return true;
}

bool is_dash_line(string& text, int line_start, int line_end) {
  return is_character_line(text, line_start, line_end, '-');
}

bool is_equal_line(string& text, int line_start, int line_end) {
  return is_character_line(text, line_start, line_end, '=');
}

vector<string> get_tests(string& spec_tests) {
  vector<string> tests;
  int test_start = 0;
  int line_start = 0;
  while (line_start < spec_tests.length()) {
    // Find the end of the current line.
    int line_end = get_line_end(spec_tests, line_start);

    // Add test when finding a test break line. Skip equal line right at the
    // beginning of the file.
    if (is_equal_line(spec_tests, line_start, line_end) &&
        test_start != line_start) {
      int test_size = line_start - test_start - 1;
      tests.push_back(spec_tests.substr(test_start, test_size));
      test_start = line_end + 1;
    }
    // Add final test at end of file that isn't followed by a test break line.
    else if (line_end + 1 >= spec_tests.length()) {
      int test_size = line_end - test_start;
      tests.push_back(spec_tests.substr(test_start, test_size));
    }

    // Move to the next line.
    line_start = line_end + 1;
  }
  return tests;
}

struct TestCase {
  string input;
  string result;
};

TestCase get_test_case(string test) {
  int line_start = 0;
  int line_end = 0;
  while (line_start < test.length()) {
    // Find the end of the current line.
    line_end = get_line_end(test, line_start);

    // Find the dash line in this test.
    if (is_dash_line(test, line_start, line_end)) {
      break;
    }

    // Move to the next line.
    line_start = line_end + 1;
  }

  int input_start = 0;
  int input_size = line_start == 0 ? 0 : line_start - input_start - 1;
  int result_start = line_end == test.length() ? line_end : line_end + 1;
  int result_size = test.length() - result_start;
  return TestCase{.input = test.substr(input_start, input_size),
                  .result = test.substr(result_start, result_size)};
}

vector<TestCase> get_test_cases(vector<string>& tests) {
  vector<TestCase> test_cases;
  for (const auto& t : tests) {
    test_cases.push_back(get_test_case(t));
  }
  return test_cases;
}

int main() {
  string tokenizer_test_file = read_file("tokenizer.test");
  vector<string> tokenizer_tests = get_tests(tokenizer_test_file);
  vector<TestCase> tokenizer_test_cases = get_test_cases(tokenizer_tests);
  for (const auto& t : tokenizer_test_cases) {
    print("TEST INPUT:");
    print(t.input);
    print("TEST RESULT:");
    print(t.result);
  }
}