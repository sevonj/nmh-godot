//
// Created by sevonj on 14.9.2024.
//

#ifndef NMH_FLCG_H
#define NMH_FLCG_H

#include <fstream>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "util_swap_endian.h"

namespace godot {

/* Grasshopper Manufacture FLCG Node for Godot
 *
 */
class FLCG : public Node3D {
  GDCLASS(FLCG, Node3D)

 private:
  // --- Header --- //

  const int32_t magic_const = 1195592774;  // = {'F','L','C','G'} (as LE int!)
  const int32_t version_const = 1;         //

  // FLCG header
  // 28B (or more if there are unused values in the following padding)
  struct FLCGHeader {
    int8_t magic[4];         // 'FLCG'
    int32_t version;         // 1 (in Little-endian!), Probably version.
    int32_t unused;          // Probably unused.
    uint16_t num_models;     //
    uint16_t num_materials;  //
    uint32_t off_models;     // After materials?
    uint32_t off_materials;  // 0x40?
    int32_t unkwnown;        //

    static FLCGHeader read(std::ifstream& file) {
      FLCGHeader self;
      file.read(reinterpret_cast<char*>(&self), sizeof(FLCGHeader));
      // magic not swapped!
      // version not swapped!
      self.unused = swap_endian<int32_t>(self.unused);
      self.num_models = swap_endian<uint16_t>(self.num_models);
      self.num_materials = swap_endian<uint16_t>(self.num_materials);
      self.off_models = swap_endian<uint32_t>(self.off_models);
      self.off_materials = swap_endian<uint32_t>(self.off_materials);
      self.unkwnown = swap_endian<int32_t>(self.unkwnown);
      return self;
    }
  };

  // --- Models --- //

  // FLCG Model header
  // 60B (or more if there are unused values in the following padding)
  struct FLCGModel {
    int8_t name[8];       // Object name truncated to 8B. Shift JIS
    float unkf_0x08;      //
    int32_t unk_0x0c;     //
    int8_t zeropad[8];    //
    int32_t off_prev;     // Linked list
    int32_t off_next;     // Linked list
    float origin[3];      // XYZ position
    int32_t unk_0x2c;     // Always 0. Unused 4th component of previous vector?
    float unkf_0x30;      //
    int32_t unused_0x34;  // Always 0
    int32_t off_data;     //

    // Read from file
    static FLCGModel read(std::ifstream& file) {
      FLCGModel self;
      file.read(reinterpret_cast<char*>(&self), sizeof(FLCGModel));
      // name not swapped!
      self.unkf_0x08 = swap_endian<float>(self.unkf_0x08);
      self.unk_0x0c = swap_endian<int32_t>(self.unk_0x0c);
      // zeropad[8] not swapped!
      self.off_prev = swap_endian<int32_t>(self.off_prev);
      self.off_next = swap_endian<int32_t>(self.off_next);
      self.origin[0] = swap_endian<float>(self.origin[0]);
      self.origin[1] = swap_endian<float>(self.origin[1]);
      self.origin[2] = swap_endian<float>(self.origin[2]);
      self.unk_0x2c = swap_endian<int32_t>(self.unk_0x2c);
      self.unkf_0x30 = swap_endian<float>(self.unkf_0x30);
      self.unused_0x34 = swap_endian<int32_t>(self.unused_0x34);
      self.off_data = swap_endian<int32_t>(self.off_data);
      return self;
    };
  };

  // FLCG Models data
  // 40B
  struct FLCGModelData {
    int32_t off_data_a;   // Offset of unk. data. Immediately after this struct?
    int32_t off_tris;     // Offset of triangles
    uint32_t num_data_a;  // Number of unknown data
    uint32_t num_tris;    // Col. triangles
    float unk_vec_a[3];   // These are probably a bounding box.
    float unk_vec_b[3];   //

    static FLCGModelData read(std::ifstream& file) {
      FLCGModelData self;
      file.read(reinterpret_cast<char*>(&self), sizeof(FLCGModelData));
      self.off_data_a = swap_endian<int32_t>(self.off_data_a);
      self.off_tris = swap_endian<int32_t>(self.off_tris);
      self.num_data_a = swap_endian<uint32_t>(self.num_data_a);
      self.num_tris = swap_endian<uint32_t>(self.num_tris);
      self.unk_vec_a[0] = swap_endian<float>(self.unk_vec_a[0]);
      self.unk_vec_a[1] = swap_endian<float>(self.unk_vec_a[1]);
      self.unk_vec_a[2] = swap_endian<float>(self.unk_vec_a[2]);
      self.unk_vec_b[0] = swap_endian<float>(self.unk_vec_b[0]);
      self.unk_vec_b[1] = swap_endian<float>(self.unk_vec_b[1]);
      self.unk_vec_b[2] = swap_endian<float>(self.unk_vec_b[2]);
      return self;
    }
  };

  // FLCG Model data A
  // 32B
  struct FLCGModelDataA {
    int32_t off_firstchild;  // ?? Anyway, these form some kind of hierarchy.
    int32_t off_next;        // ?? Anyway, these form some kind of hierarchy.
    int32_t off_tri;         // Offset of a col. triangle.
    int32_t flags;           // Maybe
    float unk_vec[4];        // 4x unknown floats
  };

  // FLCG Collision Triangle
  // 48B
  struct FLCGColTri {
    int32_t off_next;      //
    int32_t unused;        // Always 0?
    int32_t off_material;  //
    float v0[3];           //
    float v1[3];           //
    float v2[3];           //

    static FLCGColTri read(std::ifstream& file) {
      FLCGColTri self;
      file.read(reinterpret_cast<char*>(&self), sizeof(FLCGColTri));
      self.off_next = swap_endian<int32_t>(self.off_next);
      self.unused = swap_endian<int32_t>(self.unused);
      self.off_material = swap_endian<int32_t>(self.off_material);
      self.v0[0] = swap_endian<float>(self.v0[0]);
      self.v0[1] = swap_endian<float>(self.v0[1]);
      self.v0[2] = swap_endian<float>(self.v0[2]);
      self.v1[0] = swap_endian<float>(self.v1[0]);
      self.v1[1] = swap_endian<float>(self.v1[1]);
      self.v1[2] = swap_endian<float>(self.v1[2]);
      self.v2[0] = swap_endian<float>(self.v2[0]);
      self.v2[1] = swap_endian<float>(self.v2[1]);
      self.v2[2] = swap_endian<float>(self.v2[2]);
      return self;
    }
  };

  // ---

  FLCGHeader header;

  // Load models and add them as children.
  void load_models(Node3D* parent, std::ifstream& file, int offset);
  // Load geometry of a specific model.
  static Ref<ArrayMesh> load_geometry(std::ifstream& file, FLCGModel& obj);

 protected:
  static void _bind_methods();

 public:
  FLCG() {};
  ~FLCG() {};

  void open(const String& filepath);
};

}  // namespace godot

#endif  // NMH_FLCG_H
