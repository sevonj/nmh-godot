//
// Created by sevonj on 17.9.2024.
//

#ifndef MAGIC
#define MAGIC

#include <fstream>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

//
class GHMFile : public Object {
  GDCLASS(GHMFile, Object)

 private:
  //
 protected:
  static void _bind_methods() {
    godot::ClassDB::bind_static_method(get_class_static(),
                                       godot::D_METHOD("get_file_type", "path"),
                                       &GHMFile::get_file_type);
  }

 public:
  GHMFile() {};
  ~GHMFile() {};

  // magic values as little-endian int32 (int64 for strimages)
  enum MagicValue {
    MV_UNKNOWN = -1,
    MV_FLCG = 1195592774,
    MV_GAN2 = 843989319,
    MV_GCT0 = 810828615,
    MV_GDLG = 1196180551,
    MV_GMF2 = 843468103,
    MV_RMHG = 1195920722,
    MV_RSAR = 1380012882,
    MV_RSTM = 1297371986,
    MV_SCR0 = 810697555,
    MV_SEST = 1414743379,
    MV_STMD = 1145918547,
    MV_STSD = 1146311763,
    MV_STRI____ = 1230132307,
    MV_STRIMAG2 = 4992030512161903699,
    MV_STRIMAGE = 3622936225441272915,
    MV_THP = 5261396,
  };

  // Read a null-terminated string from file stream
  static MagicValue get_magic(std::ifstream& file) {
    int32_t magic32;
    file.read((char*)&magic32, sizeof(magic32));

    switch (magic32) {
      case MV_FLCG:
        return MagicValue::MV_FLCG;
      case MV_GAN2:
        return MagicValue::MV_GAN2;
      case MV_GCT0:
        return MagicValue::MV_GCT0;
      case MV_GDLG:
        return MagicValue::MV_GDLG;
      case MV_GMF2:
        return MagicValue::MV_GMF2;
      case MV_RMHG:
        return MagicValue::MV_RMHG;
      case MV_RSAR:
        return MagicValue::MV_RSAR;
      case MV_RSTM:
        return MagicValue::MV_RSTM;
      case MV_SCR0:
        return MagicValue::MV_SCR0;
      case MV_SEST:
        return MagicValue::MV_RSTM;
      case MV_STMD:
        return MagicValue::MV_STMD;
      case MV_STSD:
        return MagicValue::MV_STSD;
      case MV_STRI____:
        file.seekg(-4, std::ios::cur);
        int64_t magic64;
        file.read((char*)&magic64, sizeof(magic64));
        switch (magic64) {
          case MV_STRIMAG2:
            return MagicValue::MV_STRIMAG2;
          case MV_STRIMAGE:
            return MagicValue::MV_STRIMAGE;
          default:
            return MagicValue::MV_UNKNOWN;
        }
      case MV_THP:
        return MagicValue::MV_THP;
      default:
        return MV_UNKNOWN;
    }
  }

  static MagicValue magic32(int32_t magic) {
    switch (magic) {
      case MV_FLCG:
        return MagicValue::MV_FLCG;
      case MV_GAN2:
        return MagicValue::MV_GAN2;
      case MV_GCT0:
        return MagicValue::MV_GCT0;
      case MV_GDLG:
        return MagicValue::MV_GDLG;
      case MV_GMF2:
        return MagicValue::MV_GMF2;
      case MV_RMHG:
        return MagicValue::MV_RMHG;
      case MV_RSAR:
        return MagicValue::MV_RSAR;
      case MV_RSTM:
        return MagicValue::MV_RSTM;
      case MV_SCR0:
        return MagicValue::MV_SCR0;
      case MV_SEST:
        return MagicValue::MV_RSTM;
      case MV_STMD:
        return MagicValue::MV_STMD;
      case MV_STSD:
        return MagicValue::MV_STSD;
      case MV_STRI____:
        return MagicValue::MV_STRI____;
      case MV_THP:
        return MagicValue::MV_THP;
      default:
        return MV_UNKNOWN;
    }
  }

  // Open a file and get a type string.
  static String get_file_type(const String& filepath) {
    const char* path = filepath.utf8().get_data();

    std::ifstream file(path, std::ios::binary);
    if (!file) {
      UtilityFunctions::push_error("Unable to open.");
      return String();
    }
    return magic_to_string(get_magic(file));
  }

  static String magic_to_string(MagicValue magic) {
    switch (magic) {
      case MV_FLCG:
        return String("FLCG");
      case MV_GAN2:
        return String("GAN2");
      case MV_GCT0:
        return String("GCT0");
      case MV_GDLG:
        return String("GDLG");
      case MV_GMF2:
        return String("GMF2");
      case MV_RMHG:
        return String("RMHG");
      case MV_RSAR:
        return String("RSAR");
      case MV_RSTM:
        return String("RSTM");
      case MV_SCR0:
        return String("SCR0");
      case MV_SEST:
        return String("RSTM");
      case MV_STMD:
        return String("STMD");
      case MV_STSD:
        return String("STSD");
      case MV_STRIMAG2:
        return String("STRIMAG2");
      case MV_STRIMAGE:
        return String("STRIMAGE");
      case MV_THP:
        return String("THP");
      default:
        return String("unknown format!");
    }
  }
};
}  // namespace godot
#endif  // MAGIC