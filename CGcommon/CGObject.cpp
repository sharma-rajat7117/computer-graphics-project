#include "CGObject.h"
namespace CGCommon
{
	CGObject::CGObject()
	{
	}

	CGObject::~CGObject()
	{
	}

	void CGObject::Draw()
	{
		glDrawElements(GL_TRIANGLES, this->Mesh.Indices.size(), GL_UNSIGNED_INT, (void*)((this->startIBO) * sizeof(unsigned int))); 
	}

	glm::mat4 CGObject::createTransform()
	{
		glm::mat4 localTransform = glm::mat4(1.0);
		localTransform = glm::rotate(localTransform, this->initialRotateAngle.x + this->rotateAngles.x, glm::vec3(1, 0, 0));
		localTransform = glm::rotate(localTransform, this->initialRotateAngle.y + this->rotateAngles.y, glm::vec3(0, 1, 0));
		localTransform = glm::rotate(localTransform, this->initialRotateAngle.z + this->rotateAngles.z, glm::vec3(0, 0, 1));
		localTransform = glm::translate(localTransform, this->initialTranslateVector + this->translateVector);
		localTransform = glm::scale(localTransform, this->initialScaleVector);

		glm::mat4 parentTransform = Parent == nullptr ? glm::mat4(1.0) : Parent->globalTransform;
		return parentTransform * localTransform;
	}
}