#ifndef FILE_CC
#define FILE_CC

#include <fstream>
#include <sstream>

#include "builtins.cc"

String readFile(StringView filename) {
  // Create an input file stream.
  std::ifstream inputFile(filename.data());

  // Check if the file was opened successfully.
  if (!inputFile.is_open()) {
    error("Could not open file: {}.", filename);
  }

  // Read the whole file into a buffer.
  std::stringstream buffer;
  buffer << inputFile.rdbuf();

  // Close the file
  inputFile.close();

  // Return the content buffer as a string.
  return buffer.str();
}

#endif  // FILE_CC