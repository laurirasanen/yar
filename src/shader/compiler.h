#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"

#define SHADER_ENTRY_VERTEX  "vertex"
#define SHADER_ENTRY_PIXEL   "pixel"
#define SHADER_ENTRY_COMPUTE "compute"
#define SHADER_ENTRY_COUNT   3

namespace yar
{
struct ShaderProgram
{
    std::string                          name;
    Slang::ComPtr<slang::IEntryPoint>    entry;
    Slang::ComPtr<slang::IComponentType> composed;
    Slang::ComPtr<slang::IComponentType> linked;
    Slang::ComPtr<slang::IBlob>          spirv;
};

struct ShaderModule
{
    std::string                   name;
    Slang::ComPtr<slang::IBlob>   diagnostics;
    Slang::ComPtr<slang::IModule> slangModule;
    std::vector<ShaderProgram>    programs;
};

class ShaderCompiler
{
  public:
    ShaderCompiler();
    ~ShaderCompiler();

    ShaderCompiler(const ShaderCompiler&)            = delete;
    ShaderCompiler(ShaderCompiler&&)                 = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(ShaderCompiler&&)      = delete;

    const void* GetSpirv(std::string name, std::string entryName, size_t& size);

  private:
    bool LoadModule(
        const std::string      name,
        const std::string_view path,
        const std::string_view source
    );
    void QueryEntries();
    void Compose();
    void Link();
    void Compile();

    void PrintDiagnostics(Slang::ComPtr<slang::IBlob> diagnostics);

    Slang::ComPtr<slang::IGlobalSession> m_globalSession;
    Slang::ComPtr<slang::ISession>       m_session;
    std::vector<ShaderModule>            m_modules;
};
} // namespace yar
