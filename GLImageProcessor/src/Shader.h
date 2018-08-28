#ifndef SHADER_H
#define SHADER_H

#include <map>
#include <vector>
#include "linmath.h"

class Shader
{
	public:
		/// <summary>
		/// Imports & creates a new shader
		/// </summary>
		static Shader*			  Import(const std::string& filename);
		/// <summary>
		/// Import multiple shaders from a list file.
		/// The list file must include path to the shaders relative to the application directory.
		/// </summary>				  
		static void				  ImportMultiple(const std::string& filename);
		/// <summary>
		/// Find program by name string from imported shader collection
		/// </summary>				  
		static Shader*			  Find(const std::string& name);
		/// <summary>
		/// Find program by name hash from imported shader collection
		/// </summary>				  
		static Shader*			  Find(int hashId);
		/// <summary>
		/// Convert string to shader hashId
		/// </summary>				  
		static int				  HashId(const std::string& name);
		/// <summary>
		/// Returns the namehashIds of all imported shaders
		/// </summary>
		static std::vector<int>   GetAllNameHashIds();
		/// <summary>
		/// Delete all shaders & clear shaderCollection
		/// </summary>
		static void				  ReleaseAll();

		/// <summary>
		/// Submits a vector4 value to the shader
		/// </summary>
		void SetVector4(int hashId, vec4 value);
		/// <summary>
		/// Submits a vector3 value to the shader
		/// </summary>
		void SetVector3(int hashId, vec3 value);
		/// <summary>
		/// Submits a vector2 value to the shader
		/// </summary>
		void SetVector2(int hashId, vec2 value);
		/// <summary>
		/// Submits a float value to the shader
		/// </summary>
		void SetFloat(int hashId, float value);

		/// <summary>
		/// Sets shader to be used in the upcoming drawcalls
		/// or until a new program is assigned
		/// </summary>
		void UseProgram()
		{
			glUseProgram(programId);
		}
		/// <summary>
		/// Recompiles the shader
		/// </summary>
		void Reimport();

	private:
		///</summary>
		/// Private constructor for a shader object.
		/// Should only be used in import method.
		///</summary>
		Shader(unsigned int sprogramId, const std::vector<std::string>& uniforms, std::string filepath);

		///<summary>
		/// Caches locations for uniform variables in the shader program.
		///</summary>
		void MapVariables(const std::vector<std::string>& uniforms);

		/// <summary>
		/// dictionary of shader program variable locations
		/// </summary>
		std::map<int, int> variableMap;
		/// <summary>
		/// OpenGL shader program id 
		/// </summary>
		unsigned int  programId;
		/// <summary>
		/// cached filepath for potential reimport
		/// </summary>
		std::string   cachedFilepath;
};
#endif