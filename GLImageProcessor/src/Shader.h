#include <map>
#include <vector>
#include "linmath.h"

class Shader
{
	public:
		static Shader*	ImportShader(const string& filename);
		static Shader*  Find(const string& name);
		static Shader*  Find(const int& hashId);
		static int		HashId(const string& name);
		static void		ReleaseAll();

		void SetVector4(const int& hashId, vec4 value);
		void SetVector3(const int& hashId, vec3 value);
		void SetVector2(const int& hashId, vec2 value);
		void SetFloat(const int& hashId, float value);
		void UseProgram()
		{
			glUseProgram(programId);
		}

	private:
		Shader(const unsigned int& sprogramId, const vector<string>& uniforms);

		map<int, int> variableMap;
		unsigned int  programId;

};

