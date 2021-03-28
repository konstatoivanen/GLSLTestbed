#include "PrecompiledHeader.h"
#include "Rendering/Objects/Material.h"
#include "Rendering/Objects/TextureXD.h"
#include "Core/Application.h"
#include <yaml-cpp/yaml.h>

namespace YAML 
{
	using namespace PK::Utilities;
	using namespace PK::Rendering::Objects;
	using namespace PK::Math;

	#define DECLARE_VECTOR_CONVERTER(type, count)				\
	template<>													\
	struct convert<type##count>									\
	{															\
		static Node encode(const type##count & rhs)				\
		{														\
			Node node;											\
			for (auto i = 0; i < count; ++i)					\
			{													\
				node.push_back(rhs[i]);							\
			}													\
			node.SetStyle(EmitterStyle::Flow);					\
			return node;										\
		}														\
																\
		static bool decode(const Node& node, type##count & rhs)	\
		{														\
			if (!node.IsSequence() || node.size() != count)		\
			{													\
				return false;									\
			}													\
																\
			for (auto i = 0; i < count; ++i)					\
			{													\
				rhs[i] = node[i].as<type>();					\
			}													\
																\
			return true;										\
		}														\
	};															\

	#define DECLARE_MATRIX_CONVERTER(type, count)							\
	template<>																\
	struct convert<type##count##x##count>									\
	{																		\
		static Node encode(const type##count##x##count & rhs)				\
		{																	\
			Node node;														\
			for (auto i = 0; i < count; ++i)								\
			for (auto j = 0; j < count; ++j)								\
			{																\
				node.push_back(rhs[i][j]);									\
			}																\
			node.SetStyle(EmitterStyle::Flow);								\
			return node;													\
		}																	\
																			\
		static bool decode(const Node& node, type##count##x##count & rhs)	\
		{																	\
			if (!node.IsSequence() || node.size() != count * count)			\
			{																\
				return false;												\
			}																\
																			\
			for (auto i = 0; i < count; ++i)								\
			for (auto j = 0; j < count; ++j)								\
			{																\
				rhs[i][j] = node[i * count + j].as<type>();					\
			}																\
																			\
			return true;													\
		}																	\
	};																		\

	DECLARE_VECTOR_CONVERTER(float, 2)
	DECLARE_VECTOR_CONVERTER(float, 3)
	DECLARE_VECTOR_CONVERTER(float, 4)
	DECLARE_VECTOR_CONVERTER(int, 2)
	DECLARE_VECTOR_CONVERTER(int, 3)
	DECLARE_VECTOR_CONVERTER(int, 4)
	DECLARE_VECTOR_CONVERTER(uint, 2)
	DECLARE_VECTOR_CONVERTER(uint, 3)
	DECLARE_VECTOR_CONVERTER(uint, 4)

	DECLARE_MATRIX_CONVERTER(float, 2)
	DECLARE_MATRIX_CONVERTER(float, 3)
	DECLARE_MATRIX_CONVERTER(float, 4)

	#undef DECLARE_VECTOR_CONVERTER
	#undef DECLARE_MATRIX_CONVERTER

	template<>
	struct convert<Weak<TextureXD>>
	{
		static Node encode(const Weak<TextureXD>& rhs)
		{
			Node node;
			node.push_back(rhs.lock()->GetFileName());
			node.SetStyle(EmitterStyle::Default);
			return node;
		}

		static bool decode(const Node& node, Weak<TextureXD>& rhs)
		{
			auto path = node.as<std::string>();
			rhs = Application::GetService<AssetDatabase>()->Load<TextureXD>(path);
			return true;
		}
	};

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
void AssetImporters::Import<Material>(const std::string& filepath, Ref<Material>& material)
{
    material->Clear();

	YAML::Node root = YAML::LoadFile(filepath);

	auto data = root["Material"];
	PK_CORE_ASSERT(data, "Could not locate material (%s) header in file.", filepath.c_str());
	
	auto shaderPathProp = data["Shader"];
	PK_CORE_ASSERT(shaderPathProp, "Material (%s) doesn't define a shader.", filepath.c_str());

	auto shaderPath = shaderPathProp.as<std::string>();
	material->m_shader = Application::GetService<AssetDatabase>()->Load<Shader>(shaderPath);
	material->m_renderQueueIndex = material->m_shader.lock()->GetRenderQueueIndex();

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
				case CG_TYPE::TEXTURE: material->SetTexture(nameHash, values.as<Weak<TextureXD>>().lock()->GetGraphicsID()); break;
			}
		}
	}
}