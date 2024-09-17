#ifndef MAGIC
#define MAGIC

#include <climits>
#include <fstream>

enum Magic{
    UNKNOWN = -1,
    FLCG,
    CGT0,
    GMF2,
    RMHG,
    STRIMAG2,
    STRIMAGE,

}

// Read a null-terminated string from file stream
Magic check_magic(std::ifstream& file) {
  std::string s;
  std::getline(file, s, '\0');
  return s;
}

#endif  // MAGIC