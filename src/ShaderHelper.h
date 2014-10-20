#pragma once
#include <string>
#include <vector>

class ShaderHelper
{
public:
	ShaderHelper() = delete;
	~ShaderHelper() = delete;

	static std::string defaultVertexShader;
	static std::string defaultFragmentShader;
	static std::string texVertexShader;
	static std::string texFragmentShader;

	static std::string animVertexShader;
};

