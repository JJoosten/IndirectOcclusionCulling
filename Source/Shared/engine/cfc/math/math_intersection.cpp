#include <cfc/base.h>
#include <cfc/math/math_intersection.h>

bool cfc::math::intersection::ray::Intersect(aabb& gpu_box, float& fst)
{
	fst = 0.0f;
	float st, et, fet = 1;

	for (int i = 0; i < 3; i++) {
		if (start.V[i] < end.V[i]) {
			if (start.V[i] > gpu_box.max.V[i] || end.V[i] < gpu_box.min.V[i])
				return false;
			float di = end.V[i] - start.V[i];
			st = (start.V[i] < gpu_box.min.V[i]) ? (gpu_box.min.V[i] - start.V[i]) / di : 0;
			et = (end.V[i] > gpu_box.max.V[i]) ? (gpu_box.max.V[i] - start.V[i]) / di : 1;
		}
		else {
			if (end.V[i] > gpu_box.max.V[i] || start.V[i] < gpu_box.min.V[i])
				return false;
			float di = end.V[i] - start.V[i];
			st = (start.V[i] > gpu_box.max.V[i]) ? (gpu_box.max.V[i] - start.V[i]) / di : 0;
			et = (end.V[i] < gpu_box.min.V[i]) ? (gpu_box.min.V[i] - start.V[i]) / di : 1;
		}

		if (st > fst) fst = st;
		if (et < fet) fet = et;
		if (fet < fst)
			return false;
	}

	return true;
}

bool cfc::math::intersection::aabb::Intersect(ray& line, float& depth)
{
	return line.Intersect(*this, depth);
}
