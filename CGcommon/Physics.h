#pragma once
#include <iostream>
using namespace std;
#include <stdlib.h>
#include "CGobject.h"

namespace Physics
{
	// Physics
	bool gravity = true;
	//	float coef = 0.8f;  // coefficient of restitution
	float lastcollisionV[3] = { 0.0f };
	bool staticsflag = false;
	float forces(int j)
	{
		if (!gravity)
			return (0.0);
		else if (j == 1)    // y-axis
			return (-1.0f);
		else
			return (0.0);
	}

	void collision(CGCommon::CGObject* sceneObject)
	{
		for (int i = 0; i < 3; i++)
		{
			if (sceneObject->translateVector[i] >= 1.0)
			{
				
				sceneObject->velocity[i] = -sceneObject->coef * sceneObject->velocity[i];
				sceneObject->translateVector[i] = 1.0 - sceneObject->coef * (sceneObject->translateVector[i] - 1.0);
				
			
			}

			if (sceneObject->translateVector[i] <= -1.0)
			{
				
				if (fabs(fabs(sceneObject->velocity[1]) - fabs(-sceneObject->coef * sceneObject->velocity[1])) > 0.06) 
				{
					sceneObject->velocity[i] = -sceneObject->coef * sceneObject->velocity[i];
					sceneObject->translateVector[i] = -1.0 ;
					cout << sceneObject->velocity[i] << endl;

				}
				else
				{
					sceneObject->velocity[i] = 0;
					sceneObject->translateVector[i] = -1.0;
					//gravity = false;
				}
			
			}
			lastcollisionV[i] = sceneObject->velocity[i];
		}
	}

	void updatePhysics(float deltaTime, CGCommon::CGObject* sceneObject)
	{
		if (sceneObject->translateVector[1] <= -1.0 && fabs(fabs(sceneObject->velocity[1]) - fabs(-sceneObject->coef * sceneObject->velocity[1])) < 0.02)
		{
		}
		else
		{
			for (int j = 0; j < 3; j++)  // x, y, z directions
			{
				sceneObject->translateVector[j] += deltaTime * sceneObject->velocity[j];
				sceneObject->velocity[j] += deltaTime * forces(j) / sceneObject->mass;
			}
		}
		collision(sceneObject);
	}
}