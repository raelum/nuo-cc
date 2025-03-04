#ifndef FILE_CC
#define FILE_CC

#include <fstream>
#include <sstream>

#include "builtins.cc"

Result<String> readFile(StringView fileName) {
  IfStream inputFile(fileName.data());
  if (!inputFile.is_open()) {
    Error("Could not open file: {}.", fileName);
  }
  StringStream buffer;
  buffer << inputFile.rdbuf();
  inputFile.close();
  return Ok(buffer.str());
}

Result<None> writeFile(StringView fileName, StringView fileContents) {
  OfStream outputFile(fileName.data());
  if (!outputFile.is_open()) {
    Error("Could not open file: {}.", fileName);
  }
  outputFile << fileContents;
  outputFile.close();
  return Ok();
}

#endif  // FILE_CC