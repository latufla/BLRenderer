#include "stdafx.h"
#include "View.h"

using std::string;

View::View(uint32_t id, string name, string path) {
	this->id = id;
	this->name = name;
	this->path = path;
}

bool View::doAnimationStep(uint32_t stepMSec) {
	animation.timeMSec += stepMSec; 
	if (animation.timeMSec <= animation.durationMSec)
		return true;

	if (animation.loop) {
		animation.timeMSec = 0;
		return true;
	}
		
	return false;
}

void View::setAnimation(string name, uint32_t durationMSec, bool loop) {
	animation.name = name; 
	animation.durationMSec = durationMSec;
	animation.loop = loop;
}
