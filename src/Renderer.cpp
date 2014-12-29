#include "utils/SharedHeaders.h"
#include "Renderer.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "exceptions/Exception.h"
#include "processors/ProcessorBase.h"

using std::string;

using std::shared_ptr;
using std::make_shared;

using glm::mat4;
using glm::vec3;
using glm::lookAt;
using glm::perspective;
using glm::ortho;


namespace br {
	Renderer::Renderer(shared_ptr<AssetLoader> loader,
		uint32_t wndX,
		uint32_t wndY,
		uint32_t wndW, 
		uint32_t wndH)
		: loader(loader) {
		
		window = make_shared <WindowVendor>(wndX, wndY, wndW, wndH);
		initEgl();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);

		// TODO: turn off when FXs only
// 		glEnable(GL_CULL_FACE);
// 		glCullFace(GL_BACK);
// 		glFrontFace(GL_CCW);
	}

	void Renderer::addProcessor(shared_ptr<ProcessorBase> val) {
		auto it = find(cbegin(processors), cend(processors), val);
		if(it != cend(processors))
			throw LogicException(EXCEPTION_INFO, "has such processor");

		val->start(window);
		processors.push_back(val);
	}

	void Renderer::removeProcessor(shared_ptr<ProcessorBase> val) {
		auto it = find(cbegin(processors), cend(processors), val);
		if(it == cend(processors))
			throw LogicException(EXCEPTION_INFO, "no such processor");

		val->stop();
		processors.erase(it);
	}

	void Renderer::setCamera(float x, float y, float z) {
		camera.x = x;
		camera.y = y;
		camera.z = z;
	}

	bool Renderer::doStep(long long stepMSec) {
		auto winSize = window->getSize();
		glViewport(0, 0, (uint32_t)winSize.w, (uint32_t)winSize.h);
	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 view = lookAt(vec3{camera.x, camera.y, camera.z}, vec3{0, 0, 0}, vec3{0, 1, 0});
		mat4 projection = perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f);
		mat4 projectionView = projection * view;
		
		auto scaleFactor = window->getScaleFactor();
		float sx = scaleFactor.first;
		float sy = scaleFactor.second;
		mat4 orthoProjection = ortho(-sx, sx, -sy, sy);
		for(auto i : processors) {
			ProcessorBase::StepData stepData {
				stepMSec, 
				projectionView, 
				orthoProjection
			};
			i->tryDoStep(stepData);
		}

		eglSwapBuffers(eglContext.display, eglContext.surface);

		return window->doStep();
	}
	

	void Renderer::initEgl(){
		EGLint minorVersion;
		EGLint majorVersion;
	
		auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if(display == EGL_NO_DISPLAY)
			throw EglException(EXCEPTION_INFO, "can`t get display");
	
		if(!eglInitialize(display, &majorVersion, &minorVersion))
			throw EglException(EXCEPTION_INFO, "cant init display");	
	
		const EGLint cfgAttribs[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_RED_SIZE, 5,
			EGL_GREEN_SIZE, 6,
			EGL_BLUE_SIZE, 5,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, 8,
			EGL_STENCIL_SIZE, 8,
			// EGL_SAMPLE_BUFFERS, 1,
			EGL_NONE
		};
	
	
		const EGLint maxConfigs = 2;
		EGLConfig configs[2];
		EGLint numConfigs;
		if(!eglChooseConfig(display, cfgAttribs, configs, maxConfigs, &numConfigs))
			throw EglException(EXCEPTION_INFO, "can`t choose config");
	
		const EGLint srfAttribs[] = {
			EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
			EGL_NONE
		};
	
		auto surface = eglCreateWindowSurface(display, configs[0], window->nativeWindow, srfAttribs);
		if (surface == EGL_NO_SURFACE)
			throw EglException(EXCEPTION_INFO, "can`t create window surface");

	
		const EGLint ctxAttribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};
	
		auto context = eglCreateContext(display, configs[0], EGL_NO_CONTEXT, ctxAttribs);
		if (context == EGL_NO_CONTEXT)
			throw EglException(EXCEPTION_INFO, "can`t create context");

	
		if(!eglMakeCurrent(display, surface, surface, context))
			throw EglException(EXCEPTION_INFO, "can`t make context current");
	
		eglContext.display = display;
		eglContext.surface = surface;
		eglContext.context = context;
	}
}
