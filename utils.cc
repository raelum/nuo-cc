#include <format>
#include <iostream>
#include <string>
#include <vector>

using string = std::string;

template <typename T>
using vector = std::vector<T>;

template <typename T>
void print(T& message) {
  std::cout << message << std::endl;
};

template <typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args) {
  std::cout << std::vformat(fmt.get(), std::make_format_args(args...))
            << std::endl;
};

template <typename... Args>
void error(std::format_string<Args...> fmt, Args&&... args) {
  std::cout << "ERROR: "
            << std::vformat(fmt.get(), std::make_format_args(args...))
            << std::endl;
  std::abort();
};