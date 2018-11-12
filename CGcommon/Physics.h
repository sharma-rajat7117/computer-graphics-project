#pragma once
#include <stdlib.h>

namespace Physics
{
	// Physics
	bool gravity = true;
	float coef = 0.8f;  // coefficient of restitution

	float forces(int i, int j)
	{
		if (!gravity)
			return (0.0);
		else if (j == 1)    // y-axis
			return (-1.0);
		else
			return (0.0);
	}
}