#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Ref.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Core/UpdateStep.h"
#include "Core/AssetDataBase.h"
#include "Core/EntityDatabase.h"
#include "Rendering/GizmoRenderer.h"

class DebugEngine : public IService, public PKECS::ISimpleStep, public PKECS::IStep<Input>, public PKECS::IStep<GizmoRenderer>
{
	public:
		DebugEngine(AssetDatabase* assetDatabase, Time* time, PKECS::EntityDatabase* entityDb);
		~DebugEngine();
		void Step(Input* input) override;
		void Step(int condition) override;
		void Step(GizmoRenderer* gizmos) override;

	private:
		PKECS::EntityDatabase* m_entityDb;
		AssetDatabase* m_assetDatabase;
		Time* m_time;
	
		Weak<Material> materialMetal;
		Weak<Material> materialGravel;
		Weak<Mesh> cornellBox;
		Weak<Material> cornellBoxMaterial;
};