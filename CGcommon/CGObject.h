#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
//#include "Mesh.h"
#include "..\Dependencies\OBJ_loader.h"

namespace CGCommon
{
	class CGObject
	{
	public:
		CGObject();
		~CGObject();

		objl::Mesh Mesh;

		int startVBO = 0;
		int startIBO = 0;

		// Initial transform
		glm::vec3 initialTranslateVector = glm::vec3(0, 0, 0);
		glm::vec3 initialScaleVector = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 initialRotateAngle = glm::vec3(0, 0, 0);

		// Transform vectors
		glm::vec3 translateVector = glm::vec3(0, 0, 0);
		glm::vec3 scaleVector = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 rotateAngles = glm::vec3(0, 0, 0);

		// Color
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // White

		// Current position
		glm::mat4 globalTransform = glm::mat4(1.0);
		
		CGObject *Parent = nullptr;

		void Draw();
		glm::mat4 createTransform();
	};
}
