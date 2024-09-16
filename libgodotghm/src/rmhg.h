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

#include "util_swap_endian.h"

namespace godot {

/* Grasshopper Manufacture RMGH archive object for Godot
 *
 */
class RMHG : public Node3D {
  GDCLASS(RMHG, Node3D)

 private:
  // --- Types --- //

  const int32_t magic_const = 1195920722;  // = {'R','M','H','G'}

  // File Header
  //
  struct RMHGHeader {
    int8_t magic[4];           // 'RMHG'
    uint32_t num_resources;    //
    uint32_t attr_ptr;         //
    uint32_t version;          //
    uint32_t off_stringtable;  //

    static RMHGHeader read(std::ifstream& file) {
      RMHGHeader self;
      file.read(reinterpret_cast<char*>(&self), sizeof(RMHGHeader));
      return self;
    }
  };

  // 12B little-endian
  struct StringtableHeader {
    uint32_t num_strings;
    int32_t unknown;
    int32_t flags;

    static StringtableHeader read(std::ifstream& file) {
      StringtableHeader self;
      file.read(reinterpret_cast<char*>(&self), sizeof(StringtableHeader));
      return self;
    }
  };

  // --- Data
  String file;
  RMHGHeader header;
  PackedStringArray stringtable;

  // --- Methods

  void load_stringtable(std::ifstream& file);

 protected:
  static void _bind_methods();

 public:
  RMHG() {};
  ~RMHG() {};

  void open(const String& filepath);

  //
  PackedStringArray get_strings();
};

}  // namespace godot

#endif  // NMH_RMHG_H
