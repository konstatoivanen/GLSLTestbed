#include <map>
#include <vector>
#include "linmath.h"

class Shader
{
	public:
		Shader(const unsigned int& sprogramId, const vector<string>& uniforms);

		static Shader*	LoadShader(const string& filename);
		static void		ReleaseAll();
		static int		HashId(const string& name);

		void SetVector4(int hashId, vec4 value);
		void SetVector3(int hashId, vec3 value);
		void SetVector2(int hashId, vec2 value);
		void SetFloat(int hashId, float value);

		int GetProgramId()
		{
			return programId;
		}

	private:
		map<int, int>	uniformLocations;
		unsigned int	programId;

};

