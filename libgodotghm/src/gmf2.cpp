//
// Created by sevonj on 4.9.2024.
//

#include "gmf2.h"

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>

#include "util_swap_endian.h"

using namespace godot;  // namespace godot

void GMF2::_bind_methods() {
  godot::ClassDB::bind_method(godot::D_METHOD("open", "path"),
                              &godot::GMF2::open);
  godot::ClassDB::bind_method(godot::D_METHOD("open_at_offset", "path", "file_offset"),
                              &godot::GMF2::open_at_offset);
}

void GMF2::open(const String& filepath) { open_at_offset(filepath, 0); }

void GMF2::open_at_offset(const String& filepath, int file_offset) {
  UtilityFunctions::print("Opening ", filepath, " as GMF2.");
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
  file.read((char*)&magic, sizeof(magic));
  if (magic != magic_const) {
    UtilityFunctions::push_error("Magic check: File isn't GMF2. ", path);
    return;
  }
  int32_t version;
  file.read((char*)&version, sizeof(version));
  if (version != version_const) {
    UtilityFunctions::push_error("Version check: Expected 2, got ", version);
    return;
  }

  file.seekg(stream_begin, std::ios::beg);
  file.read(reinterpret_cast<char*>(&header), sizeof(GMF2Header));
  //*
  UtilityFunctions::print("File contents:");
  UtilityFunctions::print("Objects:   ", header.num_objects);
  UtilityFunctions::print("Textures:  ", header.num_textures);
  UtilityFunctions::print("(unused?): ", header.num_unused);
  UtilityFunctions::print("materials: ", header.num_materials);
  UtilityFunctions::print(" ");
  UtilityFunctions::print("unk_0x30: ", header.unk_0x30);
  UtilityFunctions::print("unk_0x34: ", header.unk_0x34);
  // */
  //if (file_offset != 0) {
  //  return;
  //}
  load_objects(this, file, header.off_objects);

  file.close();
  UtilityFunctions::print("Done!");
}

void GMF2::load_objects(Node3D* parent, std::ifstream& file, int offset) {
  // Loop though linked list
  int32_t off_next = offset;
  while (off_next != 0) {
    file.seekg(stream_begin + off_next, std::ios::beg);
    GMF2Object obj = GMF2Object::read(file);

    // Node creation
    MeshInstance3D* node = memnew(MeshInstance3D);
    parent->add_child(node);
    auto pos = Vector3(obj.position[0], obj.position[1], obj.position[2]) * .1;
    node->set_position(pos);
    node->rotate_y(obj.rotation[1]);
    node->set_name(obj.name);

    // Geometry
    if (obj.off_surfaces != 0) {
      Ref<ArrayMesh> mesh = load_object_geometry(file, obj);
      node->set_mesh(mesh);
    }

    // Children recurse
    if (obj.off_firstchild != 0) {
      load_objects(node, file, obj.off_firstchild);
    }

    off_next = obj.off_next;
  }
}

Ref<ArrayMesh> GMF2::load_object_geometry(std::ifstream& file,
                                          GMF2Object& obj) {
  Ref<ArrayMesh> mesh = memnew(ArrayMesh);
  SurfaceTool* st = memnew(SurfaceTool);

  auto vec_type = (obj.v_divisor == -1) ? GMF2VecType::FLOAT : GMF2VecType::I16;
  auto v_format = (obj.off_v_format == 0) ? GMF2VFormat::A : GMF2VFormat::B;

  // Loop though linked list
  int32_t off_next = obj.off_surfaces;
  while (off_next != 0) {
    file.seekg(stream_begin + off_next, std::ios::beg);

    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    // GMF Surface header
    GMF2Surface surf = GMF2Surface::read(file);

    // GMF Surface data
    // NOTICE: Big-Endian
    file.seekg(stream_begin + surf.off_data, std::ios::beg);
    uint32_t len_data;
    uint16_t num_vertices;
    int16_t unknown;
    file.read((char*)&len_data, sizeof(len_data));
    file.read((char*)&num_vertices, sizeof(num_vertices));
    file.read((char*)&unknown, sizeof(unknown));
    len_data = swap_endian<uint32_t>(len_data);
    num_vertices = swap_endian<uint16_t>(num_vertices);
    unknown = swap_endian<int16_t>(unknown);

    file.seekg(24, std::ios::cur);

    // GMF Surface data buffer
    // NOTICE: Big-Endian
    int remaining_vertices = num_vertices;
    while (remaining_vertices > 0) {
      // -- Tristrip head
      uint16_t command;
      uint16_t num_v;
      file.read((char*)&command, sizeof(command));
      command = swap_endian<uint16_t>(command);
      if (command != 0x99) {
        UtilityFunctions::push_error("Unsupported GPU command: ", command);
        break;
      }
      file.read((char*)&num_v, sizeof(num_v));
      num_v = swap_endian<uint16_t>(num_v);

      // -- Tristrip data
      Vertex vertices[num_v];

      // Read vertices
      for (int i = 0; i < num_v; i++) {
        vertices[i] = (v_format == GMF2VFormat::A)
                          ? read_vertex_b(file, obj.v_divisor, obj.off_v_buf)
                          : read_vertex_a(file, obj.v_divisor, obj.off_v_buf);
      }

      // Store vertices
      for (int i = 0; i < num_v - 2; i++) {
        Vertex v_a = vertices[i];
        Vertex v_b;
        Vertex v_c;
        // We aren't triangle-stripping, undo alternating loop direction
        if (i % 2 == 0) {
          v_b = vertices[i + 2];
          v_c = vertices[i + 1];
        } else {
          v_b = vertices[i + 1];
          v_c = vertices[i + 2];
        }
        st->set_uv(v_a.uv);
        st->set_normal(v_a.norm);
        st->add_vertex(v_a.pos);
        st->set_uv(v_b.uv);
        st->set_normal(v_b.norm);
        st->add_vertex(v_b.pos);
        st->set_uv(v_c.uv);
        st->set_normal(v_c.norm);
        st->add_vertex(v_c.pos);
      }

      remaining_vertices -= num_v;
    }

    st->commit(mesh);
    off_next = surf.off_next;
  }

  return mesh;
}

GMF2::Vertex GMF2::read_vertex_b(std::ifstream& file, int divisor,
                                 int off_buf) {
  Vertex vert;

  uint16_t idx;
  int8_t norm_x;
  int8_t norm_y;
  int8_t norm_z;
  int16_t u;
  int16_t v;
  file.read((char*)&idx, sizeof(idx));
  file.read((char*)&norm_x, sizeof(norm_x));
  file.read((char*)&norm_y, sizeof(norm_y));
  file.read((char*)&norm_z, sizeof(norm_z));
  file.read((char*)&u, sizeof(u));
  file.read((char*)&v, sizeof(v));
  idx = swap_endian<uint16_t>(idx);
  norm_x = swap_endian<int8_t>(norm_x);
  norm_y = swap_endian<int8_t>(norm_y);
  norm_z = swap_endian<int8_t>(norm_z);
  u = swap_endian<uint16_t>(u);
  v = swap_endian<uint16_t>(v);

  vert.pos = read_v_pos(divisor, file, off_buf, idx);

  vert.norm = Vector3(norm_x, norm_y, norm_z);
  vert.uv = Vector2(u, v) / pow(2, 10);

  return vert;
}

GMF2::Vertex GMF2::read_vertex_a(std::ifstream& file, int divisor,
                                 int off_buf) {
  Vertex vert;

  uint16_t idx;
  int8_t norm_x;
  int8_t norm_y;
  int8_t norm_z;
  int16_t col;
  int16_t u;
  int16_t v;
  file.read((char*)&idx, sizeof(idx));
  file.read((char*)&norm_x, sizeof(norm_x));
  file.read((char*)&norm_y, sizeof(norm_y));
  file.read((char*)&norm_z, sizeof(norm_z));
  file.read((char*)&col, sizeof(col));
  file.read((char*)&u, sizeof(u));
  file.read((char*)&v, sizeof(v));
  idx = swap_endian<uint16_t>(idx);
  norm_x = swap_endian<int8_t>(norm_x);
  norm_y = swap_endian<int8_t>(norm_y);
  norm_z = swap_endian<int8_t>(norm_z);
  col = swap_endian<uint16_t>(col);
  u = swap_endian<uint16_t>(u);
  v = swap_endian<uint16_t>(v);

  vert.pos = read_v_pos(divisor, file, off_buf, idx);

  vert.norm = Vector3(norm_x, norm_y, norm_z);
  vert.uv = Vector2(u, v) / pow(2, 10);

  return vert;
}

Vector3 GMF2::read_v_pos(int divisor, std::ifstream& file, int off_buf,
                         uint16_t idx) {
  // Remember position in file!
  auto stream_pos = file.tellg();

  Vector3 v_pos;

  // No v scale divisor means it's float. Otherwise it's short.
  if (divisor == -1) {
    file.seekg(stream_begin + off_buf + (idx * 3 * 4), std::ios::beg);

    float x;
    float y;
    float z;
    file.read((char*)&x, sizeof(x));
    file.read((char*)&y, sizeof(y));
    file.read((char*)&z, sizeof(z));
    x = swap_endian<float>(x);
    y = swap_endian<float>(y);
    z = swap_endian<float>(z);

    v_pos = Vector3(x, y, z);

  } else {
    file.seekg(stream_begin + off_buf + (idx * 3 * 2), std::ios::beg);

    int16_t x;
    int16_t y;
    int16_t z;
    file.read((char*)&x, sizeof(x));
    file.read((char*)&y, sizeof(y));
    file.read((char*)&z, sizeof(z));
    x = swap_endian<int16_t>(x);
    y = swap_endian<int16_t>(y);
    z = swap_endian<int16_t>(z);

    v_pos = Vector3(x, y, z) / pow(2., divisor);
  }
  v_pos /= 10.;

  file.seekg(stream_pos);
  return v_pos;
}
