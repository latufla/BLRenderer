#include "../../utils/SharedHeaders.h"
#include "ModelRenderProcessor.h"

#include "../../assets/Model3d.h"
#include "../../assets/Texture2d.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "../../exceptions/Exception.h"
#include "../../assets/AssetLoader.h"

using std::shared_ptr;
using std::weak_ptr;
using std::pair;
using std::vector;
using std::unordered_map;
using std::string;
using std::out_of_range;

using glm::mat4;
using glm::make_mat4;

namespace br {
	ModelRenderProcessor::ModelRenderProcessor(shared_ptr<IAssetLoader>loader) 
		: ProcessorBase(loader){
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

		auto model = loader->getModelBy(object->getPath());
		Animation3d& animation = model->getAnimationBy(animName);
		object->playAnimation(animation, loop);
	}

	void ModelRenderProcessor::stopAnimation(uint32_t objId, string animName) {
		View* object;
		try {
			object = &idToObject.at(objId);
		} catch(out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, objId);
		}

		auto model = loader->getModelBy(object->getPath());
		Animation3d& animation = model->getAnimationBy(animName);
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
			object.doAnimationStep(stepData.stepMSec);

			mat4 mvpMatrix = stepData.perspectiveView * object.getTransform();

			auto model = loader->getModelBy(object.getPath());
			vector<Mesh3d>& meshes = model->getMeshes();
			for(auto& s : meshes) {
				auto material = model->getMaterialBy(s);
				auto programName = material.getProgramName();
				auto programContext = nameToProgramContext.at(programName);

				std::vector<IGraphicsConnector::ProgramParam> params;
				IGraphicsConnector::ProgramParam mvp;
				mvp.id = programContext.mvp;
				mvp.mat4 = std::make_shared<glm::mat4>(mvpMatrix);
				params.push_back(mvp);

				auto bonesData = prepareAnimationStep(object, s);			
				string meshName = model->getUniqueMeshName(s);
				auto& buffer = meshToBuffer.at(meshName);

				sGConnector->draw(buffer, programContext, params, bonesData);
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

		auto model = loader->getModelBy(object.getPath());
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
		auto model = loader->getModelBy(modelPath);
		auto materials = model->getMaterials();
		for(auto& i : materials) {
			auto program = loader->getProgramBy(i.getProgramName());
			auto programName = program->getName();
			if(!hasMaterialWithProgram(programName))
				loadProgramToGpu(programName, program->getVertexShader(), program->getFragmentShader());
		}

		vector<Mesh3d>& meshes = model->getMeshes();
		for(auto& s : meshes) {
			string meshName = model->getUniqueMeshName(s);

			loadGeometryToGpu(meshName, s.getRawVertices(), s.getIndices());

			Texture2d& texture = model->getTextureBy(s);
			loadTextureToGpu(texture);

			auto& buffer = meshToBuffer.at(meshName);
			buffer.texture = textureToId.at(texture.getPath());
		}
	}

	void ModelRenderProcessor::deleteModelFromGpu(string modelPath) {
		auto model = loader->getModelBy(modelPath);
		auto materials = model->getMaterials();
		for(auto& i : materials) {
			auto program = loader->getProgramBy(i.getProgramName());
			auto programName = program->getName();
			
			if(!hasMaterialWithProgram(programName))
				deleteProgramFromGpu(programName);
		}

		vector<Mesh3d>& meshes = model->getMeshes();
		unordered_map<uint32_t, string> texturesToRemove;
		for(auto& s : meshes) {
			string mName = model->getUniqueMeshName(s);
			deleteGeometryFromGpu(mName);
			
			Texture2d& texture = model->getTextureBy(s);
			texturesToRemove.emplace(s.getMaterialId(), texture.getPath());
		}

		for(auto& s : texturesToRemove) {
			deleteTextureFromGpu(s.second);
		}
	}

	bool ModelRenderProcessor::hasMaterialWithProgram(std::string name) {
		for(auto& i : idToObject) {
			View& object = i.second;
			auto model = loader->getModelBy(object.getPath());
			auto materials = model->getMaterials();
			for(auto& j : materials) {
				if(j.getProgramName() == name)
					return true;
			}
		}
		return false;
	}
}
