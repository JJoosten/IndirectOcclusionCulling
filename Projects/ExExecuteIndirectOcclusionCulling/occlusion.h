#pragma once

#include <cfc/base.h>

struct aabb
{
	union
	{
		struct
		{
			float MinX, MinY, MinZ;
		};
		float Min[3];
	};
	union
	{
		struct
		{
			float MaxX, MaxY, MaxZ;
		};
		float Max[3];
	};
};

