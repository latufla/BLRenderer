#include "../../utils/SharedHeaders.h"
#include "ModelRenderProcessor.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../../Model3d.h"
#include "../../Texture2d.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "../../exceptions/Exception.h"

using std::shared_ptr;
using std::pair;
using std::vector;
using std::array;
using std::string;
using glm::mat4;
using glm::make_mat4;

namespace br {
	ModelRenderProcessor::ModelRenderProcessor(shared_ptr<AssetLoader>loader, pair<std::string, std::string> shaders) 
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
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, id);
		}

		string modelPath = object->getPath();
		idToObject.erase(id);

		if(!hasObjectWithModel(modelPath)) // last out
			deleteModelFromGpu(modelPath);
	}

	void ModelRenderProcessor::playAnimation(uint32_t objId, std::string animName, bool loop) {
		View* object;
		try {
			object = &idToObject.at(objId);
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, objId);
		}

		Model3d& model = loader->getModelBy(object->getPath());
		Animation3d& animation = model.getAnimationBy(animName);
		object->playAnimation(animation, loop);
	}

	void ModelRenderProcessor::stopAnimation(uint32_t objId, std::string animName) {
		View* object;
		try {
			object = &idToObject.at(objId);
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, objId);
		}

		Model3d& model = loader->getModelBy(object->getPath());
		Animation3d& animation = model.getAnimationBy(animName);
		object->stopAnimation(animation);
	}

	void ModelRenderProcessor::transformObject(uint32_t objId, const array<float, 16> tForm) {
		View* object;
		try {
			object = &idToObject.at(objId);
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, objId);
		}

		auto t = make_mat4(tForm.data());
		object->setTransform(t);
	}
	
	ProcessorBase::ProgramContext ModelRenderProcessor::fillProgramContext(uint32_t pObject) {
		ProgramContext program = __super::fillProgramContext(pObject);

		program.position = glGetAttribLocation(pObject, "aPosition");
		program.texPosition = glGetAttribLocation(pObject, "aTexCoord");

		program.bones = glGetUniformLocation(pObject, "bones");
		program.boneIds = glGetAttribLocation(pObject, "boneIds");
		program.weights = glGetAttribLocation(pObject, "weights");

		program.sampler = glGetUniformLocation(pObject, "sTexture");
		program.mvpMatrix = glGetUniformLocation(pObject, "mvpMatrix");

		return program;
	}

	void ModelRenderProcessor::doStep(StepData& stepData) {
		glUseProgram(program.id);

		for(auto& i : idToObject) {
			View& object = i.second;
			mat4 mvpMatrix = stepData.perspectiveView * object.getTransform();
			glUniformMatrix4fv(program.mvpMatrix, 1, GL_FALSE, &mvpMatrix[0][0]);

			Model3d& model = loader->getModelBy(object.getPath());
			vector<Mesh3d>& meshes = model.getMeshes();
			for(auto& s : meshes) {
				auto bonesData = prepareAnimationStep(object, s, stepData.stepMSec);
				for(auto& i : bonesData) {
					glUniformMatrix4fv(program.bones + i.first, 1, GL_FALSE, &(i.second.finalTransform[0][0]));
				}

				string meshName = model.getUniqueMeshName(s);
				GpuBufferData& buffers = meshToBuffer.at(meshName);
				glBindBuffer(GL_ARRAY_BUFFER, buffers.vBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.iBuffer);

				uint8_t offset = 0;
				glEnableVertexAttribArray(program.position);
				glVertexAttribPointer(program.position, Mesh3d::VERTEX3D_POSITION, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);

 				offset += Mesh3d::VERTEX3D_POSITION * sizeof(float);
				glEnableVertexAttribArray(program.texPosition);
				glVertexAttribPointer(program.texPosition, Mesh3d::VERTEX3D_TEXTURE, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);

				offset += Mesh3d::VERTEX3D_TEXTURE * sizeof(float);
				glEnableVertexAttribArray(program.boneIds);
				glVertexAttribPointer(program.boneIds, Mesh3d::VERTEX3D_BONEIDS, GL_UNSIGNED_SHORT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);

				offset += Mesh3d::VERTEX3D_BONEIDS * sizeof(uint16_t);
				glEnableVertexAttribArray(program.weights);
				glVertexAttribPointer(program.weights, Mesh3d::VERTEX3D_WEIGHTS, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, buffers.texture);

				glUniform1i(program.sampler, 0);

				glDrawElements(GL_TRIANGLES, buffers.iBufferLenght, GL_UNSIGNED_SHORT, (void*)0);
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(program.position);
		glDisableVertexAttribArray(program.texPosition);
		glDisableVertexAttribArray(program.boneIds);
		glDisableVertexAttribArray(program.weights);

		glUseProgram(0);
	}

	BoneTransformer::BonesDataMap ModelRenderProcessor::prepareAnimationStep(View& object, Mesh3d& m, long long stepMSec) {
		BoneTransformer::BonesDataMap res;
		auto& boneIdToOffset = m.getBoneIdToOffset();
		for(auto& i : boneIdToOffset) {
			BoneTransformer::BoneData bData{i.second};
			res.emplace(i.first, bData);
		}

		object.doAnimationStep(stepMSec);

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
			uint32_t vBuffer;
			glGenBuffers(1, &vBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vBuffer);

			auto vertices = s.getVertices();
			GLint szInBytes = sizeof(Vertex3d) * vertices.size();
			glBufferData(GL_ARRAY_BUFFER, szInBytes, &vertices[0], GL_STATIC_DRAW);

			GLint loadedBytes = 0;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &loadedBytes);
			if(szInBytes != loadedBytes) {
				glDeleteBuffers(1, &vBuffer);
				throw GpuException(EXCEPTION_INFO, modelPath + " can`t load vertices");
			}


			uint32_t iBuffer;
			glGenBuffers(1, &iBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);

			auto indices = s.getIndices();
			uint32_t iBufferLength = indices.size();
			szInBytes = sizeof(uint16_t) * iBufferLength;
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, szInBytes, &indices[0], GL_STATIC_DRAW);

			glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &loadedBytes);
			if(szInBytes != loadedBytes) {
				glDeleteBuffers(1, &iBuffer);
				throw GpuException(EXCEPTION_INFO, modelPath + " can`t load indices");
			}


			auto& materials = model.getMaterials();
			Material3d& m = materials.at(s.getMaterialId());
			Texture2d& texture = m.getTexture();
			loadTextureToGpu(texture);

			uint32_t textureId = textureToId.at(texture.getPath());
			string meshName = model.getUniqueMeshName(s);
			GpuBufferData buffer{vBuffer, iBuffer, iBufferLength, textureId};
			meshToBuffer.emplace(meshName, buffer);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void ModelRenderProcessor::deleteModelFromGpu(string modelPath) {
		Model3d& model = loader->getModelBy(modelPath);
		vector<Mesh3d>& meshes = model.getMeshes();
		for(auto& s : meshes) {
			string mName = model.getUniqueMeshName(s);
			GpuBufferData& buffers = meshToBuffer.at(mName);

			glDeleteBuffers(1, &buffers.vBuffer);
			glDeleteBuffers(1, &buffers.iBuffer);

			auto& materials = model.getMaterials();
			Material3d& m = materials.at(s.getMaterialId());
			Texture2d& texture = m.getTexture();
			deleteTextureFromGpu(texture.getPath());

			meshToBuffer.erase(mName);
		}
	}
}
