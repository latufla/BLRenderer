#pragma once
#include "../../utils/SharedHeaders.h"
#include <vector>
#include <memory>
#include "../Mesh3d.h"
#include "../Material3d.h"
#include "../Animation3d.h"
#include "../../utils/bones/BNode.h"
#include "../../utils/bones/BoneNodeData.h"

namespace br {
	class IModel3d {
	public:
		virtual std::string getPath() const = 0;

		virtual std::vector<Mesh3d>& getMeshes() = 0;
		virtual std::shared_ptr<Mesh3d> getHitMesh() = 0;

		virtual std::vector<Material3d>& getMaterials() = 0;

		virtual Material3d& getMaterialBy(Mesh3d const&) = 0;
		virtual Texture2d& getTextureBy(Mesh3d const&) = 0;

		virtual void addAnimation(Animation3d const&) = 0;
		virtual Animation3d& getAnimationBy(std::string name) = 0;
		
		virtual BNode<BoneNodeData>& getBoneTree() = 0;

		virtual std::string getUniqueMeshName(const Mesh3d&) = 0;
	};
}