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

// Inner implementation of TRY_ASSIGN.
#define TRY_ASSIGN_IMPL(resultVar, dest, expr) \
  auto resultVar = (expr);                     \
  if (!resultVar.ok()) [[unlikely]] {          \
    return Error(resultVar.error());           \
  }                                            \
  dest = std::move(resultVar.value());

// Assigns the value of a Result to the destination or returns an error Result.
// We defer the implementation TRY_ASSIGN_IMPL which allows us to concat line
// number to end of _result variable.
#define TRY_ASSIGN(dest, expr) \
  TRY_ASSIGN_IMPL(CONCAT(_result, __LINE__), dest, expr)

// Inner implementation of TRY_CALL. We expect TRY_CALL to be used for functions
// that return a Result<None> so we apply a static assert to enforce this.
#define TRY_CALL_IMPL(result_var, expr)                                  \
  auto result_var = (expr);                                              \
  static_assert(std::is_same<decltype(result_var), Result<None>>::value, \
                "Single argument to TRY must be of Result<None> type."); \
  if (!result_var.ok()) [[unlikely]] {                                   \
    return Error(result_var.error());                                    \
  }

// Returns an error Result if the called expression returned an error Result.
// We defer the implementation TRY_CALL_IMPL which allows us to concat line
// number to end of _result variable.
#define TRY_CALL(expr) TRY_CALL_IMPL(CONCAT(_result, __LINE__), expr)

// Helper function to get the desired macro name based on the number of
// args passed to it.
#define GET_MACRO(_1, _2, NAME, ...) NAME

// Handles both TRY_ASSIGN or TRY_CALL cases. Usage example:
// TRY(int i, attemptCreatingInt());
// TRY(attemptCalculation());
//
// It does this by using GET_MACRO to produce the target macro name depening
// on the number of arguments provided to this one. For example if TRY
// receives 1 arg, then GET_MACRO sees arg1, TRY_ASSIGN, TRY_CALL
// from which RETURN_IF_ERROR is picked to be returned.
#define TRY(...) GET_MACRO(__VA_ARGS__, TRY_ASSIGN, TRY_CALL)(__VA_ARGS__)

#endif  // BUILTINS_CC