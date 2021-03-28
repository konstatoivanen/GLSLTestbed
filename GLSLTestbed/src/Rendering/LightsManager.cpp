#include "PrecompiledHeader.h"
#include "LightsManager.h"
#include "Rendering/Graphics.h"
#include "Utilities/HashCache.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"

PK::Rendering::LightsManager::LightsManager(AssetDatabase* assetDatabase)
{
	m_passKeywords[0] = StringHashID::StringToID("PASS_CLUSTERS");
	m_passKeywords[1] = StringHashID::StringToID("PASS_DISPATCH");

	m_computeLightAssignment = assetDatabase->Find<Shader>("CS_ClusteredLightAssignment");
	m_computeDepthReset = assetDatabase->Find<Shader>("CS_ClusteredDepthReset");
	m_computeDepthTiles = assetDatabase->Find<Shader>("CS_ClusteredDepthMinMax");
	m_computeCullClusters = assetDatabase->Find<Shader>("CS_ClusteredCullClusters");

	m_depthTiles = CreateRef<ComputeBuffer>(BufferLayout(
	{
		{CG_TYPE::INT2, "DEPTHMINMAX"},
	}), GridSizeX * GridSizeY * GridSizeZ, GL_STATIC_COPY);

	m_lightsBuffer = CreateRef<ComputeBuffer>(BufferLayout(
	{
		{CG_TYPE::FLOAT4, "COLOR"},
		{CG_TYPE::FLOAT4, "DIRECTION"}
	}), 32);

	m_VisibleClusterList = CreateRef<ComputeBuffer>(BufferLayout(
	{
		{CG_TYPE::INT, "INDEX"},
	}), ClusterCount, GL_STATIC_COPY);

	m_clusterDispatchInfo = CreateRef<ComputeBuffer>(BufferLayout(
	{
		{CG_TYPE::INT, "DISPATCH_ARGUMENTS", 5}
	}), 1);

	m_globalLightsList = CreateRef<ComputeBuffer>(BufferLayout(
	{
		{CG_TYPE::INT, "INDEX" }
	}), ClusterCount * MaxLightsPerTile, GL_STATIC_COPY);

	m_lightTiles = CreateRef<ComputeBuffer>(BufferLayout(
	{
		{CG_TYPE::INT, "INDEX"},
	}), ClusterCount, GL_STATIC_COPY);
}

void PK::Rendering::LightsManager::Update(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights, const uint2& resolution, float znear, float zfar)
{
	m_lightsBuffer->ValidateSize((uint)visibleLights.count + 1);
	auto buffer = m_lightsBuffer->BeginMapBuffer<Structs::PKPointLight>();

	for (size_t i = 0; i < visibleLights.count; ++i)
	{
		auto* view = entityDb->Query<ECS::EntityViews::PointLightRenderable>(ECS::EGID(visibleLights[i], (uint)ECS::ENTITY_GROUPS::ACTIVE));
		buffer[i] = { view->pointLight->color, float4(view->transform->position, view->pointLight->radius) };
	}

	buffer[visibleLights.count] = { CG_COLOR_CLEAR, CG_FLOAT4_ZERO };
	m_lightsBuffer->EndMapBuffer();

	float frustuminfo[5] = 
	{ 
		(float)GridSizeZ / glm::log2(zfar / znear),
		-(float)GridSizeZ * glm::log2(znear) / log2(zfar / znear), 
		std::ceilf(resolution.x / (float)GridSizeX), 
		znear,
		zfar / znear 
	};

	auto hashCache = HashCache::Get();

	GraphicsAPI::SetGlobalInt(hashCache->pk_LightCount, (int)visibleLights.count);
	GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_Lights, m_lightsBuffer->GetGraphicsID());
	GraphicsAPI::SetGlobalFloat(hashCache->pk_ClusterFrustumInfo, frustuminfo, 5);
	GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_GlobalLightsList, m_globalLightsList->GetGraphicsID());
	GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_LightTiles, m_lightTiles->GetGraphicsID());
	m_properties.SetComputeBuffer(hashCache->pk_FDepthRanges, m_depthTiles->GetGraphicsID());
	m_properties.SetComputeBuffer(hashCache->pk_ClusterDispatchInfo, m_clusterDispatchInfo->GetGraphicsID());
	m_properties.SetComputeBuffer(hashCache->pk_VisibleClusters, m_VisibleClusterList->GetGraphicsID());

	uint zero[5] = { 0, 0, 0, 0, 0 };
	m_clusterDispatchInfo->SubmitData(&zero, 0, CG_TYPE_SIZE_INT * 5);
}

void PK::Rendering::LightsManager::UpdateLightTiles(const uint2& resolution)
{
	auto depthCountX = (uint)std::ceilf(resolution.x / 16.0f);
	auto depthCountY = (uint)std::ceilf(resolution.y / 16.0f);

	GraphicsAPI::DispatchCompute(m_computeDepthReset.lock(), { 1,1, GridSizeZ / 4 }, m_properties);
	GraphicsAPI::DispatchCompute(m_computeDepthTiles.lock(), { depthCountX, depthCountY, 1 }, m_properties);
	
	m_properties.SetKeywords({ m_passKeywords[0] });
	GraphicsAPI::DispatchCompute(m_computeCullClusters.lock(), { GridSizeX, GridSizeY, GridSizeZ }, m_properties);

	m_properties.SetKeywords({ m_passKeywords[1] });
	GraphicsAPI::DispatchCompute(m_computeCullClusters.lock(), { 1, 1, 1}, m_properties);
	
	GraphicsAPI::DispatchComputeIndirect(m_computeLightAssignment.lock(), m_clusterDispatchInfo->GetGraphicsID(), 0, m_properties);
}
