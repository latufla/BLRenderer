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
