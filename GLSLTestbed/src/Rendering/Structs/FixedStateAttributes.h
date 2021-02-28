#pragma once
#include <GLAD\glad.h>

struct BlendMode
{
	GLenum Source;
	GLenum Destination;
};

inline bool operator==(const BlendMode& lhs, const BlendMode& rhs) { return lhs.Source == rhs.Source && lhs.Destination == rhs.Destination; }
inline bool operator!=(const BlendMode& lhs, const BlendMode& rhs) { return lhs.Source != rhs.Source || lhs.Destination != rhs.Destination; }

struct FixedStateAttributes
{
	bool ZWriteEnabled;
	bool ZTestEnabled;
	bool BlendEnabled;
	bool CullEnabled;
	GLenum ZTest;
	GLenum CullMode;
	BlendMode Blend;
	unsigned char ColorMask;
};