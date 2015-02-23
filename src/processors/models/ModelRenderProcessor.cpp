#include "../../utils/SharedHeaders.h"
#include "ModelRenderProcessor.h"

#include "../../assets/Model3d.h"
#include "../../assets/Texture2d.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "../../exceptions/Exception.h"

using std::shared_ptr;
using std::pair;
using std::vector;
using std::array;
using std::unordered_map;
using std::string;
using std::out_of_range;

using glm::mat4;
using glm::make_mat4;

namespace br {
	ModelRenderProcessor::ModelRenderProcessor(shared_ptr<IAssetLoader>loader, pair<string, string> shaders) 
		: ProcessorBase(loader, shaders){
	}
	
	ModelRenderProcessor::~ModelRenderProcessor() {
		auto allObjects = idToObject;
		for(auto& i : allObjects) {
			removeObject(i.first);
		}
	}
	
	void ModelRenderProcessor::addObject(uint32_t id, string modelPath) {
		auto it = idToObject.find(id);
		if(it != cend(idToObject))
			throw InvalidObjectIdException(EXCEPTION_INFO, id);

		if(!hasObjectWithModel(modelPath)) // first in
			loadModelToGpu(modelPath);

		View object{id, modelPath};
		idToObject.emplace(id, object);	

		stopAnimation(id, Animation3d::DEFAULT_ANIMATION_NAME);
	}

	void ModelRenderProcessor::removeObject(uint32_t id) {
		View* object;
		try {
			object = &idToObject.at(id);
		} catch(out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, id);
		}

		string modelPath = object->getPath();
		idToObject.erase(id);

		if(!hasObjectWithModel(modelPath)) // last out
			deleteModelFromGpu(modelPath);
	}

	void ModelRenderProcessor::playAnimation(uint32_t objId, string animName, bool loop) {
		View* object;
		try {
			object = &idToObject.at(objId);
		} catch(out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, objId);
		}

		Model3d& model = loader->getModelBy(object->getPath());
		Animation3d& animation = model.getAnimationBy(animName);
		object->playAnimation(animation, loop);
	}

	void ModelRenderProcessor::stopAnimation(uint32_t objId, string animName) {
		View* object;
		try {
			object = &idToObject.at(objId);
		} catch(out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, objId);
		}

		Model3d& model = loader->getModelBy(object->getPath());
		Animation3d& animation = model.getAnimationBy(animName);
		object->stopAnimation(animation);
	}

	void ModelRenderProcessor::transformObject(uint32_t objId, const mat4& transform) {
		View* object;
		try {
			object = &idToObject.at(objId);
		} catch(out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, objId);
		}
		object->setTransform(transform);
	}
	
	void ModelRenderProcessor::doStep(const StepData& stepData) {
		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		for(auto& i : idToObject) {
			View& object = i.second;
			mat4 mvpMatrix = stepData.perspectiveView * object.getTransform();

			Model3d& model = loader->getModelBy(object.getPath());
			vector<Mesh3d>& meshes = model.getMeshes();

			object.doAnimationStep(stepData.stepMSec);
			for(auto& s : meshes) {
				auto bonesData = prepareAnimationStep(object, s);			
				string meshName = model.getUniqueMeshName(s);
				auto& buffer = meshToBuffer.at(meshName);
				sGConnector->draw(buffer, program, mvpMatrix, bonesData);
			}
		}

		StepData step = stepData;
		step.extraData = &idToObject;
		__super::doStep(step);
	}

	BoneTransformer::BonesDataMap ModelRenderProcessor::prepareAnimationStep(View& object, Mesh3d& m) {
		BoneTransformer::BonesDataMap res;
		auto& boneIdToOffset = m.getBoneIdToOffset();
		for(auto& i : boneIdToOffset) {
			BoneTransformer::BoneData bData{i.second};
			res.emplace(i.first, bData);
		}

		Model3d& model = loader->getModelBy(object.getPath());
		boneTransformer.transform(object, model, res);
		return res;
	}

	bool ModelRenderProcessor::hasObjectWithModel(string path) {
		auto& it = find_if(cbegin(idToObject), cend(idToObject), [&path](pair<uint32_t, View> i)->bool {
			return i.second.getPath() == path;
		});
		return it != cend(idToObject);
	}

	void ModelRenderProcessor::loadModelToGpu(string modelPath) {
		Model3d& model = loader->getModelBy(modelPath);
		vector<Mesh3d>& meshes = model.getMeshes();
		for(auto& s : meshes) {
			string meshName = model.getUniqueMeshName(s);

			loadGeometryToGpu(meshName, s.getRawVertices(), s.getIndices());

			Texture2d& texture = model.getTextureBy(s);
			loadTextureToGpu(texture);

			auto& buffer = meshToBuffer.at(meshName);
			buffer.texture = textureToId.at(texture.getPath());
		}
	}

	void ModelRenderProcessor::deleteModelFromGpu(string modelPath) {
		Model3d& model = loader->getModelBy(modelPath);
		vector<Mesh3d>& meshes = model.getMeshes();

		unordered_map<uint32_t, string> texturesToRemove;
		for(auto& s : meshes) {
			string mName = model.getUniqueMeshName(s);
			deleteGeometryFromGpu(mName);
			
			Texture2d& texture = model.getTextureBy(s);
			texturesToRemove.emplace(s.getMaterialId(), texture.getPath());
		}

		for(auto& s : texturesToRemove) {
			deleteTextureFromGpu(s.second);
		}
	}
}
