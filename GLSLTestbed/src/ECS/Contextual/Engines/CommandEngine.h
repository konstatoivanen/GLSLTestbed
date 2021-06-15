#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Ref.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Core/UpdateStep.h"
#include "Core/AssetDataBase.h"
#include "ECS/EntityDatabase.h"

namespace PK::ECS::Engines
{
	using namespace PK::Utilities;
	using namespace PK::Rendering::Objects;

	enum class CommandArgument : char
	{
		Query,
		Reload,
		Assets,
		StringParameter,
		Variants,
		Uniforms,
		GPUMemory,
		TypeShader,
		TypeMesh,
		TypeTexture,
		TypeMaterial
	};

	class CommandEngine : public IService, public IStep<Input>
	{
		public:
			const static std::unordered_map<std::string, CommandArgument> ArgumentMap;

			CommandEngine(AssetDatabase* assetDatabase, Time* time, EntityDatabase* entityDb);
			void Step(Input* input) override;

		private:
			void QueryShaderVariants(std::vector<std::string> arguments);
			void QueryShaderUniforms(std::vector<std::string> arguments);
			void QueryGPUMemory(std::vector<std::string> arguments);
			void ReloadShaders(std::vector<std::string> arguments);
			void ReloadMaterials(std::vector<std::string> arguments);
			void ReloadTextures(std::vector<std::string> arguments);
			void ReloadMeshes(std::vector<std::string> arguments);
			void QueryLoadedShaders(std::vector<std::string> arguments);
			void QueryLoadedMaterials(std::vector<std::string> arguments);
			void QueryLoadedTextures(std::vector<std::string> arguments);
			void QueryLoadedMeshes(std::vector<std::string> arguments);
			void QueryLoadedAssets(std::vector<std::string> arguments);

			std::map<std::vector<CommandArgument>, std::function<void(const std::vector<std::string>&)>> m_commands;
			EntityDatabase* m_entityDb;
			AssetDatabase* m_assetDatabase;
			Time* m_time;
	};
}