#pragma once
#include <GLAD\glad.h>

namespace PK::Rendering::Structs
{
	struct BlendMode
	{
		GLenum Source = GL_ZERO;
		GLenum Destination = GL_ONE;
	};
	
	inline bool operator==(const BlendMode& lhs, const BlendMode& rhs) { return lhs.Source == rhs.Source && lhs.Destination == rhs.Destination; }
	inline bool operator!=(const BlendMode& lhs, const BlendMode& rhs) { return lhs.Source != rhs.Source || lhs.Destination != rhs.Destination; }
	
	struct FixedStateAttributes
	{
		bool ZWriteEnabled = false;
		bool ZTestEnabled = false;
		bool BlendEnabled = false;
		bool CullEnabled = false;
		GLenum ZTest = GL_FALSE;
		GLenum CullMode = GL_FALSE;
		BlendMode Blend;
		unsigned char ColorMask = 0;
	};
}