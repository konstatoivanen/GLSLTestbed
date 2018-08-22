
class Shader
{
	public:
		static unsigned int LoadShader(const string& filename);

		static unsigned int GetProgram(const string& name);
		static unsigned int GetProgram(int hashId);
		static int HashId(const string& name);
};

