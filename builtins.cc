#ifndef BUILTINS_CC
#define BUILTINS_CC

#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Simplified types.
using String = std::string;
using StringView = std::string_view;
using StringStream = std::stringstream;
template <typename T>
using Vector = std::vector<T>;
using IfStream = std::ifstream;
using OfStream = std::ofstream;

// Convenience print function.
template <typename T>
void print(const T& message) {
  std::cout << message << std::endl;
};
// Convenience print function with format args.
template <typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args) {
  std::cout << std::vformat(fmt.get(), std::make_format_args(args...))
            << std::endl;
};

// Helper macros to generate enums and their string names.
#define ENUM_GENERATOR(ENUM) ENUM,
#define STRING_GENERATOR(STRING) #STRING,

// Result type to gracefully handle errors.
template <typename T>
struct Result {
  enum class Type { OK, ERROR };
  Type type;
  T val;
  String err;

  bool ok() { return this->type == Type::OK; }
  T value() { return this->val; }
  String error() { return this->err; }
};

// Represents a "void" type when returning a Result from a function without a
// return value.
struct None {};

// Creates an Ok result with the given value.
template <typename T>
Result<T> Ok(T value) {
  return Result<T>{.type = Result<T>::Type::OK, .val = value};
};

// Creates an Ok result for void functions.
Result<None> Ok() {
  return Result<None>{.type = Result<None>::Type::OK, .val = None{}};
};

// Helper struct that helps us automatically deduce the template T param when
// returning an error result. C++ normally doesn't use return type to help
// deduce T, except for conversion operators. We take advantage of this, by
// returning a struct that has a templated conversion operator for Return<T>.
// C++ sees this struct and it's destination type, figures out what T should be
// before calling the conversion operator which then creates a Result<T> with
// the appropriate type!
struct DeduceReturnForErrorResult {
  String error;

  template <typename T>
  operator Result<T>() {
    return Result<T>{.type = Result<T>::Type::ERROR, .err = error};
  }
};

// Creates an Error result with the given string.
DeduceReturnForErrorResult Error(String error) {
  return DeduceReturnForErrorResult{.error = error};
};

// Creates an Error result with the given format string and arguments.
template <typename... Args>
DeduceReturnForErrorResult Error(std::format_string<Args...> fmt,
                                 Args&&... args) {
  return DeduceReturnForErrorResult{
      .error = std::vformat(fmt.get(), std::make_format_args(args...))};
};

// Helper macros for concatenating macro values.
#define CONCAT_INNER(x, y) x##y
#define CONCAT(x, y) CONCAT_INNER(x, y)

// Inner implementation of TRY_ASSIGN, which allows us to concatenate a unique
// id to the result_var variable name.
#define TRY_ASSIGN_IMPL(resultVar, dest, expr) \
  auto resultVar = (expr);                     \
  if (!resultVar.ok()) [[unlikely]] {          \
    return Error(resultVar.error());           \
  }                                            \
  dest = std::move(resultVar.value());

// Assigns the value of a Result to the destination or returns an error Result.
#define TRY_ASSIGN(dest, expr) \
  TRY_ASSIGN_IMPL(CONCAT(_result, __LINE__), dest, expr)

#endif  // BUILTINS_CC