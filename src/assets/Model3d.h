#pragma once
#include "interfaces/IModel3d.h"

namespace br {
	class Model3d : public IModel3d {
	public:
		Model3d(std::string path,
			std::vector<Mesh3d> const& meshes,
			std::vector<Material3d> const& materials,
			BNode<BoneNodeData> const& boneTree,
			Animation3d const& defaultAnimation);

		virtual std::string getPath() const override;

		virtual std::vector<Mesh3d>& getMeshes() override;
		virtual std::shared_ptr<Mesh3d> getHitMesh() override;

		virtual std::vector<Material3d>& getMaterials() override;

		virtual BNode<BoneNodeData>& getBoneTree() override;

		virtual Material3d& getMaterialBy(Mesh3d const&) override;
		virtual Texture2d& getTextureBy(Mesh3d const&) override;

		virtual void addAnimation(Animation3d const&) override;
		virtual Animation3d& getAnimationBy(std::string name) override;
		
		virtual std::string getUniqueMeshName(const Mesh3d&) override;
	private:
		void initHitMesh(std::vector<Mesh3d>&);

		std::string path;
		
		std::vector<Mesh3d> meshes;
		std::vector<Material3d> materials;
	
		std::shared_ptr<Mesh3d> hitMesh;
		
		BNode<BoneNodeData> boneTree;
	
		std::unordered_map<std::string, Animation3d> nameToAnimation;
	};
}

