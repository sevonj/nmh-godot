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
                                       godot::D_METHOD("get_magic_str", "path"),
                                       &GHMFile::get_magic_str);
  }

 public:
  GHMFile() {};
  ~GHMFile() {};

  enum MagicValues {
    MV_UNKNOWN = -1,
    MV_FLCG = 1195592774,
    MV_GAN2 = 843989319,
    MV_CGT0 = 810828615,
    MV_GMF2 = 843468103,
    MV_RMHG = 1195920722,
    MV_RSAR = 1380012882,
    MV_RSTM = 1297371986,
    MV_SEST = 1414743379,
    MV_STMD = 1145918547,
    MV_STSD = 1146311763,
    MV_STRI____ = 1230132307,
    MV_STRIMAG2 = 4992030512161903699,
    MV_STRIMAGE = 3622936225441272915,
    MV_THP = 5261396,
  };

  // Read a null-terminated string from file stream
  static MagicValues get_magic(std::ifstream& file) {
    int32_t magic32;
    file.read((char*)&magic32, sizeof(magic32));

    switch (magic32) {
      case MV_FLCG:
        return MagicValues::MV_FLCG;
      case MV_GAN2:
        return MagicValues::MV_GAN2;
      case MV_CGT0:
        return MagicValues::MV_CGT0;
      case MV_GMF2:
        return MagicValues::MV_GMF2;
      case MV_RMHG:
        return MagicValues::MV_RMHG;
      case MV_RSAR:
        return MagicValues::MV_RSAR;
      case MV_RSTM:
        return MagicValues::MV_RSTM;
      case MV_SEST:
        return MagicValues::MV_RSTM;
      case MV_STMD:
        return MagicValues::MV_STMD;
      case MV_STSD:
        return MagicValues::MV_STSD;
      case MV_STRI____:
        file.seekg(-4, std::ios::cur);
        int64_t magic64;
        file.read((char*)&magic64, sizeof(magic64));
        switch (magic64) {
          case MV_STRIMAG2:
            return MagicValues::MV_STRIMAG2;
          case MV_STRIMAGE:
            return MagicValues::MV_STRIMAGE;
          default:
            return MagicValues::MV_UNKNOWN;
        }
      case MV_THP:
        return MagicValues::MV_THP;
      default:
        return MV_UNKNOWN;
    }
  }

  static String get_magic_str(const String& filepath) {
    const char* path = filepath.utf8().get_data();

    String str;

    std::ifstream file(path, std::ios::binary);
    if (!file) {
      UtilityFunctions::push_error("Unable to open.");
      return str;
    }

    switch (get_magic(file)) {
      case MV_FLCG:
        str += "FLCG";
        break;
      case MV_GAN2:
        str += "GAN2";
        break;
      case MV_CGT0:
        str += "CGT0";
        break;
      case MV_GMF2:
        str += "GMF2";
        break;
      case MV_RMHG:
        str += "RMHG";
        break;
      case MV_RSAR:
        str += "RSAR";
        break;
      case MV_RSTM:
        str += "RSTM";
        break;
      case MV_SEST:
        str += "RSTM";
        break;
      case MV_STMD:
        str += "STMD";
        break;
      case MV_STSD:
        str += "STSD";
        break;
      case MV_STRIMAG2:
        str += "STRIMAG2";
        break;
      case MV_STRIMAGE:
        str += "STRIMAGE";
        break;
      case MV_THP:
        str += "THP";
        break;
      default:
        str += "";
        break;
    }
    return str;
  }
};
}  // namespace godot
#endif  // MAGIC