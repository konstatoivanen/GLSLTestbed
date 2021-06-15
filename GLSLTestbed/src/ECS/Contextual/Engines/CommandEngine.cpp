#include "PrecompiledHeader.h"
#include "CommandEngine.h"
#include "Rendering/GraphicsAPI.h"
#include "Utilities/StringUtilities.h"
#include "Rendering/Objects/TextureXD.h"

namespace PK::ECS::Engines
{
    const std::unordered_map<std::string, CommandArgument> CommandEngine::ArgumentMap =
    {
        {std::string("query"),      CommandArgument::Query},
        {std::string("reload"),     CommandArgument::Reload},
        {std::string("assets"),     CommandArgument::Assets},
        {std::string("variants"),   CommandArgument::Variants},
        {std::string("uniforms"),   CommandArgument::Uniforms},
        {std::string("gpu_memory"), CommandArgument::GPUMemory},
        {std::string("shader"),     CommandArgument::TypeShader},
        {std::string("mesh"),       CommandArgument::TypeMesh},
        {std::string("texture"),    CommandArgument::TypeTexture},
        {std::string("material"),   CommandArgument::TypeMaterial},
    };

    void CommandEngine::QueryShaderVariants(std::vector<std::string> arguments)
    {
        auto shader = m_assetDatabase->TryFind<Shader>(arguments[2].c_str());

        if (shader != nullptr)
        {
            shader->ListVariants();
        }
        else
        {
            PK::Utilities::Debug::InsertNewLine();
            PK_CORE_LOG_WARNING("Could not find shader with keyword: %s", arguments[2].c_str());
            PK::Utilities::Debug::InsertNewLine();
        }
    }

    void CommandEngine::QueryShaderUniforms(std::vector<std::string> arguments)
    {
        auto shader = m_assetDatabase->TryFind<Shader>(arguments[2].c_str());

        if (shader != nullptr)
        {
            shader->ListProperties();
        }
        else
        {
            PK::Utilities::Debug::InsertNewLine();
            PK_CORE_LOG_WARNING("Could not find shader with keyword: %s", arguments[2].c_str());
            PK::Utilities::Debug::InsertNewLine();
        }
    }

    void CommandEngine::QueryGPUMemory(std::vector<std::string> arguments)
    {
        PK_CORE_LOG("GPU Memory usage in kb: %i", Rendering::GraphicsAPI::GetMemoryUsageKB());
    }

    void CommandEngine::ReloadShaders(std::vector<std::string> arguments)
    {
        m_assetDatabase->ReloadDirectory<Shader>(arguments[2].c_str());
        PK_CORE_LOG("Reimported shaders in folder: %s", arguments[2].c_str());
    }

    void CommandEngine::ReloadMaterials(std::vector<std::string> arguments)
    {
        m_assetDatabase->ReloadDirectory<Material>(arguments[2].c_str());
        PK_CORE_LOG("Reimported shaders in folder: %s", arguments[2].c_str());
    }

    void CommandEngine::ReloadTextures(std::vector<std::string> arguments)
    {
        m_assetDatabase->ReloadDirectory<TextureXD>(arguments[2].c_str());
        PK_CORE_LOG("Reimported shaders in folder: %s", arguments[2].c_str());
    }

    void CommandEngine::ReloadMeshes(std::vector<std::string> arguments)
    {
        m_assetDatabase->ReloadDirectory<Mesh>(arguments[2].c_str());
        PK_CORE_LOG("Reimported shaders in folder: %s", arguments[2].c_str());
    }

    void CommandEngine::QueryLoadedShaders(std::vector<std::string> arguments) { m_assetDatabase->ListAssetsOfType<Shader>(); }
    void CommandEngine::QueryLoadedMaterials(std::vector<std::string> arguments) { m_assetDatabase->ListAssetsOfType<Material>(); }
    void CommandEngine::QueryLoadedTextures(std::vector<std::string> arguments) { m_assetDatabase->ListAssetsOfType<TextureXD>(); }
    void CommandEngine::QueryLoadedMeshes(std::vector<std::string> arguments) { m_assetDatabase->ListAssetsOfType<Mesh>(); }
    void CommandEngine::QueryLoadedAssets(std::vector<std::string> arguments) { m_assetDatabase->ListAssets(); }

    CommandEngine::CommandEngine(AssetDatabase* assetDatabase, Time* time, EntityDatabase* entityDb)
    {
        m_entityDb = entityDb;
        m_assetDatabase = assetDatabase;
        m_time = time;

        m_commands[{CommandArgument::Query, CommandArgument::TypeShader, CommandArgument::StringParameter, CommandArgument::Variants}] = PK_BIND_FUNCTION(QueryShaderVariants);
        m_commands[{CommandArgument::Query, CommandArgument::TypeShader, CommandArgument::StringParameter, CommandArgument::Uniforms}] = PK_BIND_FUNCTION(QueryShaderUniforms);
        m_commands[{CommandArgument::Query, CommandArgument::GPUMemory}] = PK_BIND_FUNCTION(QueryGPUMemory);
        m_commands[{CommandArgument::Query, CommandArgument::Assets, CommandArgument::TypeShader}] = PK_BIND_FUNCTION(QueryLoadedShaders);
        m_commands[{CommandArgument::Query, CommandArgument::Assets, CommandArgument::TypeMaterial}] = PK_BIND_FUNCTION(QueryLoadedMaterials);
        m_commands[{CommandArgument::Query, CommandArgument::Assets, CommandArgument::TypeMesh}] = PK_BIND_FUNCTION(QueryLoadedMeshes);
        m_commands[{CommandArgument::Query, CommandArgument::Assets, CommandArgument::TypeTexture}] = PK_BIND_FUNCTION(QueryLoadedTextures);
        m_commands[{CommandArgument::Query, CommandArgument::Assets}] = PK_BIND_FUNCTION(QueryLoadedAssets);
        m_commands[{CommandArgument::Reload, CommandArgument::TypeShader, CommandArgument::StringParameter}] = PK_BIND_FUNCTION(ReloadShaders);
        m_commands[{CommandArgument::Reload, CommandArgument::TypeMesh, CommandArgument::StringParameter}] = PK_BIND_FUNCTION(ReloadMeshes);
        m_commands[{CommandArgument::Reload, CommandArgument::TypeMaterial, CommandArgument::StringParameter}] = PK_BIND_FUNCTION(ReloadMaterials);
        m_commands[{CommandArgument::Reload, CommandArgument::TypeTexture, CommandArgument::StringParameter}] = PK_BIND_FUNCTION(ReloadTextures);
    }
    
    void CommandEngine::Step(Input* input)
    {
        if (!input->GetKeyDown(KeyCode::TAB))
        {
            return;
        }

        PK::Utilities::Debug::InsertNewLine();
        PK::Utilities::Debug::PKLog((int)PK::Utilities::Debug::ConsoleColor::LOG_INPUT, "Awaiting command input...");

        std::string argument;
        std::string command;
        std::getline(std::cin, command);
        std::istringstream iss(command);
        std::vector<CommandArgument> commandArguments;
        std::vector<std::string> arguments;

        while (iss >> argument)
        {
            arguments.push_back(argument);

            if (ArgumentMap.count(argument))
            {
                commandArguments.push_back(ArgumentMap.at(argument));
            }
            else
            {
                commandArguments.push_back(CommandArgument::StringParameter);
            }
        }

        if (commandArguments.empty())
        {
            return;
        }

        if (m_commands.count(commandArguments))
        {
            m_commands.at(commandArguments)(arguments);
        }
        else
        {
            PK::Utilities::Debug::InsertNewLine();
            PK_CORE_LOG_WARNING("Unknown command!");
            PK::Utilities::Debug::InsertNewLine();
        }
    }
}