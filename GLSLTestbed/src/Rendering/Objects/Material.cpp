#include "PrecompiledHeader.h"
#include "Rendering/Objects/Material.h"
#include "Rendering/Objects/TextureXD.h"
#include "Core/Application.h"
#include <yaml-cpp/yaml.h>

namespace YAML 
{
	template<>
	struct convert<float2>
	{
		static Node encode(const float2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, float2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<float3>
	{
		static Node encode(const float3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, float3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<float4>
	{
		static Node encode(const float4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, float4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<float2x2>
	{
		static Node encode(const float2x2& rhs)
		{
			Node node;
			node.push_back(rhs[0].x);
			node.push_back(rhs[0].y);

			node.push_back(rhs[1].x);
			node.push_back(rhs[1].y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, float2x2& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs[0].x = node[0].as<float>();
			rhs[0].y = node[1].as<float>();

			rhs[1].x = node[2].as<float>();
			rhs[1].y = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<float3x3>
	{
		static Node encode(const float3x3& rhs)
		{
			Node node;
			node.push_back(rhs[0].x);
			node.push_back(rhs[0].y);
			node.push_back(rhs[0].z);

			node.push_back(rhs[1].x);
			node.push_back(rhs[1].y);
			node.push_back(rhs[1].z);

			node.push_back(rhs[2].x);
			node.push_back(rhs[2].y);
			node.push_back(rhs[2].z);

			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, float3x3& rhs)
		{
			if (!node.IsSequence() || node.size() != 9)
			{
				return false;
			}

			rhs[0].x = node[0].as<float>();
			rhs[0].y = node[1].as<float>();
			rhs[0].z = node[2].as<float>();

			rhs[1].x = node[3].as<float>();
			rhs[1].y = node[4].as<float>();
			rhs[1].z = node[5].as<float>();

			rhs[2].x = node[6].as<float>();
			rhs[2].y = node[7].as<float>();
			rhs[2].z = node[8].as<float>();
			return true;
		}
	};

	template<>
	struct convert<float4x4>
	{
		static Node encode(const float4x4& rhs)
		{
			Node node;
			node.push_back(rhs[0].x);
			node.push_back(rhs[0].y);
			node.push_back(rhs[0].z);
			node.push_back(rhs[0].w);

			node.push_back(rhs[1].x);
			node.push_back(rhs[1].y);
			node.push_back(rhs[1].z);
			node.push_back(rhs[1].w);

			node.push_back(rhs[2].x);
			node.push_back(rhs[2].y);
			node.push_back(rhs[2].z);
			node.push_back(rhs[2].w);

			node.push_back(rhs[3].x);
			node.push_back(rhs[3].y);
			node.push_back(rhs[3].z);
			node.push_back(rhs[3].w);

			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, float4x4& rhs)
		{
			if (!node.IsSequence() || node.size() != 16)
			{
				return false;
			}

			rhs[0].x = node[0].as<float>();
			rhs[0].y = node[1].as<float>();
			rhs[0].z = node[2].as<float>();
			rhs[0].w = node[3].as<float>();

			rhs[1].x = node[4].as<float>();
			rhs[1].y = node[5].as<float>();
			rhs[1].z = node[6].as<float>();
			rhs[1].w = node[7].as<float>();

			rhs[2].x = node[8].as<float>();
			rhs[2].y = node[9].as<float>();
			rhs[2].z = node[10].as<float>();
			rhs[2].w = node[11].as<float>();

			rhs[3].x = node[12].as<float>();
			rhs[3].y = node[13].as<float>();
			rhs[3].z = node[14].as<float>();
			rhs[3].w = node[15].as<float>();
			return true;
		}
	};

	template<>
	struct convert<int2>
	{
		static Node encode(const int2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, int2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node[0].as<int>();
			rhs.y = node[1].as<int>();
			return true;
		}
	};

	template<>
	struct convert<int3>
	{
		static Node encode(const int3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, int3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<int>();
			rhs.y = node[1].as<int>();
			rhs.z = node[2].as<int>();
			return true;
		}
	};

	template<>
	struct convert<int4>
	{
		static Node encode(const int4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, int4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<int>();
			rhs.y = node[1].as<int>();
			rhs.z = node[2].as<int>();
			rhs.w = node[3].as<int>();
			return true;
		}
	};

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
			auto typeIdx = CGConvert::FromString(typeName.c_str());
			auto values = property.second["Value"];

			switch (typeIdx)
			{
				case CG_TYPE_FLOAT: material->SetFloat(nameHash, values.as<float>()); break;
				case CG_TYPE_FLOAT2: material->SetFloat2(nameHash, values.as<float2>()); break;
				case CG_TYPE_FLOAT3: material->SetFloat3(nameHash, values.as<float3>()); break;
				case CG_TYPE_FLOAT4: material->SetFloat4(nameHash, values.as<float4>()); break;
				case CG_TYPE_FLOAT2X2: material->SetFloat2x2(nameHash, values.as<float2x2>()); break;
				case CG_TYPE_FLOAT3X3: material->SetFloat3x3(nameHash, values.as<float3x3>()); break;
				case CG_TYPE_FLOAT4X4: material->SetFloat4x4(nameHash, values.as<float4x4>()); break;
				case CG_TYPE_INT: material->SetInt(nameHash, values.as<int>()); break;
				case CG_TYPE_INT2: material->SetInt2(nameHash, values.as<int2>()); break;
				case CG_TYPE_INT3: material->SetInt3(nameHash, values.as<int3>()); break;
				case CG_TYPE_INT4: material->SetInt4(nameHash, values.as<int4>()); break;
				case CG_TYPE_TEXTURE: material->SetTexture(nameHash, values.as<Weak<TextureXD>>().lock()->GetGraphicsID()); break;
			}
		}
	}
}