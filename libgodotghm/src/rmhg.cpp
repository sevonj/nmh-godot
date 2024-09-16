//
// Created by sevonj on 14.9.2024.
//

#include "rmhg.h"

#include "util_read.h"

using namespace godot;  // namespace godot

void RMHG::_bind_methods() {
  godot::ClassDB::bind_method(godot::D_METHOD("open", "path"),
                              &godot::RMHG::open);
  godot::ClassDB::bind_method(godot::D_METHOD("get_strings"),
                              &godot::RMHG::get_strings);
}

void RMHG::load_stringtable(std::ifstream& file) {
  if (header.off_stringtable == 0) {
    UtilityFunctions::push_error("header.off_stringtable == 0");
    return;
  }

  file.seekg(header.off_stringtable, std::ios::beg);

  auto tbl_header = StringtableHeader::read(file);
  file.seekg(4, std::ios::cur);

  // Get pointers
  int32_t str_pointers[tbl_header.num_strings];
  for (int i = 0; i < tbl_header.num_strings; i++) {
    int32_t off_str;
    file.read(reinterpret_cast<char*>(&off_str), sizeof(int32_t));
    off_str += header.off_stringtable;
    str_pointers[i] = off_str;
  }

  // Get strings
  stringtable = PackedStringArray();
  for (auto off_str : str_pointers) {
    std::string str;
    file.seekg(off_str, std::ios::beg);
    while (true) {
      char ch;
      file.read((char*)&ch, sizeof(ch));
      if (true) {  // XOR the string
        ch ^= 0x8D;
      }
      if (ch == 0) {
        break;
      }
      str += ch;
    }
    stringtable.push_back(String(str.c_str()));
  }
}

void RMHG::open(const String& filepath) {
  UtilityFunctions::print("Opening ", filepath, " as RMHG.");
  const char* path = filepath.utf8().get_data();

  // Open file
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    UtilityFunctions::push_error("Unable to open.");
    return;
  }

  // Check magic and version
  int32_t magic;
  int32_t version;
  file.read((char*)&magic, sizeof(magic));
  if (magic != magic_const) {
    UtilityFunctions::push_error("Magic check: File isn't FLCG. ", path);
    return;
  }

  file.seekg(0, std::ios::beg);

  header = RMHGHeader::read(file);

  load_stringtable(file);

  file.close();
  UtilityFunctions::print("Done!");
}

// --- Public --- //

PackedStringArray godot::RMHG::get_strings() { return stringtable.duplicate(); }
