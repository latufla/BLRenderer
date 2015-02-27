#include "../../utils/SharedHeaders.h"
#include "ImageRenderProcessor.h"

#include "../../exceptions/Exception.h"

#include <gtc/matrix_transform.hpp>
#include "../../assets/AssetLoader.h"

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
	ImageRenderProcessor::ImageRenderProcessor(shared_ptr<IAssetLoader>loader)
		: ProcessorBase(loader) {
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

		auto programContext = nameToProgramContext.at(AssetLoader::IMAGE_PROGRAM);
		for(auto& i : idToImage) {
			Image& object = i.second;

			std::vector<IGraphicsConnector::ProgramParam> params;

			IGraphicsConnector::ProgramParam mvp;
			mvp.id = programContext->getLoc("mvp");
			mat4 translation = translate(mat4(), vec3(object.getPosition(), 0.0f));
			mvp.mat4 = std::make_shared<glm::mat4>(translation * stepData.ortho);
			params.push_back(mvp);

			auto& buffer = meshToBuffer.at(object.getPath());
			sGConnector->draw(buffer, programContext, params);
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
		if(idToImage.empty()) {
			auto programContext = loader->getProgramBy(AssetLoader::IMAGE_PROGRAM);
			loadProgramToGpu(programContext->getName(), programContext);
		}

		string pathAsKey = image.getPath();		
		auto mesh = image.getMesh();
		loadGeometryToGpu(pathAsKey, mesh->getRawVertices(), mesh->getIndices());

		Texture2d& texture = loader->getTextureBy(pathAsKey);
		loadTextureToGpu(texture);		

		auto& buffer = meshToBuffer.at(pathAsKey);
		buffer.texture = textureToId.at(pathAsKey);
	}

	void ImageRenderProcessor::deleteImageFromGpu(string pathAsKey) {
		if(idToImage.empty())
			deleteProgramFromGpu(AssetLoader::IMAGE_PROGRAM);
	
		deleteGeometryFromGpu(pathAsKey);
		deleteTextureFromGpu(pathAsKey);
	}
}
