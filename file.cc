#include <fstream>
#include <sstream>
#include <string>

#include "utils.cc"

std::string read_file(std::string_view filename) {
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