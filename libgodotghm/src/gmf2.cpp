//
// Created by sevonj on 4.9.2024.
//

#include "gmf2.h"

#include <cstring>
#include <fstream>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <iostream>

#include "util_swap_endian.h"

using namespace godot;  // namespace godot

void GMF2::_bind_methods() {
  godot::ClassDB::bind_method(godot::D_METHOD("open", "path"),
                              &godot::GMF2::open);
}

void GMF2::open(const String& filepath) {
  UtilityFunctions::print("Opening ", filepath, " as GMF2.");
  const char* path = filepath.utf8().get_data();

  // Open file
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    UtilityFunctions::push_error("Unable to open.");
    return;
  }

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
  file.seekg(0, std::ios::beg);

  // File header
  file.read(reinterpret_cast<char*>(&header), sizeof(GMF2Header));
  UtilityFunctions::print("File contents:");
  UtilityFunctions::print("Objects:   ", header.num_objects);
  UtilityFunctions::print("Textures:  ", header.num_textures);
  UtilityFunctions::print("(unused?): ", header.num_unused);
  UtilityFunctions::print("materials: ", header.num_materials);
  UtilityFunctions::print(" ");
  UtilityFunctions::print("unk_0x30: ", header.unk_0x30);
  UtilityFunctions::print("unk_0x34: ", header.unk_0x34);

  // Objects
  file.seekg(header.off_objects, std::ios::beg);
  objects.resize(header.num_objects);
  load_objects(this, file);

  // print_tree_pretty();

  Dictionary object_dict = Dictionary();

  file.close();
  UtilityFunctions::print("Done!");
}

void GMF2::load_objects(Node3D* parent, std::ifstream& file) {
  GMF2Object obj;
  file.read(reinterpret_cast<char*>(&obj), sizeof(GMF2Object));
  objects.push_back(obj);

  float x = obj.position[0] * .1;
  float y = obj.position[1] * .1;
  float z = obj.position[2] * .1;

  MeshInstance3D* node = memnew(MeshInstance3D);
  parent->add_child(node);
  node->set_position(Vector3(x, y, z));
  node->rotate_y(obj.rotation[1]);
  node->set_name(obj.name);

  if (obj.off_surfaces != 0) {
    Ref<ArrayMesh> mesh = load_object_geometry(file, obj);
    node->set_mesh(mesh);
  }

  if (obj.off_firstchild != 0) {
    file.seekg(obj.off_firstchild, std::ios::beg);
    load_objects(node, file);
  }

  if (obj.off_next != 0) {
    file.seekg(obj.off_next, std::ios::beg);
    load_objects(parent, file);
  }
}

Ref<ArrayMesh> GMF2::load_object_geometry(std::ifstream& file,
                                          GMF2Object& obj) {
  Ref<ArrayMesh> mesh = memnew(ArrayMesh);
  SurfaceTool* st = memnew(SurfaceTool);

  int vertex_coord_type;
  if (obj.v_divisor == -1) {
    vertex_coord_type = GMF2VertexCoordType::FLOAT;
  } else {
    vertex_coord_type = GMF2VertexCoordType::SHORT;
  }

  int vertex_format;
  if (obj.off_v_format == 0) {
    vertex_format = GMF2VertexFormat::IINNNUUUU;
  } else {
    vertex_format = GMF2VertexFormat::IINNNCCUUUU;
  }

  int off_next = obj.off_surfaces;

  while (off_next != 0) {
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    // GMF Surface header
    GMF2Surface surf;
    file.seekg(off_next, std::ios::beg);
    file.read(reinterpret_cast<char*>(&surf), sizeof(GMF2Surface));

    // GMF Surface data
    // NOTICE: Big-Endian
    file.seekg(surf.off_data, std::ios::beg);
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
      uint16_t command;
      uint16_t num_v;
      file.read((char*)&command, sizeof(command));
      command = swap_endian<uint16_t>(command);
      if (command != 0x99) {
        UtilityFunctions::push_error("ERR Cmd is: ", command);
        st->commit(mesh);
        break;
      }

      file.read((char*)&num_v, sizeof(num_v));
      num_v = swap_endian<uint16_t>(num_v);
      remaining_vertices -= num_v;
      Vertex vertices[num_v];

      for (int i = 0; i < num_v; i++) {
        if (vertex_format == GMF2VertexFormat::IINNNUUUU) {
          vertices[i] =
              parse_vertex_iinnnuuuu(file, obj.v_divisor, obj.off_v_buf);
        } else {
          vertices[i] =
              parse_vertex_iinnnccuuuu(file, obj.v_divisor, obj.off_v_buf);
        }
      }
      for (int i = 0; i < num_v - 2; i++) {
        Vertex v_a = vertices[i];
        Vertex v_b;
        Vertex v_c;
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
    }

    st->commit(mesh);

    off_next = surf.off_next;
  }

  return mesh;
}

GMF2::Vertex GMF2::parse_vertex_iinnnuuuu(std::ifstream& file, int v_divisor,
                                          int off_v_buf) {
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

  vert.pos = parse_vertex_pos(v_divisor, file, off_v_buf, idx);

  vert.norm = Vector3(norm_x, norm_y, norm_z);
  vert.uv = Vector2(u, v) / pow(2, 10);

  return vert;
}

GMF2::Vertex GMF2::parse_vertex_iinnnccuuuu(std::ifstream& file, int v_divisor,
                                            int off_v_buf) {
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

  vert.pos = parse_vertex_pos(v_divisor, file, off_v_buf, idx);

  vert.norm = Vector3(norm_x, norm_y, norm_z);
  vert.uv = Vector2(u, v) / pow(2, 10);

  return vert;
}

Vector3 GMF2::parse_vertex_pos(int v_divisor, std::ifstream& file,
                               int off_v_buf, int16_t idx) {
  // Remember position in file!
  auto stream_pos = file.tellg();

  Vector3 v_pos;

  // No v scale divisor means it's float. Otherwise it's short.
  if (v_divisor == -1) {
    file.seekg(off_v_buf + (idx * 3 * 4), std::ios::beg);

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
    file.seekg(off_v_buf + (idx * 3 * 2), std::ios::beg);

    int16_t x;
    int16_t y;
    int16_t z;
    file.read((char*)&x, sizeof(x));
    file.read((char*)&y, sizeof(y));
    file.read((char*)&z, sizeof(z));
    x = swap_endian<int16_t>(x);
    y = swap_endian<int16_t>(y);
    z = swap_endian<int16_t>(z);

    v_pos = Vector3(x, y, z) / pow(2., v_divisor);
  }
  v_pos /= 10.;

  file.seekg(stream_pos);
  return v_pos;
}
