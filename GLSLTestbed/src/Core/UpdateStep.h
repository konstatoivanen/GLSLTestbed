#pragma once

namespace PK::Core
{
	enum class UpdateStep
	{
		OpenFrame,
		UpdateInput,
		UpdateEngines,
		PreRender,
		Render,
		PostRender,
		CloseFrame,
	};
}