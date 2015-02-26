#pragma once
#include "../../utils/SharedHeaders.h"
#include <glm.hpp>
#include "IWindowVendor.h"
#include <utility>
#include "../../utils/bones/BoneTransformer.h"
#include <unordered_map>
#include "IProgramContext.h"

namespace br {
	class IGraphicsConnector {
	public:
		struct GpuBufferData {
			uint32_t vBuffer;
			uint32_t iBuffer;
			uint32_t iBufferLenght;

			uint32_t texture;
		};

		struct ProgramParam {
			int32_t id;
			std::shared_ptr<glm::mat4> mat4;
			std::shared_ptr<glm::vec4> vec4;
		};


		virtual void setViewport(const IWindowVendor::Rect& size) = 0;
		virtual void clear() = 0;

		virtual void swapBuffers() = 0;

		virtual IWindowVendor::Rect getWindowSize() const = 0;
		virtual float getAspectRatio() const = 0;

		virtual glm::vec2 getMousePosition() const = 0;

		virtual bool doStep() = 0;

		virtual std::shared_ptr<IProgramContext> createProgram(std::pair<std::string, std::string> shaders,
			std::unordered_map<std::string, std::string> attributeBindings,
			std::unordered_map<std::string, std::string> uniformBindings) = 0;

		virtual void deleteProgram(std::shared_ptr<IProgramContext>) = 0;

		virtual uint32_t loadTextureToGpu(std::vector<uint8_t> const& texture, uint32_t width, uint32_t height) = 0;
		virtual void deleteTextureFromGpu(uint32_t) = 0;

		virtual GpuBufferData loadGeometryToGpu(std::vector<float>& vertices, std::vector<uint16_t>& indices) = 0;
		virtual void deleteGeometryFromGpu(GpuBufferData&) = 0;

		virtual void setBlending(bool) = 0;

		virtual void draw(GpuBufferData& buffer, std::shared_ptr<IProgramContext> program, std::vector<ProgramParam> params) = 0;
		virtual void draw(GpuBufferData& buffer, std::shared_ptr<IProgramContext> program, std::vector<ProgramParam> params, BoneTransformer::BonesDataMap& bonesData) = 0;
	};
}

