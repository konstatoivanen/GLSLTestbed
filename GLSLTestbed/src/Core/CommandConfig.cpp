#include "PrecompiledHeader.h"
#include "CommandConfig.h"

namespace PK::Core
{
	CommandConfig::CommandConfig() { values = { &Commands }; }

	template<>
	bool AssetImporters::IsValidExtension<CommandConfig>(const std::filesystem::path& extension) { return extension.compare(".keycfg") == 0; }
	
	template<>
	void AssetImporters::Import(const std::string& filepath, Ref<CommandConfig>& config) { config->Load(filepath); }
}