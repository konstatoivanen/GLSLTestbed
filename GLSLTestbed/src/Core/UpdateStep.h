#pragma once

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