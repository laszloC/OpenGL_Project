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

		void randomize(int maxXOffset = 10, int maxYOffset = 10, float minScaleOffset = 0.9f, float maxScaleOffset = 1.2f);
		
		void draw(Shader shader, glm::mat4 view);

		Model3D model;
		std::vector<glm::mat4> modelMatrices;

	private:

		void initCluster(std::string filename, std::string basePath, int size);
			
	};

}