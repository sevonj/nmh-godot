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

  godot::ClassDB::bind_method(godot::D_METHOD("get_root"),
                              &godot::RMHG::get_root);

  godot::ClassDB::bind_method(godot::D_METHOD("get_file_path"),
                              &godot::RMHG::get_file_path);
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
      if (header.version > 1040) {  // XOR the string
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
  if (header.off_resourcetable == 0) {
    UtilityFunctions::push_error("header.off_stringtable == 0");
    return;
  }
  file.seekg(stream_begin + header.off_resourcetable, std::ios::beg);

  root.instantiate();
  root->set_offset(0);
  root->set_size(file.left);
  root->set_dir(1);
  root->set_version(header.version);
  root->set_magic(GHMFile::magic32(*((int32_t*)header.magic)));
  // because .split[-1] crashes?
  auto pathsplit = opened_filepath.split("/", false);
  root->set_filename(pathsplit[pathsplit.size() - 1]);

  load_packed_dir(root, file);
}

void RMHG::load_packed_dir(Ref<RMHGDirDescriptor> parent, std::ifstream& file) {
  file.seekg(stream_begin + parent->get_offset());
  auto dir_header = RMHGHeader::read(file);
  parent->set_num_res(dir_header.num_resources);

  file.seekg(parent->get_offset() + dir_header.off_resourcetable,
             std::ios::beg);

  UtilityFunctions::print(" Name: ", parent->get_name(),
                          " Offset: ", parent->get_offset());
  UtilityFunctions::print(" num_res: ", dir_header.num_resources,
                          " off_res: ", dir_header.off_resourcetable,
                          " off_str: ", dir_header.off_stringtable);

  // Sanity check: Subdir shared stringtable
  if (parent->get_offset() != 0 && dir_header.off_stringtable != 0) {
    UtilityFunctions::push_error("Subdir has its own stringtable?!");
    return;
  }

  for (int i = 0; i < dir_header.num_resources; i++) {
    int32_t off_resource;  //
    int32_t len_resource;  //
    int32_t is_dir;        //
    int32_t res_version;   //
    int32_t str_idx;       // Directory name in string table
    file.read(reinterpret_cast<char*>(&off_resource), sizeof(int32_t));
    off_resource += parent->get_offset();
    file.read(reinterpret_cast<char*>(&len_resource), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&is_dir), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&res_version), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&str_idx), sizeof(int32_t));
    file.ignore(12);

    bool fail = false;

    // Sanity check: res offset
    if (off_resource == 0) {
      UtilityFunctions::push_error("Resource offset is zero!");
      fail = true;
    }
    // Sanity check: res size
    if (len_resource == 0) {
      UtilityFunctions::push_warning("Resource length is zero!");
      if (is_dir) {
        UtilityFunctions::push_error("Subdir size is 0!");
        fail = true;
      }
    }
    // Sanity check: Dir flag
    if (!(0 <= is_dir && is_dir <= 1)) {
      UtilityFunctions::push_error("is_dir value out of range! got ", is_dir);
      fail = true;
    }
    // Sanity check: Version
    if (res_version != header.version) {
      UtilityFunctions::push_warning("resource version mismatch: ", res_version,
                                     "/", header.version);
    }
    // Sanity check: string index
    if (!(0 <= str_idx < stringtable.size())) {
      UtilityFunctions::push_error("String table index out of range!");
      fail = true;
    }

    if (fail) {
      parent->set_error();

      Ref<RMHGFileDescriptor> resource;
      resource.instantiate();
      resource->set_filepath(opened_filepath);
      resource->set_offset(off_resource);
      resource->set_size(len_resource);
      resource->set_dir(is_dir);
      resource->set_version(res_version);
      resource->set_magic(GHMFile::MV_UNKNOWN);
      String filename = "ERROR ";
      resource->set_filename((0 <= str_idx && str_idx < stringtable.size())
                                 ? stringtable[str_idx] + " [ERROR]"
                                 : "??? [ERROR]");
      resource->set_error();
      parent->add_resource(resource);
      continue;
      if (0 <= str_idx < stringtable.size()) {
        UtilityFunctions::print("str_idx: ", str_idx, " - ",
                                stringtable[str_idx]);
      }
      UtilityFunctions::print("off: ", off_resource);
      UtilityFunctions::print("len: ", off_resource);
      UtilityFunctions::print("dir: ", is_dir);
      UtilityFunctions::print("ver: ", res_version);
      UtilityFunctions::print("str_idx: ", str_idx);
      UtilityFunctions::print("");
      return;
    }
    // We do some seeking for magic and recursion, but we're also looping
    // resources here
    auto stream_position = file.tellg();

    file.seekg(off_resource);
    auto res_magic = GHMFile::get_magic(file);
    file.seekg(stream_position, std::ios::beg);

    if (str_idx < stringtable.size()) {
      UtilityFunctions::print("str_idx: ", str_idx, " - ",
                              stringtable[str_idx]);
    } else {
      UtilityFunctions::print("str_idx: ", str_idx);
    }

    if (is_dir == 0) {
      Ref<RMHGFileDescriptor> resource;
      resource.instantiate();
      resource->set_filepath(opened_filepath);
      resource->set_offset(off_resource);
      resource->set_size(len_resource);
      resource->set_dir(is_dir);
      resource->set_version(res_version);
      resource->set_magic(res_magic);
      resource->set_filename(stringtable[str_idx]);
      parent->add_resource(resource);
    } else {
      // Sanity check: Magic
      if (res_magic != GHMFile::MV_RMHG) {
        UtilityFunctions::push_error(
            "Packed subdirectory isn't a valid archive!");
        return;
      }
      Ref<RMHGDirDescriptor> subdir;
      subdir.instantiate();
      subdir->set_filepath(opened_filepath);
      subdir->set_offset(off_resource);
      subdir->set_size(len_resource);
      subdir->set_dir(is_dir);
      subdir->set_version(res_version);
      subdir->set_magic(res_magic);
      subdir->set_filename(stringtable[str_idx]);
      parent->add_resource(subdir);
      load_packed_dir(subdir, file);
      file.seekg(stream_position, std::ios::beg);
    }
  }
}

void RMHG::open_at_offset(const String& filepath, int file_offset) {
  UtilityFunctions::print("Opening ", filepath, " as RMHG.");
  opened_filepath = filepath;
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

  UtilityFunctions::print("num_res: ", header.num_resources);
  UtilityFunctions::print("off_att: ", header.off_resourcetable);
  UtilityFunctions::print("off_str: ", header.off_stringtable);

  if (file_offset != 0) {
    return;
  }

  load_stringtable(file);
  UtilityFunctions::print("stringtable: ", stringtable);
  load_attributes(file);

  file.close();
  UtilityFunctions::print("Done!");
}

// --- Public --- //

PackedStringArray godot::RMHG::get_strings() { return stringtable.duplicate(); }

Ref<RMHGDirDescriptor> godot::RMHG::get_root() { return root; }
