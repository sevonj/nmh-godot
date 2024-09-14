//
// Created by sevonj on 14.9.2024.
//

#include "flcg.h"

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>

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
  int32_t version;
  file.read((char*)&magic, sizeof(magic));
  if (magic != magic_const) {
    UtilityFunctions::push_error("Magic check: File isn't FLCG. ", path);
    return;
  }
  file.read((char*)&version, sizeof(version));
  if (version != version_const) {
    UtilityFunctions::push_error("Version check: Expected 2, got ", version);
    return;
  }
  file.seekg(0, std::ios::beg);

  header = FLCGHeader::read(file);

  load_models(this, file, header.off_models);

  file.close();
  UtilityFunctions::print("Done!");
}

void FLCG::load_models(Node3D* parent, std::ifstream& file, int offset) {
  // Loop though the linked list
  int32_t off_next = offset;
  while (off_next != 0) {
    file.seekg(off_next, std::ios::beg);
    FLCGModel mdl = FLCGModel::read(file);

    // Node creation
    MeshInstance3D* node = memnew(MeshInstance3D);
    parent->add_child(node);
    auto origin = Vector3(mdl.origin[0], mdl.origin[1], mdl.origin[2]) * .1;
    node->set_position(origin);
    node->set_name(reinterpret_cast<char*>(&mdl.name));

    // Mesh
    Ref<ArrayMesh> mesh = load_geometry(file, mdl);
    node->set_mesh(mesh);
    node->create_trimesh_collision();

    off_next = mdl.off_next;
  }
}

Ref<ArrayMesh> FLCG::load_geometry(std::ifstream& file, FLCGModel& mdl) {
  file.seekg(mdl.off_data, std::ios::beg);
  FLCGModelData data = FLCGModelData::read(file);

  // Mesh tools
  Ref<ArrayMesh> mesh = memnew(ArrayMesh);
  SurfaceTool* st = memnew(SurfaceTool);
  st->begin(Mesh::PRIMITIVE_TRIANGLES);

  // Load tris
  file.seekg(data.off_tris, std::ios::beg);
  for (int i = 0; i < data.num_tris; i++) {
    FLCGColTri tri = FLCGColTri::read(file);
    st->add_vertex(Vector3(tri.v0[0], tri.v0[1], tri.v0[2]) * .1);
    st->add_vertex(Vector3(tri.v1[0], tri.v1[1], tri.v1[2]) * .1);
    st->add_vertex(Vector3(tri.v2[0], tri.v2[1], tri.v2[2]) * .1);
  }

  st->commit(mesh);

  return mesh;
}