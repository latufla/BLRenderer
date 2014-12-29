#include "utils/SharedHeaders.h"
#include "View.h"

using std::string;

namespace br {
	View::View(uint32_t id, string path) 
		: id(id),
		path(path){
	}
	
	bool View::doAnimationStep(long long stepMSec) {
		if(!animation.play)
			return false;

		animation.timeMSec += stepMSec; 
		if (animation.timeMSec <= animation.durationMSec)
			return true;
	
		if (animation.loop) {
			animation.timeMSec -= animation.durationMSec;
			return true;
		}
			
		return false;
	}
	
	void View::playAnimation(Animation3d& animation, bool loop) {
		this->animation.play = true;
		this->animation.name = animation.getName();
		this->animation.durationMSec = (long long)(animation.getDuration() * 1000);
		this->animation.loop = loop;
	}

	void View::stopAnimation(Animation3d& animation) {
		this->animation.play = false;
		this->animation.name = animation.getName();
		this->animation.durationMSec = (long long)(animation.getDuration() * 1000);
		this->animation.timeMSec = 0;
		this->animation.loop = false;
	}
}
