#include <map>
#include <vector>
#include "linmath.h"

class Shader
{
	public:
		/// <summary>
		/// Imports & creates a new shader
		/// </summary>
		static Shader*		Import(const string& filename);
		/// <summary>
		/// Import multiple shaders from a list file.
		/// The list file must include path to the shaders relative to the application directory.
		/// </summary>
		static void			ImportMultiple(const string& filename);
		/// <summary>
		/// Find program by name string from imported shader collection
		/// </summary>
		static Shader*		Find(const string& name);
		/// <summary>
		/// Find program by name hash from imported shader collection
		/// </summary>
		static Shader*		Find(const int& hashId);
		/// <summary>
		/// Convert string to shader hashId
		/// </summary>
		static int			HashId(const string& name);
		/// <summary>
		/// Returns the namehashIds of all imported shaders
		/// </summary>
		static vector<int>  GetAllNameHashIds();
		/// <summary>
		/// Delete all shaders & clear shaderCollection
		/// </summary>
		static void			ReleaseAll();

		/// <summary>
		/// Submits a vector4 value to the shader
		/// </summary>
		void SetVector4(const int& hashId, vec4 value);
		/// <summary>
		/// Submits a vector3 value to the shader
		/// </summary>
		void SetVector3(const int& hashId, vec3 value);
		/// <summary>
		/// Submits a vector2 value to the shader
		/// </summary>
		void SetVector2(const int& hashId, vec2 value);
		/// <summary>
		/// Submits a float value to the shader
		/// </summary>
		void SetFloat(const int& hashId, float value);

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
		Shader(const unsigned int& sprogramId, const vector<string>& uniforms, string filepath);

		void MapVariables(const vector<string>& uniforms);

		/// <summary>
		/// dictionary of shader program variable locations
		/// </summary>
		map<int, int> variableMap;
		/// <summary>
		/// OpenGL shader program id 
		/// </summary>
		unsigned int  programId;
		/// <summary>
		/// cached filepath for potential reimport
		/// </summary>
		string		  cachedFilepath;
};

