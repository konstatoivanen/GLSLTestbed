#include "PrecompiledHeader.h"
#include "Rendering/Objects/Material.h"
#include "Rendering/Objects/TextureXD.h"
#include "Core/Application.h"
#include "Core/YamlSerializers.h"
#include <yaml-cpp/yaml.h>

namespace YAML 
{
	using namespace PK::Utilities;
	using namespace PK::Rendering::Objects;
	using namespace PK::Math;

	Emitter& operator<<(Emitter& out, const float3& v)
	{
		out << Flow;
		out << BeginSeq << v.x << v.y << v.z << EndSeq;
		return out;
	}

	Emitter& operator<<(Emitter& out, const float4& v)
	{
		out << Flow;
		out << BeginSeq << v.x << v.y << v.z << v.w << EndSeq;
		return out;
	}
}

using namespace PK::Utilities;
using namespace PK::Rendering::Structs;
using namespace PK::Rendering::Objects;
using namespace PK::Math;

template<>
bool AssetImporters::IsValidExtension<Material>(const std::filesystem::path& extension) { return extension.compare(".material") == 0; }

template<>
void AssetImporters::Import(const std::string& filepath, Ref<Material>& material)
{
    material->Clear();

	YAML::Node root = YAML::LoadFile(filepath);

	auto data = root["Material"];
	PK_CORE_ASSERT(data, "Could not locate material (%s) header in file.", filepath.c_str());
	
	auto shaderPathProp = data["Shader"];
	PK_CORE_ASSERT(shaderPathProp, "Material (%s) doesn't define a shader.", filepath.c_str());

	auto shaderPath = shaderPathProp.as<std::string>();
	material->m_shader = Application::GetService<AssetDatabase>()->Load<Shader>(shaderPath);
	material->m_cachedShaderAssetId = material->m_shader->GetAssetID();

	auto keywords = data["Keywords"];
	
	if (keywords)
	{
		for (auto keyword : keywords)
		{
			material->SetKeyword(StringHashID::StringToID(keyword.as<std::string>()), true);
		}
	}

	auto properties = data["Properties"];

	if (properties)
	{
		for (auto property : properties)
		{
			auto propertyName = property.first.as<std::string>();

			auto type = property.second["Type"];

			if (!type)
			{
				continue;
			}

			auto nameHash = StringHashID::StringToID(propertyName);
			auto typeName = type.as<std::string>();
			auto typeIdx = Convert::FromString(typeName.c_str());
			auto values = property.second["Value"];

			switch (typeIdx)
			{
				case CG_TYPE::FLOAT: material->SetFloat(nameHash, values.as<float>()); break;
				case CG_TYPE::FLOAT2: material->SetFloat2(nameHash, values.as<float2>()); break;
				case CG_TYPE::FLOAT3: material->SetFloat3(nameHash, values.as<float3>()); break;
				case CG_TYPE::FLOAT4: material->SetFloat4(nameHash, values.as<float4>()); break;
				case CG_TYPE::FLOAT2X2: material->SetFloat2x2(nameHash, values.as<float2x2>()); break;
				case CG_TYPE::FLOAT3X3: material->SetFloat3x3(nameHash, values.as<float3x3>()); break;
				case CG_TYPE::FLOAT4X4: material->SetFloat4x4(nameHash, values.as<float4x4>()); break;
				case CG_TYPE::INT: material->SetInt(nameHash, values.as<int>()); break;
				case CG_TYPE::INT2: material->SetInt2(nameHash, values.as<int2>()); break;
				case CG_TYPE::INT3: material->SetInt3(nameHash, values.as<int3>()); break;
				case CG_TYPE::INT4: material->SetInt4(nameHash, values.as<int4>()); break;
				case CG_TYPE::TEXTURE: 
				{
					auto texture = values.as<TextureXD*>();
					texture->MakeHandleResident();
					auto handle = texture->GetBindlessHandle();
					material->SetResourceHandle(nameHash, handle); break;
				}
				break;
			}
		}
	}
}
