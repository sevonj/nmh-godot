//
// Created by sevonj on 14.9.2024.
//

#ifndef NMH_FLCG_H
#define NMH_FLCG_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

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
  };

  // ---

  FLCGHeader header;
  // std::vector<GMF2Object> objects;

  // Recursively loads objects and their children from stream, and adds them to
  // the tree.
  void load_model(Node3D* parent, std::ifstream& file);
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
