#pragma once
#include "Model3D.hpp"
#include "Shader.hpp"

namespace gps {

	class TreeCluster
	{
	public:

		TreeCluster();

		TreeCluster(std::string filename, std::string basePath, int size);
		

		void translate(glm::vec3 t);
		void scale(glm::vec3 s);
		void rotate(float angle, glm::vec3 r);

		void randomize(int minPosOffset = -10, int maxPosOffset = +10, float minScaleOffset = 0.9f, float maxScaleOffset = 1.2f);
		
		void TreeCluster::draw(Shader shader, glm::mat4 view);

		std::vector<Model3D> models;
		std::vector<glm::mat4> modelMatrices;

	private:

		void initCluster(std::string filename, std::string basePath, int size);
			
	};

}