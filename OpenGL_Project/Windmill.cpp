#include "Windmill.hpp"
#include <gtc/matrix_inverse.hpp>

namespace gps
{
	void Windmill::translate(glm::vec3 t)
	{
		this->windmillModelMatrix = glm::translate(this->windmillModelMatrix, t);
		this->bladesModelMatrix = glm::translate(this->bladesModelMatrix, t);
	}

	void Windmill::scale(glm::vec3 s)
	{
		this->windmillModelMatrix = glm::translate(this->windmillModelMatrix, s);
		this->bladesModelMatrix = glm::translate(this->bladesModelMatrix, s);
	}

	void Windmill::rotate(float angle, glm::vec3 r)
	{
		this->windmillModelMatrix = glm::rotate(this->windmillModelMatrix, angle, r);
		this->bladesModelMatrix = glm::rotate(this->bladesModelMatrix, angle, r);
	}

	void Windmill::rotateBlades(float deltaTime)
	{
		this->bladesRotationAngle += this->bladesRotationAngleStep * deltaTime;
		if (this->bladesRotationAngle > 360.0f)
		{
			this->bladesRotationAngle = 0.0f;
		}
		this->bladesModelMatrix = glm::rotate(this->bladesModelMatrix, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		this->bladesModelMatrix = glm::rotate(this->bladesModelMatrix, this->bladesRotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));
		this->bladesModelMatrix = glm::rotate(this->bladesModelMatrix, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	}


	void Windmill::draw(gps::Shader shader, glm::mat4 viewMatrix)
	{
		shader.useShaderProgram();
		shader.setMat4("model", this->bladesModelMatrix);
		glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(viewMatrix * this->bladesModelMatrix));
		shader.setMat3("normalMatrix", normalMatrix);
		blades.Draw(shader);

		shader.useShaderProgram();
		shader.setMat4("model", this->windmillModelMatrix);
		normalMatrix = glm::mat3(glm::inverseTranspose(viewMatrix * this->windmillModelMatrix));
		shader.setMat3("normalMatrix", normalMatrix);
		windmill.Draw(shader);
	}

	void Windmill::init(std::string windmill, std::string blades, std::string baseDir)
	{
		this->windmill = Model3D(windmill, baseDir);
		this->blades = Model3D(blades, baseDir);

		this->windmillModelMatrix = glm::mat4(1.0f);
		this->bladesModelMatrix = glm::mat4(1.0f);
	}


}
