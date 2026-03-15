#include <filesystem>
#include <fstream>
#include <iterator>

#include "fs.h"

namespace yar
{
bool fs_exists(const char* path)
{
    std::filesystem::path p = {path};
    return std::filesystem::exists(path);
}

std::string fs_read_text(const char* path)
{
    std::ifstream ifs(path);
    return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

std::vector<std::filesystem::path> fs_iter(const char* path, const char* ext, bool recursive)
{
    std::filesystem::path folder(path);
    if (!std::filesystem::is_directory(folder))
    {
        throw std::runtime_error(std::format("path {} is not a directory", path));
    }

    std::vector<std::filesystem::path> filepaths;

    for (const auto& entry : std::filesystem::directory_iterator(folder))
    {
        if (entry.is_regular_file() && entry.path().extension() == ext)
        {
            filepaths.push_back(entry);
        }
        else if (recursive && entry.is_directory())
        {
            filepaths.append_range(fs_iter(entry.path().c_str(), ext, true));
        }
    }

    return filepaths;
}
} // namespace yar
