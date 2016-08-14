#pragma once

#include "math.h"

namespace cfc {
namespace math {
namespace intersection {

	struct point;
	struct ray;
	struct aabb;

	struct CFC_API point : math::vector3f
	{};
			
	struct CFC_API ray
	{
		point start, end;
		bool Intersect(aabb& gpu_box, float& depth);
	};

	struct CFC_API aabb
	{
		point min, max;
		bool Intersect(ray& line, float& depth);
	};

}; // end namespace intersection
}; // end namespace math
}; // end namespace cfc