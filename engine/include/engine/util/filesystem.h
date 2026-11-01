#pragma once
#include <string>
#include <vector>

namespace Util {

std::string
get_exe_dir();
std::vector<unsigned char>
read_file_bytes(const std::string& relative_path);

std::string
read_file(const std::string& relative_path);

}
