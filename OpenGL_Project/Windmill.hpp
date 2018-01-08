#pragma once
#include "Camera.hpp"
#include "Shader.hpp"
#include "Model3D.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace gps
{
	class Windmill
	{
	public:

		Windmill()
		{
			
		}

		Windmill(std::string windmill, std::string blades, std::string baseDir)
		{
			this->init(windmill, blades, baseDir);
		}

		void translate(glm::vec3 t);

		void scale(glm::vec3 s);

		void rotate(float angle, glm::vec3 r);

		void rotateBlades(float deltaTime);

		void draw(gps::Shader shader, glm::mat4 viewMatrix);

	private:
				
		float bladesRotationAngle = 0.0f;
		float bladesRotationAngleStep = 0.001f;

		gps::Model3D windmill;
		gps::Model3D blades;

		glm::mat4 windmillModelMatrix;
		glm::mat4 bladesModelMatrix;

		void init(std::string windmill, std::string blades, std::string baseDir);

	public:
		void set_blades_rotation_angle(float blades_rotation_angle)
		{
			bladesRotationAngle = blades_rotation_angle;
		}
	};

}
