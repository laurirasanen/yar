#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace yar
{
bool                               fs_exists(const char* path);
std::string                        fs_read_text(const char* path);
std::vector<std::filesystem::path> fs_iter(const char* path, const char* ext, bool recursive);
} // namespace yar
