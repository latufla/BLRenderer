#pragma once
#include <glm.hpp>
#include "Animation3d.h"

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
	
		void playAnimation(Animation3d&, bool loop = false);
		void stopAnimation(Animation3d&);
	
		std::string getAnimationName() const { return animation.name;  }
		long long getAnimationTime() const { return animation.timeMSec; }
	
		bool doAnimationStep(long long stepMSec);
	
	private:
		uint32_t id;
		std::string path;
		
		glm::mat4 transform;
	
		struct AnimationData {
			std::string name;
			bool play;
			bool loop;
			long long durationMSec = 0;
			long long timeMSec = 0;
		} animation;
	};	
}
