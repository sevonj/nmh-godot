#ifndef UTIL_READ
#define UTIL_READ

#include <climits>
#include <fstream>

// Read a null-terminated string from file stream
std::string read_string(std::ifstream& file) {
  std::string s;
  std::getline(file, s, '\0');
  return s;
}

#endif  // UTIL_READ