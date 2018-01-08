#include "TreeCluster.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <ctime>
#include <gtc/matrix_inverse.hpp>


namespace gps
{
#define MIN_ROT_OFFSET (-180.0f)
#define MAX_ROT_OFFSET (180.0f)

	TreeCluster::TreeCluster()
	{
	}

	TreeCluster::TreeCluster(std::string filename, std::string basePath, int size)
	{
		initCluster(filename, basePath, size);
	}

	void TreeCluster::translate(glm::vec3 t)
	{
		int size = this->modelMatrices.size();
		for (int i = 0; i < size; ++i)
		{
			this->modelMatrices[i] = glm::translate(this->modelMatrices[i], t);
		}
	}

	void TreeCluster::scale(glm::vec3 s)
	{
		int size = this->modelMatrices.size();
		for (int i = 0; i < size; ++i)
		{
			this->modelMatrices[i] = glm::scale(this->modelMatrices[i], s);
		}
	}

	void TreeCluster::rotate(float angle, glm::vec3 r)
	{
		int size = this->modelMatrices.size();
		for (int i = 0; i < size; ++i)
		{
			this->modelMatrices[i] = glm::rotate(this->modelMatrices[i], glm::radians(angle), r);
		}
	}


	void TreeCluster::initCluster(std::string filename, std::string basePath, int size)
	{
		for (int i = 0; i < size; ++i)
		{
			Model3D tree = Model3D(filename, basePath);
			this->models.push_back(tree);
			this->modelMatrices.emplace_back(1.0f);
		}
	}

	void TreeCluster::randomize(int minPosOffset, int maxPosOffset, float minScaleOffset, float maxScaleOffset)
	{
		srand(time(NULL));

		int size = this->modelMatrices.size();
		for (int i = 0; i < size; ++i)
		{
			int interval = maxPosOffset - minPosOffset;
			int randPosX = rand() % interval + minPosOffset;
			int randPosZ = rand() % interval + minPosOffset;
			float randScale = (rand() / (float)RAND_MAX * (maxScaleOffset - minScaleOffset)) + minScaleOffset;
			float randRot = (rand() / (float)RAND_MAX * (MAX_ROT_OFFSET - MIN_ROT_OFFSET)) + MIN_ROT_OFFSET;
			this->modelMatrices[i] = glm::scale(this->modelMatrices[i], glm::vec3(randScale, randScale, randScale));
			this->modelMatrices[i] = glm::translate(this->modelMatrices[i], glm::vec3(randPosX, 0.0f, randPosZ));
			this->modelMatrices[i] = glm::rotate(this->modelMatrices[i], glm::radians(randRot), glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	void TreeCluster::draw(Shader shader, glm::mat4 view)
	{
		shader.useShaderProgram();
		int size = this->models.size();
		for (int i = 0; i < size; ++i)
		{
			shader.setMat4("model", this->modelMatrices[i]);
			shader.setMat3("normalMatrix", glm::mat3(glm::inverseTranspose(view * this->modelMatrices[i])));
			models[i].Draw(shader);
		}
	}
}
