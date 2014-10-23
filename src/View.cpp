#include "stdafx.h"
#include "View.h"

using std::string;

View::View() {
}

View::View(uint32_t id, string name, string path) {
	this->id = id;
	this->name = name;
	this->path = path;
}

View::~View() {
}

bool View::doAnimationStep(uint32_t stepMSec) {
	animationTime += stepMSec; 
	if (animationTime <= animationDuration)
		return true;

	if (loopAnimation) {
		animationTime = 0;
		return true;
	}
		
	return false;
}

void View::setAnimation(std::string animation, uint32_t durationMSec, bool loop) {
	this->animation = animation; 
	animationDuration = durationMSec;
	loopAnimation = loop;
}
