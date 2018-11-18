#pragma once

#include <stdlib.h>
#include "CGobject.h"

namespace Physics
{
	// Physics
	bool gravity = true;
//	float coef = 0.8f;  // coefficient of restitution

	float forces(int j)
	{
		if (!gravity)
			return (0.0);
		else if (j == 1)    // y-axis
			return (-4.9f);
		else
			return (0.0);
	}

	void collision(CGCommon::CGObject* sceneObject)
	{
		for (int i = 0; i < 3; i++)
		{
			if (sceneObject->translateVector[i] >= 10.0)
			{
				sceneObject->velocity[i] = -sceneObject->coef * sceneObject->velocity[i];
				sceneObject->translateVector[i] = 10.0 - sceneObject->coef * (sceneObject->translateVector[i] - 10.0);
			}

			if (sceneObject->translateVector[i] <= -1.0)
			{
				sceneObject->velocity[i] = -sceneObject->coef * sceneObject->velocity[i];
				sceneObject->translateVector[i] = -1.0 - sceneObject->coef * (sceneObject->translateVector[i] + 1.0);
			}
		}
	}

	void updatePhysics(float deltaTime, CGCommon::CGObject* sceneObject)
	{
		for (int j = 0; j < 3; j++)  // x, y, z directions
		{
			sceneObject->translateVector[j] += deltaTime * sceneObject->velocity[j];
			sceneObject->velocity[j] += deltaTime * forces(j) / sceneObject->mass;
		}

		collision(sceneObject);
	}
}