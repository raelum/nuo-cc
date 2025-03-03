#ifndef BUILTINS_CC
#define BUILTINS_CC

#include <format>
#include <iostream>
#include <string>
#include <vector>

// Simplified types.
using String = std::string;
using StringView = std::string_view;
template <typename T>
using Vector = std::vector<T>;

// Convenience print function.
template <typename T>
void print(T& message) {
  std::cout << message << std::endl;
};
// Convenience print function with format args.
template <typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args) {
  std::cout << std::vformat(fmt.get(), std::make_format_args(args...))
            << std::endl;
};

// Prints the error message with format args and aborts the program.
template <typename... Args>
void error(std::format_string<Args...> fmt, Args&&... args) {
  std::cout << "ERROR: "
            << std::vformat(fmt.get(), std::make_format_args(args...))
            << std::endl;
  std::abort();
};

// Use these macros to easily generate ENUMS and their associated string name.
#define ENUM_GENERATOR(ENUM) ENUM,
#define STRING_GENERATOR(STRING) #STRING,

#endif  // BUILTINS_CC