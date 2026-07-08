#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace yar
{
bool fs_exists(const char* path);
bool fs_exists(const std::filesystem::path path);

std::string fs_read_text(const char* path);
std::string fs_read_text(const std::filesystem::path path);

std::vector<uint8_t> fs_read_data(const char* path);
std::vector<uint8_t> fs_read_data(const std::filesystem::path path);

std::vector<std::filesystem::path> fs_iter(const char* path, const char* ext, bool recursive);
std::filesystem::path              fs_program_root();

std::filesystem::path fs_relative_path(const char* path);
std::filesystem::path fs_relative_path(const std::filesystem::path path);

std::filesystem::path fs_append(const std::filesystem::path path, const char* append);
} // namespace yar
