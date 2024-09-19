#ifndef SHIFTJIS_H
#define SHIFTJIS_H

#include <iconv.h>

#include <cerrno>
#include <cstring>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>
#include <vector>

using namespace godot;

std::string shiftjis_to_utf8(const std::string& input_str) {
  // Create an iconv conversion descriptor for Shift-JIS to UTF-8
  iconv_t cd = iconv_open("UTF-8", "SHIFT-JIS");
  if (cd == (iconv_t)-1) {
    UtilityFunctions::print("Failed to create iconv descriptor: ",
                            strerror(errno));
    return "";
  }

  return input_str;

  // Prepare buffers for input and output
  std::size_t inBytesLeft = input_str.size();
  std::size_t outBytesLeft = inBytesLeft * 4;  // UTF-8 may require more space

  std::vector<char> outputBuffer(outBytesLeft);
  char* inBuf = const_cast<char*>(input_str.data());
  char* outBuf = outputBuffer.data();

  // Perform the conversion
  if (iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft) ==
      (std::size_t)-1) {
    UtilityFunctions::print("iconv conversion error: ", strerror(errno));
    iconv_close(cd);
    return "";
  }

  // Close the iconv descriptor
  iconv_close(cd);

  // Return the converted UTF-8 string
  return std::string(outputBuffer.data(), outputBuffer.size() - outBytesLeft);
}

#endif  // SHIFTJIS_H