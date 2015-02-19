#include "ModelMouseProcessor.h"
#include "..\..\View.h"
#include "..\..\exceptions\Exception.h"

using std::pair;
using std::vector;
using std::string;
using std::shared_ptr;

using std::numeric_limits;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::cross;
using glm::dot;
using glm::inverse;
using glm::normalize;

namespace br {
	const float ModelMouseProcessor::MAX_DISTANCE = numeric_limits<float>::max();

	ModelMouseProcessor::ModelMouseProcessor(shared_ptr<AssetLoader>loader, pair<string, string> shaders) 
		: ProcessorBase(loader, shaders){	
	}
	
	ModelMouseProcessor::~ModelMouseProcessor() {
	}

	void ModelMouseProcessor::start(std::weak_ptr<IGraphicsConnector> graphics) {
		this->graphics = graphics;
		enabled = true;
	}

	void ModelMouseProcessor::stop() {
		enabled = false;
		mouseOver = -1;
	}

	void ModelMouseProcessor::doStep(const StepData& stepData) {
		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		auto wMousePos = sGConnector->getMousePosition();
		auto wSize = sGConnector->getWindowSize();

		vec2 mouseNDC = vec2{wMousePos.x, wMousePos.y} / vec2{wSize.w, wSize.h} * 2.0f - 1.0f;
		mouseNDC.y = -mouseNDC.y;

		mat4 toWorld = inverse(stepData.perspectiveView);

		vec4 from = toWorld * vec4(mouseNDC.x, mouseNDC.y, -1.0f, 1.0f);
		vec4 to = toWorld * vec4(mouseNDC.x, mouseNDC.y, 1.0f, 1.0f);
		from /= from.w;
		to /= to.w;
		
		vec3 start{from};
		vec3 ray{to.x - from.x, to.y - from.y, to.z - from.z};
		ray = normalize(ray);

		std::pair<View*, float> minDistData{nullptr, MAX_DISTANCE};
		auto objects = (std::unordered_map<uint32_t, View>*) stepData.extraData;
		for(auto& i : *objects) {
			View& object = i.second;

			Model3d& model = loader->getModelBy(object.getPath());
			auto& mesh = *model.getHitMesh();			
			
			BoneTransformer::BonesDataMap bonesData;
			auto& boneIdToOffset = mesh.getBoneIdToOffset();
			for(auto& i : boneIdToOffset) {
				BoneTransformer::BoneData bData{i.second};
				bonesData.emplace(i.first, bData);
			}
			boneTransformer.transform(object, model, bonesData);
			
			auto indices = mesh.getIndices();
			auto vertices = mesh.getVertices();
			uint32_t n = indices.size() - 3;
			for(uint32_t i = 0; i <= n; i += 3) {
				vector<vec3> vcs;
				for(uint8_t j = 0; j < 3; j++) {
					auto v = vertices.at(indices[i + j]);
					mat4 boneTransform{0.0f};
					for(uint8_t k = 0; k < 4; k++) {
						auto boneId = v.boneIds[k];
						auto bData = bonesData[boneId];
						boneTransform += bData.finalTransform * v.weigths[k];
					}
					auto vx = object.getTransform() * boneTransform * vec4{v.x, v.y, v.z, 1.0f};
					vx /= vx.w;
					vcs.push_back({vx.x, vx.y, vx.z});
				}

				Triangle triangle{vcs[0], vcs[1], vcs[2]};
	
				float distance = calcDistance(start, ray, triangle);
				if(distance < minDistData.second) {
					minDistData.first = &object;
					minDistData.second = distance;
				}
			}
		}
	
		mouseOver = minDistData.second != MAX_DISTANCE ? minDistData.first->getId() : -1;
	}

	float ModelMouseProcessor::calcDistance(vec3& start, vec3& dir, Triangle& triangle) {
		vec3 trCros = cross(triangle.b - triangle.a, triangle.c - triangle.a);

		// intersects with plane
		float pln = dot(trCros, triangle.c - start); // whatever point
		float ln = dot(trCros, dir);
		if(!ln) {
			if(!pln)
				return MAX_DISTANCE; // in plane

			return MAX_DISTANCE; // ||
		}

		float d = pln / ln;
		vec3 iwp{start + dir * d};

		vec3 v0 = triangle.c - triangle.a;
		vec3 v1 = triangle.b - triangle.a;
		vec3 v2 = iwp - triangle.a;

		float dot00 = dot(v0, v0);
		float dot01 = dot(v0, v1);
		float dot02 = dot(v0, v2);
		float dot11 = dot(v1, v1);
		float dot12 = dot(v1, v2);
	
		// Compute barycentric coordinates
		float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
		float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
		
		bool isInsideTriangle = (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
		return isInsideTriangle ? glm::distance(iwp, start) : MAX_DISTANCE;
	}
}
