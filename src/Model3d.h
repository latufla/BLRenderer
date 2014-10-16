#pragma once
#include <vector>
#include "Mesh3d.h"
#include "Material3d.h"

class Model3d {
public:
	Model3d();
	Model3d(std::string, const std::vector<Mesh3d>&, const std::vector<std::string>& materials);

	~Model3d();

	std::string getName() const { return name; }
	std::vector<Mesh3d>& getMeshes() { return meshes; }
	std::vector<Material3d>& getMaterials() { return materials; }

	operator std::string() const;

	std::string getUniqueMeshName(const Mesh3d&);

private:
	std::string name;
	
	std::vector<Mesh3d> meshes;
	std::vector<Material3d> materials;
};

