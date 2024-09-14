//
// Created by sevonj on 14.9.2024.
//

#include "flcg.h"

#include <cstring>
#include <fstream>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <iostream>

#include "util_swap_endian.h"

using namespace godot;  // namespace godot

void FLCG::_bind_methods() {
  godot::ClassDB::bind_method(godot::D_METHOD("open", "path"),
                              &godot::FLCG::open);
}

void FLCG::open(const String& filepath) {
  UtilityFunctions::print("Opening ", filepath, " as FLCG.");
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
    UtilityFunctions::push_error("Magic check: File isn't FLCG. ", path);
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
  file.read(reinterpret_cast<char*>(&header), sizeof(FLCGHeader));
  header.unused = swap_endian<int32_t>(header.unused);
  header.num_models = swap_endian<uint16_t>(header.num_models);
  header.num_materials = swap_endian<uint16_t>(header.num_materials);
  header.off_models = swap_endian<uint32_t>(header.off_models);
  header.off_materials = swap_endian<uint32_t>(header.off_materials);
  header.unkwnown = swap_endian<int32_t>(header.unkwnown);

  UtilityFunctions::print("File contents:");
  UtilityFunctions::print("Models:    ", header.num_models);
  UtilityFunctions::print("materials: ", header.num_materials);
  UtilityFunctions::print(" ");
  UtilityFunctions::print("unused:    ", header.unused);
  UtilityFunctions::print("unknown:   ", header.unkwnown);

  // Objects
  file.seekg(header.off_models, std::ios::beg);
  // objects.resize(header.num_models);
  load_model(this, file);

  // print_tree_pretty();

  Dictionary object_dict = Dictionary();

  file.close();
  UtilityFunctions::print("Done!");
}

void FLCG::load_model(Node3D* parent, std::ifstream& file) {
  FLCGModel mdl;
  file.read(reinterpret_cast<char*>(&mdl), sizeof(FLCGModel));
  mdl.unkf_0x08 = swap_endian<float>(mdl.unkf_0x08);
  mdl.unk_0x0c = swap_endian<int32_t>(mdl.unk_0x0c);
  mdl.off_prev = swap_endian<int32_t>(mdl.off_prev);
  mdl.off_next = swap_endian<int32_t>(mdl.off_next);
  mdl.origin[0] = swap_endian<float>(mdl.origin[0]);
  mdl.origin[1] = swap_endian<float>(mdl.origin[1]);
  mdl.origin[2] = swap_endian<float>(mdl.origin[2]);
  mdl.unk_0x2c = swap_endian<int32_t>(mdl.unk_0x2c);
  mdl.unkf_0x30 = swap_endian<float>(mdl.unkf_0x30);
  mdl.unused_0x34 = swap_endian<int32_t>(mdl.unused_0x34);
  mdl.off_data = swap_endian<int32_t>(mdl.off_data);

  /*
  // UtilityFunctions::print("name: ", mdl.name);
  UtilityFunctions::print("unkf_0x08: ", mdl.unkf_0x08);
  UtilityFunctions::print("unk_0x0c: ", mdl.unk_0x0c);
  // UtilityFunctions::print("zeropad: ", mdl.zeropad);
  UtilityFunctions::print("off_prev: ", mdl.off_prev);
  UtilityFunctions::print("off_next: ", mdl.off_next);
  UtilityFunctions::print("origin: ", mdl.origin[0], mdl.origin[1],
                          mdl.origin[2]);
  UtilityFunctions::print("unk_0x2c: ", mdl.unk_0x2c);
  UtilityFunctions::print("unkf_0x30: ", mdl.unkf_0x30);
  UtilityFunctions::print("unused_0x34: ", mdl.unused_0x34);
  UtilityFunctions::print("off_data: ", mdl.off_data);
  */
  // objects.push_back(obj);

  float x = mdl.origin[0] * .1;
  float y = mdl.origin[1] * .1;
  float z = mdl.origin[2] * .1;

  MeshInstance3D* node = memnew(MeshInstance3D);
  parent->add_child(node);
  node->set_position(Vector3(x, y, z));
  node->set_name(reinterpret_cast<char*>(&mdl.name));

  Ref<ArrayMesh> mesh = load_geometry(file, mdl);
  node->set_mesh(mesh);
  node->create_trimesh_collision();

  if (mdl.off_next != 0) {
    file.seekg(mdl.off_next, std::ios::beg);
    load_model(parent, file);
  }
}

Ref<ArrayMesh> FLCG::load_geometry(std::ifstream& file, FLCGModel& mdl) {
  Ref<ArrayMesh> mesh = memnew(ArrayMesh);
  SurfaceTool* st = memnew(SurfaceTool);

  UtilityFunctions::print("Loading model at ", mdl.off_data);

  st->begin(Mesh::PRIMITIVE_TRIANGLES);

  FLCGModelData data;
  file.seekg(mdl.off_data, std::ios::beg);
  file.read(reinterpret_cast<char*>(&data), sizeof(FLCGModelData));
  data.off_data_a = swap_endian<int32_t>(data.off_data_a);
  data.off_tris = swap_endian<int32_t>(data.off_tris);
  data.num_data_a = swap_endian<int32_t>(data.num_data_a);
  data.num_tris = swap_endian<int32_t>(data.num_tris);
  for (int ii = 0; ii < 6; ii++) {
    data.unk_vec_a[ii] = swap_endian<float>(data.unk_vec_a[ii]);
  }
  file.seekg(data.off_tris, std::ios::beg);

  for (int i = 0; i < data.num_tris; i++) {
    // break;
    FLCGColTri tri;
    file.read(reinterpret_cast<char*>(&tri), sizeof(FLCGColTri));
    tri.off_next = swap_endian<int32_t>(tri.off_next);
    tri.unused = swap_endian<int32_t>(tri.unused);
    tri.off_material = swap_endian<int32_t>(tri.off_material);
    for (int ii = 0; ii < 9; ii++) {
      tri.v0[ii] = swap_endian<float>(tri.v0[ii]) * .1;
    }
    st->add_vertex(Vector3(tri.v0[0], tri.v0[1], tri.v0[2]));
    st->add_vertex(Vector3(tri.v1[0], tri.v1[1], tri.v1[2]));
    st->add_vertex(Vector3(tri.v2[0], tri.v2[1], tri.v2[2]));
  }

  st->commit(mesh);

  return mesh;
}