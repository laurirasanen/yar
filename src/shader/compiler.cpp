#include <cstring>

#include "../log.h"
#include "../platform/fs.h"
#include "../util.h"
#include "compiler.h"

namespace yar
{
ShaderCompiler::ShaderCompiler()
{
    createGlobalSession(m_globalSession.writeRef());

    slang::TargetDesc targetDesc = {};
    targetDesc.format            = SLANG_SPIRV;
    targetDesc.profile           = m_globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets            = &targetDesc;
    sessionDesc.targetCount        = 1;

    std::array<slang::CompilerOptionEntry, 4> options = {
        slang::CompilerOptionEntry(
            slang::CompilerOptionName::Optimization,
            slang::CompilerOptionValue(
                slang::CompilerOptionValueKind::Int,
#if DEBUG
                SLANG_OPTIMIZATION_LEVEL_NONE,
#else
                SLANG_OPTIMIZATION_LEVEL_HIGH,
#endif
                0,
                nullptr,
                nullptr
            )
        ),
        slang::CompilerOptionEntry(
            slang::CompilerOptionName::EmitSpirvMethod,
            slang::CompilerOptionValue(
                slang::CompilerOptionValueKind::Int,
                SlangEmitSpirvMethod::SLANG_EMIT_SPIRV_DIRECTLY,
                0,
                nullptr,
                nullptr
            )
        ),
        // TODO: validate is broken
        slang::CompilerOptionEntry(
            slang::CompilerOptionName::ValidateIr,
            slang::CompilerOptionValue(slang::CompilerOptionValueKind::Int, 0, 0, nullptr, nullptr)
        ),
        slang::CompilerOptionEntry(
            slang::CompilerOptionName::WarningsAsErrors,
            slang::CompilerOptionValue(slang::CompilerOptionValueKind::String, 0, 0, "all", nullptr)
        )
    };
    sessionDesc.compilerOptionEntries    = options.data();
    sessionDesc.compilerOptionEntryCount = options.size();

    m_globalSession->createSession(sessionDesc, m_session.writeRef());

    auto filepaths = fs_iter("assets/shaders", ".slang", true);
    for (const auto& path : filepaths)
    {
        const std::string source = fs_read_text(path.c_str());
        LoadModule(path.filename(), path.string(), source);
    }

    QueryEntries();
    Compose();
    Link();
    Compile();
}

ShaderCompiler::~ShaderCompiler()
{
}

const void* ShaderCompiler::GetSpirv(std::string moduleName, std::string entryName, size_t& size)
{
    for (const auto& mod : m_modules)
    {
        if (mod.name != moduleName)
        {
            continue;
        }

        for (const auto& program : mod.programs)
        {
            if (program.name != entryName)
            {
                continue;
            }

            if (!program.spirv)
            {
                LOG_ERROR("Found {} {} but no spirv", moduleName, entryName);
                return nullptr;
            }

            size = program.spirv->getBufferSize();
            return program.spirv->getBufferPointer();
        }
    }

    LOG_ERROR("Program {} {} not found", moduleName, entryName);
    return nullptr;
}

bool ShaderCompiler::LoadModule(
    const std::string      name,
    const std::string_view path,
    const std::string_view source
)
{
    ShaderModule& mod = m_modules.emplace_back();
    mod.name          = name;
    mod.slangModule   = m_session->loadModuleFromSourceString(
        name.c_str(),
        path.data(),
        source.data(),
        mod.diagnostics.writeRef()
    );
    PrintDiagnostics(mod.diagnostics);

    if (!mod.slangModule)
    {
        LOG_ERROR("Failed to load module {}", name);
        return false;
    }

    LOG_DEBUG("Loaded module {}", name);
    return true;
}

void ShaderCompiler::QueryEntries()
{
    const char* entries[] = {SHADER_ENTRY_VERTEX, SHADER_ENTRY_PIXEL, SHADER_ENTRY_COMPUTE};
    static_assert(ARRAY_SIZE(entries) == SHADER_ENTRY_COUNT);
    for (auto& mod : m_modules)
    {
        if (!mod.slangModule)
        {
            continue;
        }

        for (const auto& entry : entries)
        {
            Slang::ComPtr<slang::IEntryPoint> entryPoint;
            auto result = mod.slangModule->findEntryPointByName(entry, entryPoint.writeRef());
            if (result != SLANG_OK)
            {
                continue;
            }
            LOG_DEBUG("Found entry point {} in {}", entry, mod.name);
            auto& program = mod.programs.emplace_back();
            program.name  = entry;
            program.entry = entryPoint;
        }
    }
}

void ShaderCompiler::Compose()
{
    for (auto& mod : m_modules)
    {
        for (auto& program : mod.programs)
        {
            Slang::ComPtr<slang::IComponentType>  composedProgram;
            std::array<slang::IComponentType*, 2> componentTypes = {mod.slangModule, program.entry};
            auto                                  result = m_session->createCompositeComponentType(
                componentTypes.data(),
                componentTypes.size(),
                composedProgram.writeRef(),
                mod.diagnostics.writeRef()
            );
            PrintDiagnostics(mod.diagnostics);
            if (result != SLANG_OK)
            {
                LOG_ERROR("Failed to compose {} program {}", mod.name, program.name);
                continue;
            }
            LOG_DEBUG("Composed {} program {}", mod.name, program.name);
            program.composed = composedProgram;
        }
    }
}

void ShaderCompiler::Link()
{
    for (auto& mod : m_modules)
    {
        for (auto& program : mod.programs)
        {
            Slang::ComPtr<slang::IComponentType> linkedProgram;
            auto                                 result =
                program.composed->link(linkedProgram.writeRef(), mod.diagnostics.writeRef());
            PrintDiagnostics(mod.diagnostics);
            if (result != SLANG_OK)
            {
                LOG_ERROR("Failed to link {} program {}", mod.name, program.name);
                continue;
            }
            LOG_DEBUG("Linked {} program {}", mod.name, program.name);
            program.linked = linkedProgram;
        }
    }
}

void ShaderCompiler::Compile()
{
    for (auto& mod : m_modules)
    {
        for (auto& program : mod.programs)
        {
            Slang::ComPtr<slang::IBlob> spirvCode;
            SlangResult                 result = program.linked->getEntryPointCode(
                0, // entryPointIndex
                0, // targetIndex
                spirvCode.writeRef(),
                mod.diagnostics.writeRef()
            );
            PrintDiagnostics(mod.diagnostics);
            if (result != SLANG_OK)
            {
                LOG_ERROR("Failed to compile {} program {}", mod.name, program.name);
                continue;
            }
            auto data = static_cast<const uint32_t*>(spirvCode->getBufferPointer());
            if (spirvCode->getBufferSize() < 4 || data[0] != 0x07230203)
            {
                LOG_ERROR("Invalid spirv for {} program {}", mod.name, program.name);
            }
            LOG_DEBUG("Compiled {} program {}", mod.name, program.name);
            program.spirv = spirvCode;
        }
    }
}

void ShaderCompiler::PrintDiagnostics(Slang::ComPtr<slang::IBlob> diagnostics)
{
    if (diagnostics != nullptr)
    {
        LOG_DEBUG("slang: {}", static_cast<const char*>(diagnostics->getBufferPointer()));
    }
}
} // namespace yar
