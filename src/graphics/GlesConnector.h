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
		
		virtual void setViewport(const IWindowVendor::Rect& size) override;
		virtual void clear() override;

		virtual void swapBuffers() override;

		virtual IWindowVendor::Rect getWindowSize() const override;
		virtual float getAspectRatio() const override;

		virtual glm::vec2 getMousePosition() const override;

		virtual bool doStep() override;

		virtual std::shared_ptr<IProgramContext> createProgram(std::pair<std::string, std::string> shaders,
			std::unordered_map<std::string, std::string> attributeBindings,
			std::unordered_map<std::string, std::string> uniformBindings) override;
		virtual void deleteProgram(std::shared_ptr<IProgramContext>) override;

		virtual uint32_t loadTextureToGpu(std::vector<uint8_t> const& texture, uint32_t width, uint32_t height) override;
		virtual void deleteTextureFromGpu(uint32_t) override;

		virtual GpuBufferData loadGeometryToGpu(std::vector<float>& vertices, std::vector<uint16_t>& indices) override;
		virtual void deleteGeometryFromGpu(GpuBufferData&) override;

		virtual void setBlending(bool) override;

		virtual void draw(GpuBufferData& buffer, std::shared_ptr<IProgramContext> program, std::vector<ProgramParam> params) override;
		virtual void draw(GpuBufferData& buffer, std::shared_ptr<IProgramContext> program, std::vector<ProgramParam> params, BoneTransformer::BonesDataMap& bonesData) override;

	private:
		std::shared_ptr<IWindowVendor> window;

		struct EglContext {
			void* display;
			void* surface;
			void* context;
		} eglContext;
		void initEgl();
	
		uint32_t createShader(uint32_t type, const char* source);
	
		void loadProgramParams(std::vector<ProgramParam> params);
	};
}

