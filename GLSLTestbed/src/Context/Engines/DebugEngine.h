#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Ref.h"
#include "Utilities/StringHashID.h"
#include "Utilities/Log.h"
#include "Utilities/HashCache.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Core/UpdateStep.h"
#include "Core/AssetDataBase.h"
#include "Core/Application.h"
#include "Rendering/Graphics.h"
#include "Rendering/Objects/Texture2D.h"
#include "Rendering/MeshUtility.h"
#include "Rendering/LightingUtility.h"
#include <math.h>

class DebugEngine : public IService, public PKECS::ISimpleStep, public PKECS::IStep<Input>
{
	public:
		DebugEngine(AssetDatabase* assetDatabase, Time* time);
		~DebugEngine();
		void Step(Input* input) override;
		void Step(int condition) override;

	private:
		AssetDatabase* m_assetDatabase;
		Time* m_time;
	
		Weak<Shader> cubeShader;
		Weak<Shader> iblShader;
		Ref<Mesh> cubeMesh;
		Ref<RenderTexture> renderTarget;
		Weak<Texture2D> reflectionMaps[3];
};