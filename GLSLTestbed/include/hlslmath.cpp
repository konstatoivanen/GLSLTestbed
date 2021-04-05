#include "PrecompiledHeader.h"
#include "hlslmath.h"

namespace PK::Math
{
	ushort Convert::Size(CG_TYPE type)
	{
		switch (type)
		{
			case CG_TYPE::FLOAT: return CG_TYPE_SIZE_FLOAT;
			case CG_TYPE::FLOAT2: return CG_TYPE_SIZE_FLOAT2;
			case CG_TYPE::FLOAT3: return CG_TYPE_SIZE_FLOAT3;
			case CG_TYPE::FLOAT4: return CG_TYPE_SIZE_FLOAT4;
			case CG_TYPE::FLOAT2X2: return CG_TYPE_SIZE_FLOAT2X2;
			case CG_TYPE::FLOAT3X3: return CG_TYPE_SIZE_FLOAT3X3;
			case CG_TYPE::FLOAT4X4: return CG_TYPE_SIZE_FLOAT4X4;
			case CG_TYPE::INT: 
			case CG_TYPE::UINT: return CG_TYPE_SIZE_INT;
			case CG_TYPE::INT2:
			case CG_TYPE::UINT2: return CG_TYPE_SIZE_INT2;
			case CG_TYPE::INT3:
			case CG_TYPE::UINT3: return CG_TYPE_SIZE_INT3;
			case CG_TYPE::INT4: 
			case CG_TYPE::UINT4: return CG_TYPE_SIZE_INT4;
			case CG_TYPE::SAMPLER: return CG_TYPE_SIZE_SAMPLER;
			case CG_TYPE::TEXTURE: return CG_TYPE_SIZE_TEXTURE;
			case CG_TYPE::CONSTANT_BUFFER: return CG_TYPE_SIZE_CONSTANT_BUFFER;
			case CG_TYPE::COMPUTE_BUFFER: return CG_TYPE_SIZE_COMPUTE_BUFFER;
		}
	
		return (ushort)CG_TYPE::INVALID;
	}
	
	ushort Convert::Components(CG_TYPE type)
	{
		switch (type)
		{
			case CG_TYPE::FLOAT: return CG_TYPE_COMPONENTS_FLOAT;
			case CG_TYPE::FLOAT2: return CG_TYPE_COMPONENTS_FLOAT2;
			case CG_TYPE::FLOAT3: return CG_TYPE_COMPONENTS_FLOAT3;
			case CG_TYPE::FLOAT4: return CG_TYPE_COMPONENTS_FLOAT4;
			case CG_TYPE::FLOAT2X2: return CG_TYPE_COMPONENTS_FLOAT2X2;
			case CG_TYPE::FLOAT3X3: return CG_TYPE_COMPONENTS_FLOAT3X3;
			case CG_TYPE::FLOAT4X4: return CG_TYPE_COMPONENTS_FLOAT4X4;
			case CG_TYPE::INT: 
			case CG_TYPE::UINT: return CG_TYPE_COMPONENTS_INT;
			case CG_TYPE::INT2: 
			case CG_TYPE::UINT2: return CG_TYPE_COMPONENTS_INT2;
			case CG_TYPE::INT3: 
			case CG_TYPE::UINT3: return CG_TYPE_COMPONENTS_INT3;
			case CG_TYPE::INT4: 
			case CG_TYPE::UINT4: return CG_TYPE_COMPONENTS_INT4;
			case CG_TYPE::SAMPLER: return CG_TYPE_COMPONENTS_SAMPLER;
			case CG_TYPE::TEXTURE: return CG_TYPE_COMPONENTS_TEXTURE;
			case CG_TYPE::CONSTANT_BUFFER: return CG_TYPE_COMPONENTS_CONSTANT_BUFFER;
			case CG_TYPE::COMPUTE_BUFFER: return CG_TYPE_COMPONENTS_COMPUTE_BUFFER;
		}
	
		return (ushort)CG_TYPE::INVALID;
	}
	
	ushort Convert::BaseType(CG_TYPE type)
	{
		switch (type)
		{
			case CG_TYPE::FLOAT: return GL_FLOAT;
			case CG_TYPE::FLOAT2: return GL_FLOAT;
			case CG_TYPE::FLOAT3: return GL_FLOAT;
			case CG_TYPE::FLOAT4: return GL_FLOAT;
			case CG_TYPE::FLOAT2X2: return GL_FLOAT;
			case CG_TYPE::FLOAT3X3: return GL_FLOAT;
			case CG_TYPE::FLOAT4X4: return GL_FLOAT;
			case CG_TYPE::INT: return GL_INT;
			case CG_TYPE::INT2: return GL_INT;
			case CG_TYPE::INT3: return GL_INT;
			case CG_TYPE::INT4: return GL_INT;
			case CG_TYPE::UINT: return GL_UNSIGNED_INT;
			case CG_TYPE::UINT2: return GL_UNSIGNED_INT;
			case CG_TYPE::UINT3: return GL_UNSIGNED_INT;
			case CG_TYPE::UINT4: return GL_UNSIGNED_INT;
			case CG_TYPE::SAMPLER: return GL_UNSIGNED_INT64_ARB;
			case CG_TYPE::TEXTURE: return GL_INT;
			case CG_TYPE::CONSTANT_BUFFER: return GL_INT;
			case CG_TYPE::COMPUTE_BUFFER: return GL_INT;
		}
	
		return (ushort)CG_TYPE::INVALID;
	}
	
	ushort Convert::ToNativeEnum(CG_TYPE type)
	{
		switch (type)
		{
			case CG_TYPE::FLOAT: return GL_FLOAT;
			case CG_TYPE::FLOAT2: return GL_FLOAT_VEC2;
			case CG_TYPE::FLOAT3: return GL_FLOAT_VEC3;
			case CG_TYPE::FLOAT4: return GL_FLOAT_VEC4;
			case CG_TYPE::FLOAT2X2: return GL_FLOAT_MAT2;
			case CG_TYPE::FLOAT3X3: return GL_FLOAT_MAT3;
			case CG_TYPE::FLOAT4X4: return GL_FLOAT_MAT4;
			case CG_TYPE::INT: return GL_INT;
			case CG_TYPE::INT2: return GL_INT_VEC2;
			case CG_TYPE::INT3: return GL_INT_VEC3;
			case CG_TYPE::INT4: return GL_INT_VEC4;
			case CG_TYPE::UINT: return GL_UNSIGNED_INT;
			case CG_TYPE::UINT2: return GL_UNSIGNED_INT_VEC2;
			case CG_TYPE::UINT3: return GL_UNSIGNED_INT_VEC3;
			case CG_TYPE::UINT4: return GL_UNSIGNED_INT_VEC4;
			case CG_TYPE::SAMPLER: return GL_SAMPLER;
			case CG_TYPE::TEXTURE: return GL_TEXTURE;
			case CG_TYPE::CONSTANT_BUFFER: return GL_UNIFORM_BUFFER;
			case CG_TYPE::COMPUTE_BUFFER: return GL_SHADER_STORAGE_BUFFER;
		}
	
		return (ushort)CG_TYPE::INVALID;
	}

	CG_TYPE Convert::FromNativeEnum(ushort type)
	{
		switch (type)
		{
			case GL_FLOAT: return CG_TYPE::FLOAT;
			case GL_FLOAT_VEC2: return CG_TYPE::FLOAT2;
			case GL_FLOAT_VEC3: return CG_TYPE::FLOAT3;
			case GL_FLOAT_VEC4: return CG_TYPE::FLOAT4;
			case GL_FLOAT_MAT2: return CG_TYPE::FLOAT2X2;
			case GL_FLOAT_MAT3: return CG_TYPE::FLOAT3X3;
			case GL_FLOAT_MAT4: return CG_TYPE::FLOAT4X4;
			case GL_INT: return CG_TYPE::INT;
			case GL_INT_VEC2: return CG_TYPE::INT2;
			case GL_INT_VEC3: return CG_TYPE::INT3;
			case GL_INT_VEC4: return CG_TYPE::INT4;
			case GL_UNSIGNED_INT: return CG_TYPE::UINT;
			case GL_UNSIGNED_INT_VEC2: return CG_TYPE::UINT2;
			case GL_UNSIGNED_INT_VEC3: return CG_TYPE::UINT3;
			case GL_UNSIGNED_INT_VEC4: return CG_TYPE::UINT4;
			case GL_SAMPLER:
			case GL_SAMPLER_1D:
			case GL_SAMPLER_1D_ARRAY:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_3D:
			case GL_SAMPLER_CUBE: return CG_TYPE::SAMPLER;
			case GL_TEXTURE: return CG_TYPE::TEXTURE;
			case GL_UNIFORM_BUFFER: return CG_TYPE::CONSTANT_BUFFER;
			case GL_SHADER_STORAGE_BUFFER: return CG_TYPE::COMPUTE_BUFFER;
		}

		return CG_TYPE::INVALID;
	}

	CG_TYPE Convert::FromNativeString(const char* string)
	{
		if (strcmp(string, "float") == 0) return CG_TYPE::FLOAT;
		if (strcmp(string, "vec2") == 0) return CG_TYPE::FLOAT2;
		if (strcmp(string, "float2") == 0) return CG_TYPE::FLOAT2;
		if (strcmp(string, "vec3") == 0) return CG_TYPE::FLOAT3;
		if (strcmp(string, "float3") == 0) return CG_TYPE::FLOAT3;
		if (strcmp(string, "vec4") == 0) return CG_TYPE::FLOAT4;
		if (strcmp(string, "float4") == 0) return CG_TYPE::FLOAT4;

		if (strcmp(string, "mat2") == 0) return CG_TYPE::FLOAT2X2;
		if (strcmp(string, "float2x2") == 0) return CG_TYPE::FLOAT2X2;
		if (strcmp(string, "mat3") == 0) return CG_TYPE::FLOAT3X3;
		if (strcmp(string, "float3x3") == 0) return CG_TYPE::FLOAT3X3;
		if (strcmp(string, "mat4") == 0) return CG_TYPE::FLOAT4X4;
		if (strcmp(string, "float4x4") == 0) return CG_TYPE::FLOAT4X4;
		
		if (strcmp(string, "int") == 0) return CG_TYPE::INT;
		if (strcmp(string, "ivec2") == 0) return CG_TYPE::INT2;
		if (strcmp(string, "int2") == 0) return CG_TYPE::INT2;
		if (strcmp(string, "ivec3") == 0) return CG_TYPE::INT3;
		if (strcmp(string, "int3") == 0) return CG_TYPE::INT3;
		if (strcmp(string, "ivec4") == 0) return CG_TYPE::INT4;
		if (strcmp(string, "int4") == 0) return CG_TYPE::INT4;
		if (strcmp(string, "uint") == 0) return CG_TYPE::UINT;
		if (strcmp(string, "uvec2") == 0) return CG_TYPE::UINT2;
		if (strcmp(string, "uint2") == 0) return CG_TYPE::UINT2;
		if (strcmp(string, "uvec3") == 0) return CG_TYPE::UINT3;
		if (strcmp(string, "uint3") == 0) return CG_TYPE::UINT3;
		if (strcmp(string, "uvec4") == 0) return CG_TYPE::UINT4;
		if (strcmp(string, "uint4") == 0) return CG_TYPE::UINT4;
		if (strcmp(string, "sampler1D") == 0) return CG_TYPE::SAMPLER;
		if (strcmp(string, "sampler2D") == 0) return CG_TYPE::SAMPLER;
		if (strcmp(string, "sampler3D") == 0) return CG_TYPE::SAMPLER;
		return CG_TYPE::INVALID;
	}
	
	std::string Convert::ToString(CG_TYPE type)
	{
		switch (type)
		{
			case CG_TYPE::FLOAT: return "FLOAT";
			case CG_TYPE::FLOAT2: return "FLOAT2";
			case CG_TYPE::FLOAT3: return "FLOAT3";
			case CG_TYPE::FLOAT4: return "FLOAT4";
			case CG_TYPE::FLOAT2X2: return "FLOAT2X2";
			case CG_TYPE::FLOAT3X3: return "FLOAT3X3";
			case CG_TYPE::FLOAT4X4: return "FLOAT4X4";
			case CG_TYPE::INT: return "INT";
			case CG_TYPE::INT2: return "INT2";
			case CG_TYPE::INT3: return "INT3";
			case CG_TYPE::INT4: return "INT4";
			case CG_TYPE::UINT: return "UINT";
			case CG_TYPE::UINT2: return "UINT2";
			case CG_TYPE::UINT3: return "UINT3";
			case CG_TYPE::UINT4: return "UINT4";
			case CG_TYPE::SAMPLER: return "SAMPLER";
			case CG_TYPE::TEXTURE: return "TEXTURE";
			case CG_TYPE::CONSTANT_BUFFER: return "CONSTANT_BUFFER";
			case CG_TYPE::COMPUTE_BUFFER: return "COMPUTE_BUFFER";
		}
	
		return "INVALID";
	}
	
	CG_TYPE Convert::FromString(const char* string)
	{
		if (strcmp(string, "FLOAT") == 0) return CG_TYPE::FLOAT;
		if (strcmp(string, "FLOAT2") == 0) return CG_TYPE::FLOAT2;
		if (strcmp(string, "FLOAT3") == 0) return CG_TYPE::FLOAT3;
		if (strcmp(string, "FLOAT4") == 0) return CG_TYPE::FLOAT4;
		if (strcmp(string, "FLOAT2X2") == 0) return CG_TYPE::FLOAT2X2;
		if (strcmp(string, "FLOAT3X3") == 0) return CG_TYPE::FLOAT3X3;
		if (strcmp(string, "FLOAT4X4") == 0) return CG_TYPE::FLOAT4X4;
		if (strcmp(string, "INT") == 0) return CG_TYPE::INT;
		if (strcmp(string, "INT2") == 0) return CG_TYPE::INT2;
		if (strcmp(string, "INT3") == 0) return CG_TYPE::INT3;
		if (strcmp(string, "INT4") == 0) return CG_TYPE::INT4;
		if (strcmp(string, "UINT") == 0) return CG_TYPE::UINT;
		if (strcmp(string, "UINT2") == 0) return CG_TYPE::UINT2;
		if (strcmp(string, "UINT3") == 0) return CG_TYPE::UINT3;
		if (strcmp(string, "UINT4") == 0) return CG_TYPE::UINT4;
		if (strcmp(string, "SAMPLER") == 0) return CG_TYPE::SAMPLER;
		if (strcmp(string, "TEXTURE") == 0) return CG_TYPE::TEXTURE;
		if (strcmp(string, "CONSTANT_BUFFER") == 0) return CG_TYPE::CONSTANT_BUFFER;
		if (strcmp(string, "COMPUTE_BUFFER") == 0) return CG_TYPE::COMPUTE_BUFFER;
		return CG_TYPE::INVALID;
	}
	
	float4x4 Functions::GetMatrixTRS(const float3& position, const quaternion& rotation, const float3& scale)
	{
		float qxx(rotation.x * rotation.x);
		float qyy(rotation.y * rotation.y);
		float qzz(rotation.z * rotation.z);
		float qxz(rotation.x * rotation.z);
		float qxy(rotation.x * rotation.y);
		float qyz(rotation.y * rotation.z);
		float qwx(rotation.w * rotation.x);
		float qwy(rotation.w * rotation.y);
		float qwz(rotation.w * rotation.z);
	
		float4x4 m(1.0f);
		m[3].xyz = position;
		m[0][0] = scale[0] * (1.0f - 2.0f * (qyy + qzz));
		m[0][1] = scale[0] * (2.0f * (qxy + qwz));
		m[0][2] = scale[0] * (2.0f * (qxz - qwy));
	
		m[1][0] = scale[1] * (2.0f * (qxy - qwz));
		m[1][1] = scale[1] * (1.0f - 2.0f * (qxx + qzz));
		m[1][2] = scale[1] * (2.0f * (qyz + qwx));
	
		m[2][0] = scale[2] * (2.0f * (qxz + qwy));
		m[2][1] = scale[2] * (2.0f * (qyz - qwx));
		m[2][2] = scale[2] * (1.0f - 2.0f * (qxx + qyy));
	
		return m;
	}

	float4x4 Functions::GetMatrixTRS(const float3& position, const float3& euler, const float3& scale)
	{
		return GetMatrixTRS(position, glm::quat(euler), scale);
	}
	
	float4x4 Functions::GetMatrixInvTRS(const float3& position, const quaternion& rotation, const float3& scale)
	{
		return glm::inverse(GetMatrixTRS(position, rotation, scale));
	}

	float4x4 Functions::GetMatrixInvTRS(const float3& position, const float3& euler, const float3& scale)
	{
		return GetMatrixInvTRS(position, glm::quat(euler), scale);
	}
	
	float4x4 Functions::GetMatrixTR(const float3& position, const quaternion& rotation)
	{
		float qxx(rotation.x * rotation.x);
		float qyy(rotation.y * rotation.y);
		float qzz(rotation.z * rotation.z);
		float qxz(rotation.x * rotation.z);
		float qxy(rotation.x * rotation.y);
		float qyz(rotation.y * rotation.z);
		float qwx(rotation.w * rotation.x);
		float qwy(rotation.w * rotation.y);
		float qwz(rotation.w * rotation.z);
	
		float4x4 m(1.0f);
		m[3].xyz = position;
		m[0][0] = 1.0f - 2.0f * (qyy + qzz);
		m[0][1] = 2.0f * (qxy + qwz);
		m[0][2] = 2.0f * (qxz - qwy);
				  
		m[1][0] = 2.0f * (qxy - qwz);
		m[1][1] = 1.0f - 2.0f * (qxx + qzz);
		m[1][2] = 2.0f * (qyz + qwx);
				  
		m[2][0] = 2.0f * (qxz + qwy);
		m[2][1] = 2.0f * (qyz - qwx);
		m[2][2] = 1.0f - 2.0f * (qxx + qyy);
	
		return m;
	}
	
	float4x4 Functions::GetPerspective(float fov, float aspect, float nearClip, float farClip)
	{
		auto proj = glm::perspective(fov * CG_FLOAT_DEG2RAD, aspect, nearClip, farClip);
		proj[2][2] *= -1;
		proj[2][3] *= -1;
		return proj;
	}
	
	float4x4 Functions::GetOffsetPerspective(float left, float right, float bottom, float top, float fovy, float aspect, float zNear, float zFar)
	{
		const float tanHalfFovy = tan((fovy * CG_FLOAT_DEG2RAD) / 2.0f);
		const float rcpRL = 1.0f / (right - left);
		const float rcpTB = 1.0f / (top - bottom);
		const float rcpFN = 1.0f / (zFar - zNear);
	
		float4x4 Result(0);
		Result[0][0] = (2.0f * rcpRL) / (aspect * tanHalfFovy);
		Result[1][1] = (2.0f * rcpTB) / tanHalfFovy;
		Result[2][0] = -(right + left) * rcpRL;
		Result[2][1] = -(top + bottom) * rcpTB;
		Result[2][2] = -(zFar + zNear) * rcpFN;
		Result[2][3] = 1.0f;
		Result[3][2] = (2.0f * zFar * zNear) * rcpFN;
	
		return Result;
	}
	
	float4x4 Functions::GetPerspectiveSubdivision(int index, const int3& gridSize, float fovy, float aspect, float znear, float zfar)
	{
		int3 coord;
		coord.x = index % gridSize.x;
		coord.y = (index / gridSize.x) % gridSize.y;
		coord.z = index / (gridSize.x * gridSize.y);
	
		float ix = 2.0f / gridSize.x;
		float iy = 2.0f / gridSize.y;
		float iz = 1.0f / gridSize.z;
	
		float x = -1.0f + (2.0f * (float)coord.x) / gridSize.x;
		float y = -1.0f + (2.0f * (float)coord.y) / gridSize.y;
		float z = (float)coord.z / gridSize.z;
	
		float zrange = zfar - znear;
	
		return Functions::GetOffsetPerspective(x, x + ix, y, y + iy, fovy, aspect, znear + zrange * z, znear + zrange * (z + iz));
	}
	
	size_t Functions::GetNextExponentialSize(size_t start, size_t min)
	{
		if (start < 1)
		{
			start = 1;
		}
	
		while (start < min)
		{
			start <<= 1;
		}
	
		return start;
	}
	
	color Functions::HueToRGB(float hue)
	{
		float R = abs(hue * 6 - 3) - 1;
		float G = 2 - abs(hue * 6 - 2);
		float B = 2 - abs(hue * 6 - 4);
		return float4(glm::clamp(float3(R, G, B), CG_FLOAT3_ZERO, CG_FLOAT3_ONE), 1.0f);
	}
	
	void Functions::NormalizePlane(float4* plane)
	{
		float mag;
		mag = sqrt(plane->x * plane->x + plane->y * plane->y + plane->z * plane->z);
		plane->x = plane->x / mag;
		plane->y = plane->y / mag;
		plane->z = plane->z / mag;
		plane->w = plane->w / mag;
	}
	
	// https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
	void Functions::ExtractFrustrumPlanes(const float4x4 viewprojection, FrustrumPlanes* frustrum, bool normalize)
	{
		float4* planes = frustrum->planes;
		// Left clipping plane
		planes[0].x = viewprojection[0][3] + viewprojection[0][0];
		planes[0].y = viewprojection[1][3] + viewprojection[1][0];
		planes[0].z = viewprojection[2][3] + viewprojection[2][0];
		planes[0].w = viewprojection[3][3] + viewprojection[3][0];
		// Right clipping plane
		planes[1].x = viewprojection[0][3] - viewprojection[0][0];
		planes[1].y = viewprojection[1][3] - viewprojection[1][0];
		planes[1].z = viewprojection[2][3] - viewprojection[2][0];
		planes[1].w = viewprojection[3][3] - viewprojection[3][0];
		// Top clipping plane
		planes[2].x = viewprojection[0][3] - viewprojection[0][1];
		planes[2].y = viewprojection[1][3] - viewprojection[1][1];
		planes[2].z = viewprojection[2][3] - viewprojection[2][1];
		planes[2].w = viewprojection[3][3] - viewprojection[3][1];
		// Bottom clipping plane
		planes[3].x = viewprojection[0][3] + viewprojection[0][1];
		planes[3].y = viewprojection[1][3] + viewprojection[1][1];
		planes[3].z = viewprojection[2][3] + viewprojection[2][1];
		planes[3].w = viewprojection[3][3] + viewprojection[3][1];
		// Near clipping plane
		planes[4].x = viewprojection[0][3] + viewprojection[0][2];
		planes[4].y = viewprojection[1][3] + viewprojection[1][2];
		planes[4].z = viewprojection[2][3] + viewprojection[2][2];
		planes[4].w = viewprojection[3][3] + viewprojection[3][2];
		// Far clipping plane
		planes[5].x = viewprojection[0][3] - viewprojection[0][2];
		planes[5].y = viewprojection[1][3] - viewprojection[1][2];
		planes[5].z = viewprojection[2][3] - viewprojection[2][2];
		planes[5].w = viewprojection[3][3] - viewprojection[3][2];
	
		// Normalize the plane equations, if requested
		if (normalize == true)
		{
			NormalizePlane(&planes[0]);
			NormalizePlane(&planes[1]);
			NormalizePlane(&planes[2]);
			NormalizePlane(&planes[3]);
			NormalizePlane(&planes[4]);
			NormalizePlane(&planes[5]);
		}
	}
	
	bool Functions::IntersectPlanesAABB(const float4* planes, int planeCount, const BoundingBox& aabb)
	{
		for (auto i = 0; i < planeCount; ++i)
		{
			auto& plane = planes[i];
	
			auto bx = plane.x > 0 ? aabb.max.x : aabb.min.x;
			auto by = plane.y > 0 ? aabb.max.y : aabb.min.y;
			auto bz = plane.z > 0 ? aabb.max.z : aabb.min.z;
	
			if (plane.x * bx + plane.y * by + plane.z * bz < -plane.w)
			{
				return false;
			}
		}
	
		return true;
	}

	bool Functions::IntersectAABB(const BoundingBox& a, const BoundingBox& b)
	{
		auto overlap = (!(a.min[0] > b.max[0]) && !(a.max[0] < b.min[0]));
		overlap = (!(a.min[1] > b.max[1]) && !(a.max[1] < b.min[1])) && overlap;
		overlap = (!(a.min[2] > b.max[2]) && !(a.max[2] < b.min[2])) && overlap;
		return overlap;
	}

	bool Functions::IntersectSphere(const float3& center, float radius, const BoundingBox& b)
	{
		float3 d = glm::abs(center - b.GetCenter()) - b.GetExtents();
		float r = radius - glm::compMax(min(d, float3(0.0f)));
		d = max(d, float3(0.0f));
		return radius > 0.0f && dot(d, d) <= r * r;
	}
	
	void Functions::BoundsEncapsulate(BoundingBox* bounds, const BoundingBox& other)
	{
		for (auto i = 0; i < 3; ++i)
		{
			if (other.min[i] < bounds->min[i])
			{
				bounds->min[i] = other.min[i];
			}
	
			if (other.max[i] > bounds->max[i])
			{
				bounds->max[i] = other.max[i];
			}
		}
	}
	
	int Functions::BoundsLongestAxis(const BoundingBox& bounds)
	{
		auto ext = bounds.GetExtents();
		auto length = ext[0];
		auto axis = 0;
	
		for (auto i = 1; i < 3; ++i)
		{
			if (ext[i] > length)
			{
				axis = i;
				length = ext[i];
			}
		}
	
		return axis;
	}
	
	int Functions::BoundsShortestAxis(const BoundingBox& bounds)
	{
		auto ext = bounds.GetExtents();
		auto length = ext[0];
		auto axis = 0;
	
		for (auto i = 1; i < 3; ++i)
		{
			if (ext[i] < length)
			{
				axis = i;
				length = ext[i];
			}
		}
	
		return axis;
	}
	
	void Functions::BoundsSplit(const BoundingBox& bounds, int axis, BoundingBox* out0, BoundingBox* out1)
	{
		*out0 = bounds;
		*out1 = bounds;
		out0->max[axis] = bounds.min[axis] + (bounds.max[axis] - bounds.min[axis]) * 0.5f;
		out1->min[axis] = bounds.min[axis] + (bounds.max[axis] - bounds.min[axis]) * 0.5f;
	}
	
	bool Functions::BoundsContains(const BoundingBox& bounds, const float3& point)
	{
		for (auto i = 0; i < 3; ++i)
		{
			if (bounds.min[i] > point[i] || bounds.max[i] < point[i])
			{
				return false;
			}
		}
	
	    return true;
	}
	
	BoundingBox Functions::BoundsTransform(const float4x4& matrix, const BoundingBox& bounds)
	{
		BoundingBox out(matrix[3].xyz, matrix[3].xyz);
	
		for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
		{
			auto a = matrix[i][j] * bounds.min[j];
			auto b = matrix[i][j] * bounds.max[j];
	
			if (a < b)
			{
				out.min[i] += a;
				out.max[i] += b;
			}
			else
			{
				out.min[i] += b;
				out.max[i] += a;
			}
		}
	
		return out;
	}
}