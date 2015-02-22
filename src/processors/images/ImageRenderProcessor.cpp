#include "../../utils/SharedHeaders.h"
#include "ImageRenderProcessor.h"

#include "../../exceptions/Exception.h"

#include <gtc/matrix_transform.hpp>

using std::pair;
using std::string;
using std::shared_ptr;
using std::vector;
using std::out_of_range;

using glm::mat4;
using glm::vec3;
using glm::vec2;
using glm::translate;

namespace br {
	ImageRenderProcessor::ImageRenderProcessor(shared_ptr<IAssetLoader>loader, pair<string, string> shaders)
		: ProcessorBase(loader, shaders) {
	}

	ImageRenderProcessor::~ImageRenderProcessor() {
		auto allImages = idToImage;
		for(auto& i : allImages) {
			removeImage(i.first);
		}
	}

	void ImageRenderProcessor::addImage(uint32_t id, string path, const vec2& position) {
		if(!enabled)
			throw LogicException(EXCEPTION_INFO, "ImageRenderProcessor not added to Renderer");

		auto it = idToImage.find(id);
		if(it != cend(idToImage))
			throw InvalidObjectIdException(EXCEPTION_INFO, id);

		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		auto wSize = sGConnector->getWindowSize();
		float sx = (2.0f * sGConnector->getAspectRatio()) / wSize.w;
		float sy = 2.0f / wSize.h;

		Texture2d& texture = loader->getTextureBy(path);
		Image image{texture, position, sx, sy};

		if(!hasImageWithTexture(path))
			loadImageToGpu(image);

		idToImage.emplace(id, image);
	}

	void ImageRenderProcessor::removeImage(uint32_t id) {
		if(!enabled)
			throw LogicException(EXCEPTION_INFO, "ImageRenderProcessor not added to Renderer");
		
		Image* image;
		try {
			image = &idToImage.at(id);
		} catch(out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, id);
		}


		string pathAsKey = image->getPath();
		idToImage.erase(id);

		if(!hasImageWithTexture(pathAsKey))
			deleteImageFromGpu(pathAsKey);
	}
	
	void ImageRenderProcessor::doStep(const StepData& stepData) {
		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		sGConnector->setBlending(true);

		for(auto& i : idToImage) {
			Image& object = i.second;

			mat4 translation = translate(mat4(), vec3(object.getPosition(), 0.0f));
			mat4 mvp = translation * stepData.ortho;

			auto& buffer = meshToBuffer.at(object.getPath());
			sGConnector->draw(buffer, program, mvp);
		}

		sGConnector->setBlending(false);
	}


	bool ImageRenderProcessor::hasImageWithTexture(string pathAsKey) {
		auto& it = find_if(cbegin(idToImage), cend(idToImage), [&pathAsKey](pair<uint32_t, Image> i)->bool {
			return i.second.getPath() == pathAsKey;
		});
		return it != cend(idToImage);
	}

	void ImageRenderProcessor::loadImageToGpu(Image& image) {
		vector<Vertex3d> vertices;
		for(auto& i : image.getVertices()) {
			vertices.push_back(i);
		}

		vector<uint16_t> indices;
		for(auto& i : image.getIndices()) {
			indices.push_back(i);
		}

		string pathAsKey = image.getPath();		
		loadGeometryToGpu(pathAsKey, vertices, indices);

		Texture2d& texture = loader->getTextureBy(pathAsKey);
		loadTextureToGpu(texture);		

		auto& buffer = meshToBuffer.at(pathAsKey);
		buffer.texture = textureToId.at(pathAsKey);
	}

	void ImageRenderProcessor::deleteImageFromGpu(string pathAsKey) {
		deleteGeometryFromGpu(pathAsKey);
		deleteTextureFromGpu(pathAsKey);
	}
}
