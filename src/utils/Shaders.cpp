#include "Shaders.h"
#include "..\exceptions\Exception.h"

using std::string;
using std::pair;

namespace br {
	const string Shaders::MODEL_SHADER = "modelShader";
	const string Shaders::TEXT_SHADER = "textShader";
	const string Shaders::IMAGE_SHADER = "imageShader";

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
		nameToShaderSrc.emplace(MODEL_SHADER, modelShaders);

		
		string imageVS =
			"uniform mat4 mvpMatrix;		\n"
			"attribute vec4 aPosition;		\n"

			"attribute vec2 aTexCoord;		\n"
			"varying vec2 vTexCoord;		\n"
			"void main(){					\n"
			"   gl_Position = mvpMatrix * aPosition;					\n"
			"   vTexCoord = aTexCoord;									\n"
			"}";

		string imageFS =
			"precision mediump float;                           \n"
			"varying vec2 vTexCoord;                            \n"
			"uniform sampler2D sTexture;                        \n"
			"void main(){										\n"
			"  gl_FragColor = texture2D( sTexture, vTexCoord ); \n"
			"}";

		pair<string, string> imageShaders{imageVS, imageFS};
		nameToShaderSrc.emplace(IMAGE_SHADER, imageShaders);


		string textVS =
			"uniform mat4 mvpMatrix;		\n"
			"attribute vec4 aPosition;		\n"

			"attribute vec2 aTexCoord;		\n"
			"varying vec2 vTexCoord;		\n"
			"void main(){					\n"
			"   gl_Position = mvpMatrix * aPosition;								\n"
			"   vTexCoord = aTexCoord;									\n"
			"}";

		string textFS =
			"precision mediump float;                           \n"
			"varying vec2 vTexCoord;                            \n"
			"uniform sampler2D sTexture;                        \n"
			"uniform vec4 color;"
			"void main(){										\n"
			"  gl_FragColor = texture2D( sTexture, vTexCoord ) * color; \n"
			"}";

		pair<string, string> textShaders{textVS, textFS};
		nameToShaderSrc.emplace(TEXT_SHADER, textShaders);
	}

	pair<string, string> Shaders::getShaderSrcBy(const string name) {
		try {
			return nameToShaderSrc.at(name);
		} catch(std::out_of_range&) {
			throw AssetException(EXCEPTION_INFO, name, "can`t get shader");
		}
	}
}
