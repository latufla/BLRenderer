#pragma once
#include "../../utils/SharedHeaders.h"

namespace br {
	class IProgramContext {
	public:
		virtual int32_t getId() const = 0;
		
		virtual std::string getPositionBinding() const = 0;
		virtual std::string getUvBinding() const = 0;
		
		virtual std::string getSamplerBinding() const = 0;
		
		virtual std::string getMvpBinding() const = 0;
		virtual std::string getBonesBinding() const = 0;
		virtual std::string getBoneIdsBinding() const = 0;
		virtual std::string getBoneWeightsBinding() const = 0;
		
		virtual int32_t getLoc(std::string name) = 0;
		virtual void setLoc(std::string name, int32_t val) = 0;
	};
}

