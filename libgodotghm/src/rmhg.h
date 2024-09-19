//
// Created by sevonj on 14.9.2024.
//

#ifndef NMH_RMHG_H
#define NMH_RMHG_H

#include <fstream>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "ghm_file.h"
#include "util_swap_endian.h"

namespace godot {

// A file packed into an archive
class RMHGFileDescriptor : public RefCounted {
  GDCLASS(RMHGFileDescriptor, RefCounted)
 private:
  //
 protected:
  String filepath;            // Path of the root archive
  uint32_t off_resource;      // Offset in the root archive
  uint32_t len_resource;      // Data size
  int32_t dir_flag;           //
  int32_t version;            //
  GHMFile::MagicValue magic;  //
  String filename;            //
  bool fail = false;          //

  static void _bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("_to_string()"),
                                &godot::RMHGFileDescriptor::_to_string);
    godot::ClassDB::bind_method(godot::D_METHOD("get_filepath"),
                                &godot::RMHGFileDescriptor::get_filepath);
    godot::ClassDB::bind_method(godot::D_METHOD("get_offset"),
                                &godot::RMHGFileDescriptor::get_offset);
    godot::ClassDB::bind_method(godot::D_METHOD("get_size"),
                                &godot::RMHGFileDescriptor::get_size);
    godot::ClassDB::bind_method(godot::D_METHOD("is_dir"),
                                &godot::RMHGFileDescriptor::is_dir);
    godot::ClassDB::bind_method(godot::D_METHOD("get_version"),
                                &godot::RMHGFileDescriptor::get_version);
    godot::ClassDB::bind_method(godot::D_METHOD("get_type"),
                                &godot::RMHGFileDescriptor::get_type);
    godot::ClassDB::bind_method(godot::D_METHOD("get_name"),
                                &godot::RMHGFileDescriptor::get_name);

    godot::ClassDB::bind_method(godot::D_METHOD("is_error"),
                                &godot::RMHGFileDescriptor::is_error);
  }

 public:
  String get_filepath() { return filepath; }                     // Archive path
  uint32_t get_offset() { return off_resource; }                 //
  uint32_t get_size() { return len_resource; }                   //
  uint32_t is_dir() { return dir_flag; }                         //
  uint32_t get_version() { return version; }                     //
  String get_type() { return GHMFile::magic_to_string(magic); }  // Type str
  String get_name() { return filename; }  // Get packed file name

  String _to_string() {
    String str;
    str += "[" + get_type() + "] " + filename + " ";
    str += "is_dir: " + String(std::to_string(dir_flag).c_str()) + " ";
    str += "ver: " + String(std::to_string(version).c_str()) + " ";
    str += "off: " + String(std::to_string(off_resource).c_str()) + " ";
    str += "size: " + String(std::to_string(len_resource).c_str()) + " ";
    return str;
  };

  void set_filepath(String value) { filepath = value; }
  void set_offset(uint32_t value) { off_resource = value; }
  void set_size(uint32_t value) { len_resource = value; }
  void set_dir(int32_t value) { dir_flag = value; }
  void set_version(int32_t value) { version = value; }
  void set_magic(GHMFile::MagicValue value) { magic = value; }
  void set_filename(String value) { filename = value; }

  void set_error() { fail = true; }
  bool is_error() { return fail; }
};

// A directory packed as a nested archive
class RMHGDirDescriptor : public RMHGFileDescriptor {
  GDCLASS(RMHGDirDescriptor, RMHGFileDescriptor)
 private:
  int num_resources = -1;
  Array contents;  // File or dir descriptors

 protected:
  static void _bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("get_contents"),
                                &godot::RMHGDirDescriptor::get_contents);
  }

 public:
  String _to_string() {
    auto str = RMHGFileDescriptor::_to_string();
    str += "resources: " + String(std::to_string(num_resources).c_str()) + " ";
    if (fail) {
      str += "ERROR! ";
    }
    return str;
  };
  void set_num_res(int value) { num_resources = value; }
  void add_resource(Ref<RMHGFileDescriptor> res) { contents.append(res); }
  Array get_contents() { return contents.duplicate(); }
};

/* Grasshopper Manufacture RMGH archive object for Godot
 *
 */
class RMHG : public Node3D {
  GDCLASS(RMHG, Node3D)

 private:
  // --- Types --- //

  const int32_t magic_const = 1195920722;  // = {'R','M','H','G'}

  // File Header
  // 20B
  struct RMHGHeader {
    int8_t magic[4];             // 'RMHG'
    uint32_t num_resources;      //
    uint32_t off_resourcetable;  //
    uint32_t version;            //
    uint32_t off_stringtable;    //

    static RMHGHeader read(std::ifstream& file) {
      RMHGHeader self;
      file.read(reinterpret_cast<char*>(&self), sizeof(RMHGHeader));
      return self;
    }
  };

  // 12B little-endian
  struct StringtableHeader {
    uint32_t num_strings;  //
    int32_t unknown;       //
    int32_t flags;         //

    static StringtableHeader read(std::ifstream& file) {
      StringtableHeader self;
      file.read(reinterpret_cast<char*>(&self), sizeof(StringtableHeader));
      return self;
    }
  };

  // --- Data
  std::streamoff stream_begin;    // Not 0 if file is packed into an archive
  String opened_filepath;         //
  RMHGHeader header;              //
  PackedStringArray stringtable;  //
  Ref<RMHGDirDescriptor> root;    //

  // --- Methods

  void load_stringtable(std::ifstream& file);
  void load_attributes(std::ifstream& file);
  void load_packed_dir(Ref<RMHGDirDescriptor> dir, std::ifstream& file);

 protected:
  static void _bind_methods();

 public:
  RMHG() {};
  ~RMHG() {};

  String get_file_path() { return opened_filepath; }

  // Open file
  void open(const String& filepath) { open_at_offset(filepath, 0); }
  // Open file packed into an other file
  void open_at_offset(const String& filepath, int file_offset = 0);
  //
  PackedStringArray get_strings();
  Ref<RMHGDirDescriptor> get_root();
};

}  // namespace godot

#endif  // NMH_RMHG_H
