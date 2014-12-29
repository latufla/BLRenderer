#include "Shaders.h"
#include "..\exceptions\Exception.h"

using std::string;
using std::pair;

namespace br {
	const string Shaders::MODEL_PROGRAM = "modelProgram";
	const string Shaders::MODEL_DEBUG_PROGRAM = "modelDebugProgram";
	const string Shaders::TEXT_PROGRAM = "textProgram";
	const string Shaders::IMAGE_PROGRAM = "imageProgram";

	Shaders::Shaders() {
		string modelVS =
			"uniform mat4 mvpMatrix;		\n"
			"uniform mat4 bones[25];		\n"
			"attribute vec4 aPosition;		\n"

			"attribute vec2 aTexCoord;		\n"

			"attribute vec4 boneIds;		\n"
			"attribute vec4 weights;		\n"

			"varying vec2 vTexCoord;		\n"
			"void main(){					\n"
			"	mat4 boneTransform = bones[int(boneIds.x)] * weights.x;		\n"
			"	boneTransform += bones[int(boneIds.y)] * weights.y;			\n"
			"	boneTransform += bones[int(boneIds.z)] * weights.z;			\n"
			"	boneTransform += bones[int(boneIds.w)] * weights.w;			\n"
			"   vec4 pos = boneTransform * aPosition;						\n"
			"   gl_Position = mvpMatrix * pos;								\n"
			"   vTexCoord = aTexCoord;										\n"
			"}";

		string modelFS =
			"precision mediump float;                           \n"
			"varying vec2 vTexCoord;                            \n"
			"uniform sampler2D sTexture;                        \n"
			"void main(){										\n"
			"  gl_FragColor = texture2D( sTexture, vTexCoord ); \n"
			"}";
		
		pair<string, string> modelShaders{modelVS, modelFS};
		nameToShaderPair.emplace(MODEL_PROGRAM, modelShaders);

		string modelDebugFS =
			"precision mediump float;                           \n"
			"varying vec2 vTexCoord;                            \n"
			"uniform sampler2D sTexture;                        \n"
			"void main(){										\n"
			"  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n"
			"}";

		pair<string, string> modelDebugShaders{modelVS, modelDebugFS};
		nameToShaderPair.emplace(MODEL_DEBUG_PROGRAM, modelDebugShaders);
		

		string imageVS =
			"uniform mat4 mvpMatrix;		\n"
			"attribute vec4 aPosition;		\n"

			"attribute vec2 aTexCoord;		\n"

			"varying vec2 vTexCoord;		\n"
			"void main(){					\n"
			"   gl_Position = mvpMatrix * aPosition;						\n"
			"   vTexCoord = aTexCoord;										\n"
			"}";

		string imageFS =
			"precision mediump float;                           \n"
			"varying vec2 vTexCoord;                            \n"
			"uniform sampler2D sTexture;                        \n"
			"void main(){										\n"
			"  gl_FragColor = texture2D( sTexture, vTexCoord ); \n"
			"}";

		pair<string, string> imageShaders{imageVS, imageFS};
		nameToShaderPair.emplace(IMAGE_PROGRAM, imageShaders);

		
		string textFS =
			"precision mediump float;                           \n"
			"varying vec2 vTexCoord;                            \n"
			"uniform sampler2D sTexture;                        \n"
			"uniform vec4 color;"
			"void main(){										\n"
			"  gl_FragColor = texture2D( sTexture, vTexCoord ) * color; \n"
			"}";

		pair<string, string> textShaders{imageVS, textFS};
		nameToShaderPair.emplace(TEXT_PROGRAM, textShaders);
	}

	pair<string, string> Shaders::getProgram(const string name) {
		try {
			return nameToShaderPair.at(name);
		} catch(std::out_of_range&) {
			throw AssetException(EXCEPTION_INFO, name, "can`t get shader");
		}
	}
}
