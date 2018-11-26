#pragma once
#include <iostream>
using namespace std;
#include <stdlib.h>
#include "CGobject.h"

namespace Physics
{
	// Physics
	bool gravity = true;	
	bool staticsflag = false;
	float thresholdGround = 0.8f;
	float thresholdVelocity = 4.0f;
	float forces(int j)
	{
		if (!gravity)
			return (0.0);
		else if (j == 1)    // y-axis
			return (-9.81f);
		else
			return (0.0);
	}

	void collision(float deltaTime, CGCommon::CGObject* sceneObject)
	{
		for (int i = 0; i < 3; i++)
		{
			if (sceneObject->position[i] >= 10.0)
			{
				sceneObject->velocity[i] = -sceneObject->coef * sceneObject->velocity[i];				
				sceneObject->position[i] = 10.0 - sceneObject->coef * ( sceneObject->position[i] - 10.0);
			}

			if (sceneObject->position[i] <= 0)
			{
				if (i==1 && fabs(sceneObject->position[i]) < thresholdGround && fabs(sceneObject->velocity[i]) < thresholdVelocity)
				{					
					sceneObject->position[i] = 0;
					sceneObject->velocity[i] = 0;
					return;
				}

				sceneObject->velocity[i] = -sceneObject->coef * sceneObject->velocity[i];
				sceneObject->position[i] = - sceneObject->coef * sceneObject->position[i];
			}
		}
	}

	void updatePhysics(float deltaTime, CGCommon::CGObject* sceneObject)
	{
		/*if (sceneObject->translateVector[1] <= -1.0 && fabs(fabs(sceneObject->velocity[1]) - fabs(-sceneObject->coef * sceneObject->velocity[1])) < 0.02)
		{
		}
		else
		{*/
		for (int j = 0; j < 3; j++)  // x, y, z directions
		{
			sceneObject->position[j] += deltaTime * sceneObject->velocity[j];
			sceneObject->velocity[j] += (deltaTime * forces(j)) / sceneObject->mass;
		}
		//}

		collision(deltaTime, sceneObject);
	}
}