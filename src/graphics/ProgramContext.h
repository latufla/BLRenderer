#pragma once
#include "interfaces\IProgramContext.h"
#include <unordered_map>

namespace br {
	class ProgramContext : public IProgramContext{
	public:
		ProgramContext(int32_t id);
		virtual ~ProgramContext();

		virtual int32_t getId() const override;

		virtual std::string getPositionBinding() const override;
		virtual std::string getUvBinding() const override;

		virtual std::string getSamplerBinding() const override;

		virtual std::string getMvpBinding() const override;
		virtual std::string getBonesBinding() const override;

		virtual std::string getBoneIdsBinding() const override;
		virtual std::string getBoneWeightsBinding() const override;

		virtual int32_t getLoc(std::string name) override;
		virtual void setLoc(std::string name, int32_t val) override;

	protected:
		int32_t id = -1;

		std::unordered_map<std::string, int32_t> nameToLoc;
	};
}

