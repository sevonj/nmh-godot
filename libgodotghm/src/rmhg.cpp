//
// Created by sevonj on 14.9.2024.
//

#include "rmhg.h"

using namespace godot;  // namespace godot

void RMHG::_bind_methods() {
  godot::ClassDB::bind_method(godot::D_METHOD("open", "path"),
                              &godot::RMHG::open);

  godot::ClassDB::bind_method(
      godot::D_METHOD("open_at_offset", "path", "offset"),
      &godot::RMHG::open_at_offset);

  godot::ClassDB::bind_method(godot::D_METHOD("get_strings"),
                              &godot::RMHG::get_strings);
}

void RMHG::load_stringtable(std::ifstream& file) {
  if (header.off_stringtable == 0) {
    UtilityFunctions::push_error("header.off_stringtable == 0");
    return;
  }
  file.seekg(stream_begin + header.off_stringtable, std::ios::beg);

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

void godot::RMHG::load_attributes(std::ifstream& file) {
  if (header.off_attributes == 0) {
    UtilityFunctions::push_error("header.off_stringtable == 0");
    return;
  }
  file.seekg(stream_begin + header.off_attributes, std::ios::beg);

  for (int i = 0; i < header.num_resources; i++) {
    int32_t resource_pointer;
    int32_t size;
    int32_t flags;
    int32_t version;
    int32_t str_idx;  // Directory name in string table
    file.read(reinterpret_cast<char*>(&resource_pointer), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&size), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&flags), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&version), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&str_idx), sizeof(int32_t));
    file.ignore(12);

    if (str_idx == -1) {
      UtilityFunctions::push_error("dir name string table index == -1");
      return;
    }
    String dir_name = stringtable[str_idx];
    UtilityFunctions::print("dir: ", dir_name, " size: ", size, " ptr: ", resource_pointer);
  }
}

void RMHG::open_at_offset(const String& filepath, int file_offset) {
  UtilityFunctions::print("Opening ", filepath, " as RMHG.");
  const char* path = filepath.utf8().get_data();

  // Open file
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    UtilityFunctions::push_error("Unable to open.");
    return;
  }
  file.seekg(file_offset, std::ios::beg);
  stream_begin = file.tellg();

  // Check magic and version
  int32_t magic;
  int32_t version;
  file.read((char*)&magic, sizeof(magic));
  if (magic != magic_const) {
    UtilityFunctions::push_error("Magic check: File isn't FLCG. ", path);
    return;
  }

  file.seekg(stream_begin);

  header = RMHGHeader::read(file);

  load_stringtable(file);
  load_attributes(file);

  file.close();
  UtilityFunctions::print("Done!");
}

// --- Public --- //

PackedStringArray godot::RMHG::get_strings() { return stringtable.duplicate(); }
