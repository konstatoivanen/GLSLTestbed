#include "PrecompiledHeader.h"
#include "Builders.h"

void PK::ECS::Builders::InitializeLightValues(LightImplementer* implementer, const color& color, LightType lightType, LightCookie cookie, bool castShadows, float angle, float radius)
{
	const auto intensityThreshold = 0.2f;
	const auto sphereTranslucency = 0.1f;
	auto lightColor = glm::exp(color);

	float3 gammaColor = glm::pow(float3(lightColor.rgb), float3(1.0f / 2.2f));
	float intensity = glm::compMax(gammaColor);
	auto autoRadius = radius < 0.0f ? intensity / intensityThreshold : radius;

	switch (lightType)
	{
		case LightType::Directional:
			implementer->isCullable = false;
			implementer->localAABB = Functions::CreateBoundsCenterExtents(CG_FLOAT3_ZERO, CG_FLOAT3_ONE); 
			break;
		case LightType::Point: 
			implementer->localAABB = Functions::CreateBoundsCenterExtents(CG_FLOAT3_ZERO, CG_FLOAT3_ONE * autoRadius); 
			implementer->isCullable = true;
			break;
		case LightType::Spot:
			auto a = autoRadius * glm::tan(angle * 0.5f * CG_FLOAT_DEG2RAD);
			implementer->localAABB = Functions::CreateBoundsCenterExtents({ 0.0f, 0.0f, autoRadius * 0.5f }, { a, a, autoRadius * 0.5f });
			implementer->isCullable = true;
			break;
	}

	implementer->isVisible = false;
	implementer->color = lightColor;
	implementer->radius = autoRadius;
	implementer->castShadows = castShadows;
	implementer->flags = Components::RenderHandleFlags::Light;
	implementer->cookie = cookie;
	implementer->lightType = lightType;
	implementer->angle = angle;
}
