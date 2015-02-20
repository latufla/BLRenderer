#pragma once

#include <memory>
#include <glm.hpp>
#include "interfaces/IGraphicsConnector.h"

namespace br{
	class GlesConnector : public IGraphicsConnector{
	public:
		GlesConnector() = delete;
		virtual ~GlesConnector();

		GlesConnector(const IWindowVendor::Rect& size);
		
		void setViewport(const IWindowVendor::Rect& size) override;
		void clear() override;

		void swapBuffers() override;

		IWindowVendor::Rect getWindowSize() const override;
		float getAspectRatio() const override;

		glm::vec2 getMousePosition() const override;

		bool doStep() override;

		ProgramContext createProgram(std::pair<std::string, std::string> shaders) override;
		void deleteProgram(ProgramContext&) override;
		
		uint32_t loadTextureToGpu(Texture2d&) override;
		void deleteTextureFromGpu(uint32_t) override;

		GpuBufferData loadGeometryToGpu(std::vector<Vertex3d>& vertices, std::vector<uint16_t>& indices) override;
		void deleteGeometryFromGpu(GpuBufferData&) override;

		void setBlending(bool) override;

		void draw(GpuBufferData& buffer, ProgramContext& program, glm::mat4& mvp) override;
		void draw(TextField& image, GpuBufferData& buffer, ProgramContext program, glm::mat4 mvp) override;
		void draw(GpuBufferData& buffer, ProgramContext& program, glm::mat4& mvp, BoneTransformer::BonesDataMap& bonesData) override;

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

