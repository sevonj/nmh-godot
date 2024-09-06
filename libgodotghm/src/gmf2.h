//
// Created by sevonj on 4.9.2024.
//

#ifndef NMH_GMF2_H
#define NMH_GMF2_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

/* Grasshopper Manufacture GMF2 Node for Godot
 *
 */
class GMF2 : public Node3D {
  GDCLASS(GMF2, Node3D)

 private:
  // --- Header --- //

  const int32_t magic_const = 843468103;  // = {'G','M','F','2'}
  const int32_t version_const = 2;        //

  // GMF2 header 56B(or more if there are unused values in the following null
  // padding) struct GMF2Header {
  struct GMF2Header {
    char magic[4];        // 'GMF2'
    int version;          // 2, Probably version.
    char zeropad[16];     //
    short num_objects;    //
    short num_textures;   //
    short num_unused;     //
    short num_materials;  //
    int off_objects;      //
    int off_textures;     // Always 0x70
    int off_unused;       //
    int off_materials;    //
    int unk_0x30;         //
    int unk_0x34;         //
  };

  // --- Objects --- //

  // Very hastily tested. Probably not an exhaustive list.
  enum GMF2ObjectFlags {
    VISIBLE = 0x1,                // Hiding an object hides its children too.
    D = 0x8,                      // Makes dark billboarded objects transparent
    BILLBOARD = 0x40,             //
    L = 0x800,                    // Makes billboarded objects dark
    NO_AMBIENT_LIGHT = 0x1000,    // Shadows are pure black
    NO_DIRECT_LIGHT = 0x2000,     //
    DONT_CAST_SHADOW = 0x100000,  //
  };

  enum GMF2VertexCoordType {
    SHORT,  // Divide by 2^v_divisor.
    FLOAT,  //
  };

  /* Two formats exist?
   * Bytes legend:
   *  I: Vertex coord index (ushort)
   *  N: Vertex normal (byte[3])
   *  C: Vertex color. (unknown 2B)
   *  U: UV. (short[2])
   */
  enum GMF2VertexFormat {
    IINNNCCUUUU,  // 11B has color
    IINNNUUUU,    // 9B no color
  };

  // Surface
  // 32B
  struct GMF2Surface {
    int off_prev;      // Previous surface in linked list
    int off_next;      // Next surface in linked list
    int off_data;      // Surface data
    int off_material;  // Which material to use
    short unk_0x10;    // Corrupting this seems to do nothing.
    short num_v;       // number of vertices in shared vertex buffer
    int unk_0x14;      // Zero in all world chunks
    short unk_0x18;    // Corrupting this crashes the game.
    short unk_0x1a;    // Corrupting this causes transparency glitches.
    short unk_0x1c;    // Corrupting this seems to do nothing.
    short unk_0x1e;    // Corrupting this seems to do nothing.
  };

  // Object header
  // 128B
  struct GMF2Object {
    char name[8];               // Object name truncated to 8B. Shift JIS
    int flags;                  //
    int off_v_buf;              // Vertex buffer offset.
    int off_parent;             // Parent object
    int off_firstchild;         // Child object
    int off_prev;               // Previous object in linked list
    int off_next;               // Next object in linked list
    int off_surfaces;           //
    int unused;                 // probably unused
    int unk_0x28;               // Zero in all world chunks.
    int v_divisor;              // Exponent of vertex divisor.
    float position[3];          // XYZ coords.
    float unk_0x3c;             // Unused 4th component of previous vector?
    float rotation[3];          // Euler rotation? Quaternion?
    float unk_0x4c;             //
    float scale[3];             // XYZ scale.
    float off_v_format;         // Either 1.0f or an offset to index format.
    float cullbox_position[3];  // XYZ coords.
    float unk_0x6c;             // Unused 4th component of previous vector?
    float cullbox_size[3];      // XYZ size.
    float unk_0x7c;             // Unused 4th component of previous vector?
  };

  // Intermediate format, not GMF2.
  struct Vertex {
    Vector3 pos;
    Vector3 norm;
    Color col;
    Vector2 uv;
  };
  // ---

  GMF2Header header;
  std::vector<GMF2Object> objects;

  // Recursively loads objects and their children from stream, and adds them to
  // the tree.
  void load_objects(Node3D* parent, std::ifstream& file);
  static Ref<ArrayMesh> load_object_geometry(std::ifstream& file,
                                             GMF2Object& obj);

  static Vertex parse_vertex_iinnnuuuu(std::ifstream& file, int v_divisor,
                                       int off_v_buf);

  static Vertex parse_vertex_iinnnccuuuu(std::ifstream& file, int v_divisor,
                                         int off_v_buf);

  static Vector3 parse_vertex_pos(int vertex_coord_type, std::ifstream& file,
                                  int off_v_buf, int16_t idx);

 protected:
  static void _bind_methods();

 public:
  GMF2() {};
  ~GMF2() {};

  void open(const String& filepath);
};

}  // namespace godot

#endif  // NMH_GMF2_H
