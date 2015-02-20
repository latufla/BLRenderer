#pragma once
#include "../../utils/SharedHeaders.h"
#include <glm.hpp>
#include "IWindowVendor.h"
#include <utility>
#include "../../Texture2d.h"
#include "../../Mesh3d.h"
#include "../../processors/text/TextField.h"
#include "../../utils/bones/BoneTransformer.h"

namespace br {
	class IGraphicsConnector {
	public:
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

		virtual void setViewport(const IWindowVendor::Rect& size) = 0;
		virtual void clear() = 0;

		virtual void swapBuffers() = 0;

		virtual IWindowVendor::Rect getWindowSize() const = 0;
		virtual float getAspectRatio() const = 0;

		virtual glm::vec2 getMousePosition() const = 0;

		virtual bool doStep() = 0;

		virtual ProgramContext createProgram(std::pair<std::string, std::string> shaders) = 0;
		virtual void deleteProgram(ProgramContext&) = 0;

		virtual uint32_t loadTextureToGpu(Texture2d&) = 0;
		virtual void deleteTextureFromGpu(uint32_t) = 0;

		virtual GpuBufferData loadGeometryToGpu(std::vector<Vertex3d>& vertices, std::vector<uint16_t>& indices) = 0;
		virtual void deleteGeometryFromGpu(GpuBufferData&) = 0;

		virtual void setBlending(bool) = 0;

		virtual void draw(GpuBufferData& buffer, ProgramContext& program, glm::mat4& mvp) = 0;
		virtual void draw(TextField& image, GpuBufferData& buffer, ProgramContext program, glm::mat4 mvp) = 0;
		virtual void draw(GpuBufferData& buffer, ProgramContext& program, glm::mat4& mvp, BoneTransformer::BonesDataMap& bonesData) = 0;
	};
}

