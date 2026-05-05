#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>

#if LINUX
#include <unistd.h>
#endif

#include "fs.h"

namespace yar
{
bool fs_exists(const char* path)
{
    std::filesystem::path p = {path};
    return fs_exists(path);
}

bool fs_exists(const std::filesystem::path path)
{
    return std::filesystem::exists(path);
}

std::string fs_read_text(const char* path)
{
    std::filesystem::path p = {path};
    return fs_read_text(p);
}

std::string fs_read_text(const std::filesystem::path path)
{
    std::ifstream ifs(path);
    return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

std::vector<std::filesystem::path> fs_iter(const char* path, const char* ext, bool recursive)
{
    std::filesystem::path folder(path);
    if (folder.is_relative())
    {
        folder = fs_program_root().append(path);
    }
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

std::filesystem::path fs_program_root()
{
    static std::filesystem::path root {};
    if (!root.empty())
    {
        return root;
    }

    const size_t sz = 256;
    char         buf[sz];
    std::memset(buf, '\0', sz);

#if LINUX
    ssize_t bytes = readlink("/proc/self/exe", buf, sz);
#elif WIN64
    size_t bytes = GetModuleFileName(NULL, buf, sz);
#else
#error implement me
#endif

    if (bytes <= 0)
    {
        throw std::runtime_error("Couldn't get program path");
    }
    buf[sz - 1] = '\0';

    std::filesystem::path p {buf};
    if (!p.has_parent_path())
    {
        throw std::runtime_error("Couldn't get program parent path");
    }
    root = p.parent_path();
    return root;
}

std::filesystem::path fs_relative_path(const char* path)
{
    auto root = fs_program_root();
    return root.append(path);
}

std::filesystem::path fs_relative_path(const std::filesystem::path path)
{
    return fs_relative_path(path.c_str());
}
} // namespace yar
