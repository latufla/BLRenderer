#include "stdafx.h"
#include "ShaderHelper.h"

std::string ShaderHelper::defaultVertexShader =
"uniform mat4 mvpMatrix;	  \n"
"attribute vec4 aPosition;    \n"
//"attribute vec4 aColor;		  \n"
//"varying vec4 vColor;		  \n"
"void main()                  \n"
"{                            \n"
//"   vColor = aColor;  \n"
"   gl_Position = mvpMatrix * aPosition;  \n"
"}                            \n";

std::string ShaderHelper::defaultFragmentShader =
"precision mediump float;\n"
//"varying vec4 vColor;\n"
"void main()                                  \n"
"{                                            \n"
//"  gl_FragColor = vColor;					  \n"
"  gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);					  \n"
"}                                            \n";


std::string ShaderHelper::texVertexShader =
"uniform mat4 mvpMatrix;	  \n"
"attribute vec4 aPosition;   \n"
"attribute vec2 aTexCoord;   \n"
"varying vec2 vTexCoord;     \n"
"void main()                  \n"
"{                            \n"
"   gl_Position = mvpMatrix * aPosition;  \n"
"   vTexCoord = aTexCoord;  \n"
"}                            \n";

std::string ShaderHelper::texFragmentShader = 
"precision mediump float;                            \n"
"varying vec2 vTexCoord;                            \n"
"uniform sampler2D sTexture;                        \n"
"void main()                                         \n"
"{                                                   \n"
"  gl_FragColor = texture2D( sTexture, vTexCoord );\n"
"}                                                   \n";





