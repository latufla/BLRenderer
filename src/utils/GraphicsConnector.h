#pragma once

#include <memory>
#include <glm.hpp>

#include "..\IWindowVendor.h"
#include "..\Texture2d.h"
#include "..\processors\images\Image.h"
#include "..\processors\text\TextField.h"
#include "..\bones\BoneTransformer.h"

namespace br{
	struct GpuBufferData {
		uint32_t vBuffer;
		uint32_t iBuffer;
		uint32_t iBufferLenght;

		uint32_t texture;
	};

	struct ProgramContext {
		int32_t id = -1;

		int32_t position = -1;
		int32_t uv = -1;
		int32_t sampler = -1;

		int32_t bones = -1;
		int32_t boneIds = -1;
		int32_t weights = -1;
	
		int32_t mvp = -1;

		int32_t color = -1;
	};

	class GraphicsConnector {
	public:
		GraphicsConnector() = delete;
		~GraphicsConnector() = default;

		GraphicsConnector(uint32_t wndX, uint32_t wndY, uint32_t wndW, uint32_t wndH);
		
		void setViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
		void clear();

		void swapBuffers();

		IWindowVendor::Rect getWindowSize();
		glm::vec2 getScaleFactor();

		glm::vec2 getMousePosition();

		bool doStep();

		ProgramContext createProgram(std::pair<std::string, std::string> shaders);
		void deleteProgram(ProgramContext&);
		
		uint32_t loadTextureToGpu(Texture2d&);
		void deleteTextureFromGpu(uint32_t);

		GpuBufferData loadGeometryToGpu(std::vector<Vertex3d>& vertices, std::vector<uint16_t>& indices);
		void deleteGeometryFromGpu(GpuBufferData&);

		void setBlending(bool);

		void draw(GpuBufferData& buffer, ProgramContext& program, glm::mat4& mvp);
		void draw(TextField& image, GpuBufferData& buffer, ProgramContext program, glm::mat4 mvp);
		void draw(GpuBufferData& buffer, ProgramContext& program, glm::mat4& mvp, BoneTransformer::BonesDataMap& bonesData);

	private:
		std::shared_ptr<IWindowVendor> window;

		struct EglContext {
			void* display;
			void* surface;
			void* context;
		} eglContext;
		void initEgl();
	
		uint32_t createShader(uint32_t type, const char* source);

	};
}

