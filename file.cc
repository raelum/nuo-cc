#ifndef FILE_CC
#define FILE_CC

#include <fstream>
#include <sstream>

#include "builtins.cc"

Result<String> readFile(StringView filename) {
  // Create an input file stream.
  IfStream inputFile(filename.data());

  // Check if the file was opened successfully.
  if (!inputFile.is_open()) {
    Error("Could not open file: {}.", filename);
  }

  // Read the whole file into a buffer.
  StringStream buffer;
  buffer << inputFile.rdbuf();

  // Close the file
  inputFile.close();

  // Return the content buffer as a string.
  return Ok(buffer.str());
}

#endif  // FILE_CC