#pragma once
#include <glm.hpp>

namespace br {
	class View {
	public:
		View() = delete;
		View(uint32_t id, std::string path);
	
		~View() = default;

		uint32_t getId() const { return id; }
		std::string getPath() const { return path; }
	
		glm::mat4& getTransform() { return transform; }
		void setTransform(const glm::mat4& val) { transform = val; }
	
		void setAnimation(std::string animName, uint32_t durationMSec, bool loop = false);
	
		std::string getAnimationName() const { return animation.name;  }
		uint32_t getAnimationTime() const { return animation.timeMSec; }
	
		bool doAnimationStep(uint32_t stepMSec);
	
	private:
		uint32_t id;
		std::string path;
		
		glm::mat4 transform;
	
		struct AnimationData {
			std::string name;
			bool loop;
			uint32_t durationMSec = 0;
			uint32_t timeMSec = 0;
		} animation;
	};	
}
