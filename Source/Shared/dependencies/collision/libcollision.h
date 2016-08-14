#pragma once

// Simplified BSD License
//LICENSE:
//Copyright (c) 2013, Leroy Sikkes
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met: 
//
//1. Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer. 
//2. Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution. 
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
//ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef __LIB_COLLISION_H
#define __LIB_COLLISION_H
#ifdef SN_TARGET_PS3
#ifndef LIBCOLLISION_DISABLE_SSE2
#define LIBCOLLISION_DISABLE_SSE2
#endif
#endif
#ifndef LIBCOLLISION_DISABLE_SSE2
#include <xmmintrin.h>
#endif
#include <math.h>
#include <limits>

#ifndef LIBCOLLISIONAPI
#define LIBCOLLISIONAPI
#endif

// mem alloc
#define c_new(x) new x

// Math dependency
#include <cfc/math/math.h>


namespace collision
{
	typedef cfc::math::vector3f vec3f;
	typedef cfc::math::matrix4f mat44f;

#ifdef _MSC_VER
	#undef INFINITY
	extern const float INFINITY;
#endif

	enum CollisionType
	{
		T_NULL,
		T_PLANE,
		T_SPHERE,
		T_TRIANGLE,
		T_RAY,
		T_AABB,
		T_OBB,
		T_FRUSTUM
	};

	enum CollisionFrustumPlane
	{
		FRP_TOP,
		FRP_BOTTOM,
		FRP_LEFT,
		FRP_RIGHT,
		FRP_NEAR,
		FRP_FAR
	};

	enum CollisionInOutResult
	{
		CIO_OUTSIDE=0,
		CIO_INSIDE=1,
		CIO_INTERSECTING=2
	};

	// forward declarations
	class PrimSphere;
	class PrimTri;
	class PrimRay;
	class PrimAABB;
	class PrimOBB;
	class PrimPlane;
	class PrimFrustum;
	
	class LIBCOLLISIONAPI TestResult
	{
	public:
		TestResult() { }
		TestResult(bool val) { m_type = 1; m_boolean = val; }
		TestResult(float val) { m_type = 2; m_float = val; }

		float GetFloat() { if(m_type == 2) return m_float; if(m_type == 1) return m_boolean?1.0f:INFINITY; return INFINITY; }
		bool GetBoolean() { if(m_type == 1) return m_boolean; if(m_type == 2) return m_float > 0.0f && m_float != INFINITY; return false; }

		operator float() { return GetFloat(); }
		operator bool() { return GetBoolean(); }
	protected:
		union
		{
			float m_float;
			bool m_boolean;
		};
		int m_type;
	};

	class PrimAABB;

	class Primitive
	{
	};

	class LIBCOLLISIONAPI PrimPlane: public Primitive
	{
	public:
		PrimPlane(): Primitive() { D = 0.0f; }

		int Type() const {
			return collision::T_PLANE;
		}

		void Set(const vec3f& v0, const vec3f& v1, const vec3f& v2);
		void Set(const vec3f& PlaneNormal, float d) { m_Normal = PlaneNormal; D = d; }
		void Set(const vec3f& PlaneNormal, const vec3f& PointOnPlane)	{ Set(PlaneNormal, -PointOnPlane.Dot(PlaneNormal)); }

		const float* GetPlaneValues() const { return (float*)&m_Normal; }
		float* GetPlaneValues() { return (float*)&m_Normal; }

		const vec3f& GetNormal() const { return m_Normal; }
		float GetD() const { return D; }

		int ClipPolygon(int vertCount, int vertStride, float* vertValues, float* vertOutValues, float distBias = 0.00001f);		

		// Classify (> 0: inside) (< 0: outside) (== 0: intersecting)
		float ClassifyPoint(const vec3f& pt) const { return pt.Dot(m_Normal) - D; }
		vec3f FindClosestPoint(const vec3f& pt) const { return pt - m_Normal * ((-D) + pt.Dot(m_Normal)); }
		void Normalize() { float rcpl=1.0f/m_Normal.Length(); m_Normal *= rcpl; D *= rcpl; }
	protected:
		friend class Primitive;
		friend class PrimSphere;
		friend class PrimTri;
		friend class PrimRay;
		friend class PrimAABB;
		friend class PrimOBB;
		friend class PrimFrustum;

		vec3f m_Normal;
		float D;
	};

	class LIBCOLLISIONAPI PrimSphere: public Primitive
	{
	public:
		PrimSphere(): Primitive() {  }

		int Type() const {
			return collision::T_SPHERE;
		}

		bool Test(const PrimSphere& collider) const;
		bool Test(const vec3f& collider) const; 
		bool Test(const PrimPlane& collider) const;

		CollisionInOutResult TestInOut(const PrimPlane& collider) const;

		void Set(vec3f position, float radius);

		inline float Radius() const { return m_Radius; }
		inline float Radius2() const { return m_Radius2; }
		inline const vec3f& Position() const { return m_Position; }

		bool ConvertToAABB(PrimAABB* var) const;
	protected:
		friend class Primitive;
		friend class PrimTri;
		friend class PrimRay;
		friend class PrimPlane;
		friend class PrimAABB;
		friend class PrimOBB;
		friend class PrimFrustum;

		float m_Radius;
		float m_Radius2;
		vec3f m_Position;
	};

	class LIBCOLLISIONAPI PrimTri : public Primitive
	{
	public:
		PrimTri(): Primitive() { }
		int Type() const {
			return collision::T_TRIANGLE;
		}
		//PrimTri(const PrimTri& cp): Primitive(), m_Vertex0(m_Vertices[0]), m_Vertex1(m_Vertices[1]), m_Vertex2(m_Vertices[2])  { Set(cp.m_Vertices[0], cp.m_Vertices[1], cp.m_Vertices[2]); m_Type = collision::T_TRIANGLE;}
		
		bool operator < (const PrimTri& p) const { if(m_Vertices[0] != p.m_Vertices[0]) return m_Vertices[0] < p.m_Vertices[0]; if(m_Vertices[1] != p.m_Vertices[1]) return m_Vertices[1] < p.m_Vertices[1]; return m_Vertices[2] < p.m_Vertices[2]; }
		PrimTri& operator = (const PrimTri& p) { Set(p.m_Vertices[0], p.m_Vertices[1], p.m_Vertices[2]); return (*this); }
		void Set( const vec3f& a_Vertex0, const vec3f& a_Vertex1, const vec3f& a_Vertex2 );

		bool Test(const PrimAABB& collider) const;
		bool Test(const PrimOBB& collider ) const;
		bool Test(const PrimTri& collider) const;
		bool Test(const PrimSphere& collider) const;
		bool Test(const PrimPlane& collider ) const;

		CollisionInOutResult TestInOut(const PrimPlane& collider) const;
		void Transform(PrimTri& ret, const mat44f& m) const;

		vec3f UVToPoint(float u, float v);
		vec3f PointToUV(vec3f point);

		const vec3f* Vertices() const { return m_Vertices; }
		const vec3f& Normal() { if(m_Dirty) Precompute(); return m_Normal; }
		const vec3f& Edge0() { if(m_Dirty) Precompute(); return m_Edge0; }
		const vec3f& Edge1() { if(m_Dirty) Precompute(); return m_Edge1; }

		inline void SetVertex(int i, const vec3f& x) { m_Vertices[i] = x; m_Dirty = true; }
		inline const vec3f& Vertex(int i) const {return m_Vertices[i];}
		inline const vec3f& Vertex0() const {return m_Vertices[0];}
		inline const vec3f& Vertex1() const {return m_Vertices[1];}
		inline const vec3f& Vertex2() const {return m_Vertices[2];}

		inline void FlagDirty() { m_Dirty = true; }

		bool ConvertToAABB(PrimAABB* var) const;
	protected:
		friend class Primitive;
		friend class PrimSphere;
		friend class PrimRay;
		friend class PrimPlane;
		friend class PrimAABB;
		friend class PrimOBB;
		friend class PrimFrustum;

		void Precompute();
		inline void PrecomputeConst() const { ((PrimTri*)(this))->Precompute(); }

		vec3f m_Vertices[3];
		bool m_Dirty;
		vec3f m_Normal, m_Edge0, m_Edge1;
	};

	class LIBCOLLISIONAPI PrimAABB : public Primitive
	{
	public:
		
		int Type() const {
			return collision::T_AABB;
		}

		PrimAABB():  m_Min(true), m_Max(true) {  }

		PrimAABB(const vec3f& a_min, const vec3f& a_max):m_Min(a_min), m_Max(a_max) { }

		PrimAABB& operator = (const PrimTri& t);

		void Set(vec3f center, vec3f halfExtent);
		void SetMinMax( const vec3f& min, const vec3f& max );

		bool Test (const PrimAABB& collider) const;
		bool Test (const PrimSphere& collider) const;
		bool Test (const PrimPlane& collider) const;
		bool Test (const PrimFrustum& collider) const;
		bool Test (const vec3f& collider) const;

		CollisionInOutResult TestInOut(const PrimPlane& pln) const;

		void GetVertices(vec3f* outVectors);
		void Merge(PrimAABB& other);

		void TransformOBB(collision::PrimOBB& ret, const mat44f& worldMatrix ) const ;
		void Transform(collision::PrimAABB& ret, const mat44f& worldMatrix ) const ;

		inline const vec3f& Min() const { return m_Min; }
		inline const vec3f& Max() const { return m_Max; }
		inline vec3f Center() const { return (m_Min + m_Max) * 0.5f; }
		inline vec3f Extent() const { return (m_Max - m_Min) * 0.5f; }
		inline vec3f HalfExtent() const { return (m_Max - m_Min) * 0.5f; }
		inline vec3f FullExtent() const { return (m_Max - m_Min); }
		inline float Area() { vec3f s = m_Max - m_Min; return (s.x * s.y + s.y * s.z + s.z * s.x) * 2.0f; }
		
		bool ConvertToAABB(PrimAABB* var) const;
		
		vec3f m_Min, m_Max;

	};

	class LIBCOLLISIONAPI PrimOBB: public Primitive
	{
	public:
		PrimOBB() : Primitive(), m_Trans(false), m_InvTrans(false) {}

		int Type() const
		{
			return T_OBB;
		}

		bool Test( const PrimSphere& collider ) const;
		bool Test( const vec3f& InP ) const;
		bool Test( const PrimPlane& InP ) const;
		bool Test( const PrimAABB& InP ) const;
		bool Test( const PrimOBB& InP ) const;

		void Transform( collision::PrimOBB& ret, const mat44f& worldMatrix ) const;
		// Check if the bounding gpu_box is completely behind a plane( defined by a normal and a point )
		bool BoxOutsidePlane( const PrimPlane& p ) const;
		bool IsLineInBox( const vec3f& L1, const vec3f& L2 );
		
		void Set(vec3f halfExtent) { m_HalfExtent = halfExtent; }
		void Set(vec3f halfExtent, const mat44f& m) { m_HalfExtent = halfExtent; SetMatrix(m); }

		void SetMatrix(const mat44f& m);
		inline const mat44f& GetMatrix() const { return m_Trans; }

		vec3f m_HalfExtent;

		bool ConvertToAABB(PrimAABB* var) const;
	protected:
		friend class Primitive;
		friend class PrimSphere;
		friend class PrimTri;
		friend class PrimRay;
		friend class PrimPlane;
		friend class PrimAABB;
		friend class PrimFrustum;
		mat44f m_Trans, m_InvTrans;
	};

	class LIBCOLLISIONAPI PrimRay: public Primitive
	{
	public:
		int Type() const
		{
			return T_RAY;
		}

		void Set(const vec3f& origin, const vec3f& direction) { m_Origin = origin; m_Direction = direction; }
		bool Test(const PrimTri& collider, float& dist) const;
		bool Test(const PrimSphere& collider, float& dist) const;
		bool Test(const PrimAABB& collider, float& dist) const;
		bool Test(const PrimOBB& collider, float& dist) const;
		bool Test(const PrimPlane& collider, float& dist) const;

		inline bool Test(const PrimTri& collider) const { float Distance=-1.0f; return Test(collider, Distance) && Distance > 0.0f; }
		inline bool Test(const PrimSphere& collider) const { float Distance=-1.0f; return Test(collider, Distance) && Distance > 0.0f; }
		inline bool Test(const PrimAABB& collider) const { float Distance=-1.0f; return Test(collider, Distance) && Distance > 0.0f; }
		inline bool Test(const PrimOBB& collider) const { float Distance=-1.0f; return Test(collider, Distance) && Distance > 0.0f; }
		inline bool Test(const PrimPlane& collider) const { float Distance=-1.0f; return Test(collider, Distance) && Distance > 0.0f; }

		bool Test( const PrimAABB& collider, float& dist , float& distMax ) const;
		const vec3f& Origin() const { return m_Origin; }
		const vec3f& Direction() const { return m_Direction; }

		void Transform(collision::PrimRay& ret, const mat44f& m) const;

		vec3f m_Origin;
		vec3f m_Direction;
	protected:
		friend class Primitive;
		friend class PrimSphere;
		friend class PrimTri;
		friend class PrimPlane;
		friend class PrimAABB;
		friend class PrimOBB;
		friend class PrimFrustum;

	};

	class LIBCOLLISIONAPI PrimFrustum: public Primitive
	{
	public:
		PrimFrustum() : Primitive() {}

		int Type() const
		{
			return T_FRUSTUM;
		}

		bool Test(const PrimTri& collider) const;
		bool Test(const PrimSphere& collider) const;
		bool Test(const PrimAABB& collider) const;
		bool Test(const PrimOBB& collider) const;
		
		CollisionInOutResult TestInOut(const PrimTri& collider) const;
		CollisionInOutResult TestInOut(const PrimSphere& collider) const;
		CollisionInOutResult TestInOut(const PrimAABB& collider) const;

		void SetMatrix(const mat44f& ProjectionView);
		void SetView( const mat44f& transMat, float fov, float aspect, float vnear, float vfar );
		void SetView( const mat44f& transMat, float left, float right, float bottom, float top, float vnear, float vfar );
		void SetBox( const mat44f& transMat, float left, float right, float bottom, float top, float front, float back );
		void SetCorners( const vec3f& front_leftBottom, const vec3f& front_rightBottom, const vec3f& front_rightTop, const vec3f& front_leftTop, const vec3f& back_leftBottom, const vec3f& back_rightBottom, const vec3f& back_rightTop, const vec3f& back_leftTop );

		int ClipPolygon( int vertCount, int vertStride, float* vertValues, float* vertOutValues, float distBias=0.00001f);

		mat44f m_SourceMatrix;
		PrimPlane m_Planes[6]; // top bottom left right near far
		vec3f m_Corners[8]; // front: leftBottom rightBottom rightTop leftTop, back: leftBottom rightBottom rightTop leftTop 

		bool ConvertToAABB(PrimAABB* var) const;
	protected:
		friend class Primitive;
		friend class PrimSphere;
		friend class PrimTri;
		friend class PrimRay;
		friend class PrimPlane;
		friend class PrimAABB;
		friend class PrimOBB;
	};

	LIBCOLLISIONAPI int intersect_tri_cube(double boxcenter[3], double boxhalfsize[3], const PrimTri &t);
	LIBCOLLISIONAPI int intersect_ray_tri(const float* orig, const float* dir,
		const float* vert0, const float* vert1, const float* vert2,
		double *t, double *u, double *v);
	LIBCOLLISIONAPI int intersect_ray_tri_nonegativet(const float* orig, const float* dir,
		const float* vert0, const float* vert1, const float* vert2,
		double *t, double *u, double *v);


};

#endif

#ifdef LIBCOLLISION_IMPLEMENTATION
// Simplified BSD License
//LICENSE:
//Copyright (c) 2013, Leroy Sikkes
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met: 
//
//1. Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer. 
//2. Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution. 
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
//ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Simplified BSD License
//LICENSE:
//Copyright (c) 2013, Leroy Sikkes
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met: 
//
//1. Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer. 
//2. Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution. 
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
//ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef LIBCOLLISION_DISABLE_SSE2
static __forceinline __m128 _mm_hadd4_ps(__m128& i)
{
	__m128 t;
	t = _mm_movehl_ps(t, i);
	i = _mm_add_ps(i, t);
	t = _mm_shuffle_ps(i, i, 0x55);
	i = _mm_add_ps(i, t);
	return i;
}

static __forceinline __m128 _mm_sum4_ps(const __m128& a, const __m128& b, const __m128& c, const __m128& d)
{
	/* [a0+a2 c0+c2 a1+a3 c1+c3 */
	__m128 s1 = _mm_add_ps(_mm_unpacklo_ps(a, c), _mm_unpackhi_ps(a, c));
	/* [b0+b2 d0+d2 b1+b3 d1+d3 */
	__m128 s2 = _mm_add_ps(_mm_unpacklo_ps(b, d), _mm_unpackhi_ps(b, d));
	/* [a0+a2 b0+b2 c0+c2 d0+d2]+
	[a1+a3 b1+b3 c1+c3 d1+d3] */
	return _mm_add_ps(_mm_unpacklo_ps(s1, s2), _mm_unpackhi_ps(s1, s2));
}

static __forceinline __m128 _mm_hmin4_ps(const __m128 &a)
{
	const __m128 ftemp = _mm_min_ps(a, _mm_movehl_ps(a, a));
	return _mm_min_ss(ftemp, _mm_shuffle_ps(ftemp, ftemp, 1));
}
static __forceinline __m128 _mm_hmax4_ps(const __m128 &a)
{
	const __m128 ftemp = _mm_max_ps(a, _mm_movehl_ps(a, a));
	return _mm_max_ss(ftemp, _mm_shuffle_ps(ftemp, ftemp, 1));
}

static __forceinline bool testRayBoxSSE(const __m128& bmin, const __m128& bmax, const __m128& origin, const __m128& rcpdir, __m128& nr, __m128& fr )
{
	__m128 t0, t1;
	t0 = _mm_mul_ps(_mm_sub_ps(bmin, origin), rcpdir);
	t1 = _mm_mul_ps(_mm_sub_ps(bmax, origin), rcpdir);
	nr = _mm_move_ss(_mm_min_ps(t0,t1), nr);
	fr = _mm_move_ss(_mm_max_ps(t0,t1), fr);
	nr = _mm_hmax4_ps(nr);
	fr = _mm_hmin4_ps(fr);

	return _mm_comile_ss(nr, fr) ? (_mm_comige_ss(fr, _mm_set_ss(0.0f)) != 0) : false;
}

static __forceinline void testRayBox4SSE(__m128* min4, __m128* max4, __m128* rayori, __m128* rcpraydir, __m128& dist)
{
	__m128 zero = _mm_set_ps1(0.0f);
	__m128 tmin=_mm_set_ps1(0.00001f), tmax = _mm_set_ps1(10000000.0f);
	__m128 t0, t1, nr, fr;
	for(int axis=0; axis<3; ++axis)
	{
		t0 = _mm_mul_ps(_mm_sub_ps(min4[axis], rayori[axis]), rcpraydir[axis]);
		t1 = _mm_mul_ps(_mm_sub_ps(max4[axis], rayori[axis]), rcpraydir[axis]);

		nr = _mm_min_ps(t0, t1);
		fr = _mm_max_ps(t1, t0);
		tmin = _mm_max_ps(tmin, nr);
		tmax = _mm_min_ps(fr, tmax);
	}

	dist = _mm_cmple_ps(tmin, tmax);
	//tmin = _mm_and_ps(tmin, _mm_cmpgt_ps(tmin, zero));   // remove all values < 0 in tmin
	dist = _mm_and_ps(dist, tmin);						 // everywhere tmin <= tmax: set tmin
}


static __forceinline void createRayBox4(collision::PrimAABB& box0, collision::PrimAABB& box1, collision::PrimAABB& box2, collision::PrimAABB& box3, __m128* retmin, __m128* retmax )
{
	for(int i=0; i<3; i++)
	{
		retmin[i] = _mm_set_ps(box3.Min().V[i], box2.Min().V[i], box1.Min().V[i], box0.Min().V[i]);
		retmax[i] = _mm_set_ps(box3.Max().V[i], box2.Max().V[i], box1.Max().V[i], box0.Max().V[i]);
	}
}

#endif

#include <math.h>
#include <limits.h>

#ifndef threadlocal
#ifdef _MSC_VER
#define threadlocal __declspec( thread )
#else
#define threadlocal __thread
#endif
#endif

namespace collision
{
	const float INFtemp = 1.0f;
#ifdef _MSC_VER
	const float INFINITY = std::numeric_limits<float>::infinity();
#endif

	inline float FastFabsf(float myFloat)
	{
#if _MSC_VER && (!_WIN64)
		__asm
		{
			and myFloat, 0x7FFFFFFF;
		};
		return myFloat;
#else
		return fabsf(myFloat);
#endif


		//int f = (*(int *)(&myFloat)) & 0x7FFFFFFF;
		//return *(float *)(&f);
	}

	void GetAxisProjection (const vec3f& axis, const vec3f* Triangle, float& imin, float& imax)
	{
		float dot[3] =
		{
			axis.Dot(Triangle[0]),
			axis.Dot(Triangle[1]),
			axis.Dot(Triangle[2])
		};

		imin = dot[0];
		imax = imin;

		if (dot[1] < imin)
		{
			imin = dot[1];
		}
		else if (dot[1] > imax)
		{
			imax = dot[1];
		}

		if (dot[2] < imin)
		{
			imin = dot[2];
		}
		else if (dot[2] > imax)
		{
			imax = dot[2];
		}
	}
	void GetAxisProjection (const vec3f& axis, const collision::PrimOBB& gpu_box, float& imin, float& imax)
	{
		float origin = axis.Dot(gpu_box.GetMatrix().Translation());
		float maximumExtent =
			FastFabsf(gpu_box.m_HalfExtent.V[0]*axis.Dot(gpu_box.GetMatrix().ViewLocalX())) +
			FastFabsf(gpu_box.m_HalfExtent.V[1]*axis.Dot(gpu_box.GetMatrix().ViewLocalY())) +
			FastFabsf(gpu_box.m_HalfExtent.V[2]*axis.Dot(gpu_box.GetMatrix().ViewLocalZ()));

		imin = origin - maximumExtent;
		imax = origin + maximumExtent;
	}


	int modulo3lookup[] = {0, 1, 2, 0, 1};
	int modulo3p1lookup[] = {1, 2, 0, 1, 2};
	int modulo3p2lookup[] = {2, 0, 1, 2, 0};

	// ------------------------------
	// TRANSFORM FUNCTIONS
	// ------------------------------

	void PrimAABB::Transform(collision::PrimAABB& ret, const mat44f& worldMatrix ) const
	{
		float av, bv;
		int   i, j;
		vec3f new_min, new_max;
		new_min = new_max = worldMatrix.Translation();

		const vec3f& min=Min();
		const vec3f& max=Max();

		for (i = 0; i < 3; i++)
			for (j = 0; j < 3; j++)
			{
				av = worldMatrix.MM[j][i] * min.V[j];
				bv = worldMatrix.MM[j][i] * max.V[j];
				if (av < bv)
				{
					new_min.V[i] += av;
					new_max.V[i] += bv;
				} else {
					new_min.V[i] += bv;
					new_max.V[i] += av;
				}
			}

		ret.SetMinMax(new_min, new_max);
		return;
	}

	void PrimAABB::TransformOBB(collision::PrimOBB& ret, const mat44f& worldMatrix ) const
	{
		mat44f m;
		m.Translation(Center());

		ret.Set(Extent(), worldMatrix * m );
		return;
	}

	void PrimTri::Transform( collision::PrimTri& ret, const mat44f& m ) const
	{
		ret.Set(m * m_Vertices[0] , m * m_Vertices[1], m * m_Vertices[2]);
	}

	void PrimRay::Transform( collision::PrimRay& ret, const mat44f& m ) const
	{
		float w = 0.0f;
		vec3f newRotation = m.TransformVector4D(Direction(), w);
		ret.Set(m * Origin()  , newRotation);
	}

	void PrimOBB::Transform( collision::PrimOBB& ret, const mat44f& worldMatrix ) const
	{
		ret.SetMatrix(worldMatrix * GetMatrix());
		ret.Set(m_HalfExtent);
		return;
	}

	// ------------------------------
	// COLLISION TESTS
	// ------------------------------
	bool PrimSphere::Test( const PrimSphere& collider ) const
	{
		vec3f dist=collider.m_Position - m_Position;
		float radiusSquared = m_Radius + collider.m_Radius;
		radiusSquared *= radiusSquared;
		if (dist.Dot(dist) > radiusSquared)
			return false;
		else
			return true;
	}
	bool PrimSphere::Test( const vec3f& collider ) const
	{
		vec3f dist=collider - m_Position;
		if(dist.Dot(dist) > m_Radius2)
			return false;
		else
			return true;
	}

	bool PrimSphere::Test( const PrimPlane& collider ) const
	{
		if(collider.m_Normal.x * m_Position.x + collider.m_Normal.y * m_Position.y + collider.m_Normal.z * m_Position.z - collider.D >= -m_Radius)
			return true;
		return false;
	}


	collision::CollisionInOutResult PrimSphere::TestInOut( const PrimPlane& collider ) const
	{	
		float fDistance = collider.m_Normal.x * m_Position.x + collider.m_Normal.y * m_Position.y + collider.m_Normal.z * m_Position.z - collider.D;
		if(fDistance > -m_Radius)
			return CIO_INSIDE;
		else if(fabsf(fDistance) < m_Radius )
			return CIO_INTERSECTING;
		else
			return CIO_OUTSIDE;
	}


	bool PrimAABB::Test( const PrimAABB& collider ) const
	{
		if(m_Min.x > collider.m_Max.x) return false;
		if(m_Min.y > collider.m_Max.y) return false;
		if(m_Min.z > collider.m_Max.z) return false;
		if(m_Max.x < collider.m_Min.x) return false;
		if(m_Max.y < collider.m_Min.y) return false;
		if(m_Max.z < collider.m_Min.z) return false;
		return true;
	}

	bool PrimAABB::Test( const PrimSphere& collider ) const
	{
		float totalDistance = 0;

		// Accumulate the distance of the sphere's center on each axis
		for(int i = 0; i < 3; ++i) {

			// If the sphere's center is outside the aabb, we've got distance on this axis
			if(collider.m_Position.V[i] < m_Min.V[i]) {
				float borderDistance = m_Min.V[i] - collider.m_Position.V[i];
				totalDistance += borderDistance * borderDistance;

			} else if(collider.m_Position.V[i] > m_Max.V[i]) {
				float borderDistance = collider.m_Position.V[i] - m_Max.V[i];
				totalDistance += borderDistance * borderDistance;

			}
			// Otherwise the sphere's center is within the gpu_box on this axis, so the
			// distance will be 0 and we do not need to accumulate anything at all

		}

		// If the distance to the gpu_box is lower than the sphere's radius, both are overlapping
		return (totalDistance <= collider.Radius2());
	}

	bool PrimAABB::Test( const PrimPlane& collider ) const
	{
		// fixes gaps popping up but is not gpu_box, theoretically..
		vec3f pvertex = m_Min;
		if(collider.m_Normal.x >= 0.0f)
			pvertex.x = m_Max.x;
		if(collider.m_Normal.y >= 0.0f)
			pvertex.y = m_Max.y;
		if(collider.m_Normal.z >= 0.0f)
			pvertex.z = m_Max.z;

		// if pvertex is outside, return false
		if (collider.ClassifyPoint(pvertex) < 0)
			return false;

		return true;
	}

	bool PrimAABB::Test( const vec3f& collider ) const
	{
		if(m_Min.x > collider.x) return false;
		if(m_Max.x < collider.x) return false;
		if(m_Min.y > collider.y) return false;
		if(m_Max.y < collider.y) return false;
		if(m_Min.z > collider.z) return false;
		if(m_Max.z < collider.z) return false;
		return true;
	}

	bool PrimAABB::Test( const PrimFrustum& collider ) const
	{
		return collider.Test(*this);
	}

	CollisionInOutResult PrimAABB::TestInOut(const PrimPlane& pln) const
	{
		int iInCount = 0;
		if(pln.ClassifyPoint(m_Min) >= 0)									++iInCount;
		if(pln.ClassifyPoint(vec3f(m_Max.x, m_Min.y, m_Min.z)) >= 0)	++iInCount;
		if(pln.ClassifyPoint(vec3f(m_Min.x, m_Max.y, m_Min.z)) >= 0)	++iInCount;
		if(pln.ClassifyPoint(vec3f(m_Min.x, m_Min.y, m_Max.z)) >= 0)	++iInCount;
		if(pln.ClassifyPoint(vec3f(m_Max.x, m_Max.y, m_Min.z)) >= 0)	++iInCount;
		if(pln.ClassifyPoint(vec3f(m_Min.x, m_Max.y, m_Max.z)) >= 0)	++iInCount;
		if(pln.ClassifyPoint(vec3f(m_Max.x, m_Min.y, m_Max.z)) >= 0)	++iInCount;
		if(pln.ClassifyPoint(m_Max) >= 0)									++iInCount;

		// were all the points outside of plane p?
		if(iInCount == 0)
			return CIO_OUTSIDE;
		else if(iInCount == 8) // all points inside?
			return CIO_INSIDE;
		else // we must be partly in then otherwise
			return CIO_INTERSECTING;	
	}

	bool PrimRay::Test( const PrimTri& collider, float& dist ) const
	{
#define SUB(dest,v1,v2) \
	dest[0]=v1[0]-v2[0]; \
	dest[1]=v1[1]-v2[1]; \
	dest[2]=v1[2]-v2[2]; 

#define CROSS(dest,v1,v2) \
	dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0]; 

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

		if(collider.m_Dirty)
			collider.PrecomputeConst();

		float u,v;
		const float EPSILON = 0.00001f;

		float pvec[3],tvec[3],qvec[3],*edge0,*edge1,*vec0, *ori, *dir;
		edge0 = (float*)collider.m_Edge0.V;
		edge1 = (float*)collider.m_Edge1.V;
		vec0 = (float*)collider.m_Vertices[0].V;
		ori = (float*)m_Origin.V;
		dir = (float*)m_Direction.V;

		CROSS(pvec, dir, edge1);
		float det = DOT(edge0, pvec);

		if(det > EPSILON)
		{
			SUB(tvec, ori, vec0);
			u = DOT(tvec, pvec);
			if (u < 0.0 || u > det)
				return false;

			CROSS(qvec, tvec, edge0);

			// calculate V parameter and test bounds
			v = DOT(dir, qvec);
			if (v < 0.0 || u + v > det)
				return false;
		}
		else if(det < -EPSILON)
		{
			SUB(tvec, ori, vec0);
			u = DOT(tvec, pvec);
			if (u > 0.0 || u < det)
				return false;

			CROSS(qvec, tvec, edge0);

			// calculate V parameter and test bounds
			v = DOT(dir, qvec);
			if (v > 0.0 || u + v < det)
				return false;
		}
		else
			return false;

		dist = DOT(edge1, qvec) / det;
		return true;

#undef SUB
#undef CROSS
#undef DOT
	}

	bool PrimRay::Test( const PrimSphere& collider, float& dist ) const
	{
		// fast
		vec3f A = m_Origin - collider.m_Position;
		float B = A.Dot(m_Direction);
		float C = A.Dot(A) - collider.Radius2();
		float D = B*B - C;

		if(D > 0)
		{
			dist = (-B - sqrtf(D) );
			if(dist < 0.0f)
				return false;
			else
				return true;
		}
		return false;

		// high precision
		/*double x0=m_Origin.x, y0=m_Origin.y, z0=m_Origin.z;
		double cx = collider.m_Position.x, cy=collider.m_Position.y, cz=collider.m_Position.z;
		double dx = m_Direction.x, dy = m_Direction.y, dz = m_Direction.z;
		double a = dx*dx+dy*dy+dz*dz;
		double b = 2.0*dx*(x0-cx) +  2.0*dy*(y0-cy) +  2.0*dz*(z0-cz);
		double c  =cx*cx + cy*cy + cz*cz + x0*x0 + y0*y0 + z0*z0 + -2.0 *(cx*x0 + cy*y0 + cz*z0) - static_cast<double>(collider.Radius2());
		double D = b*b-4.0*a*c;

		if(D > 0)
		{
			dist = (-b - sqrtf(D) ) / (2.0 * a);
			if(dist < 0.0f)
				return false;
			else
				return true;
		}
		return false;
		*/
	}

	bool PrimRay::Test( const PrimAABB& collider, float& dist ) const
	{
		// based on Jacco Bikker's BVH tree gpu_box intersection code (Arauna Raytracer)
		float tmin = 0.00001f, tmax = 10000000.0f;
		for ( int axis = 0; axis < 3; ++axis ) 
		{
			const float rcprdt = 1.0f / m_Direction.V[axis];
			const float t0 = (collider.m_Min.V[axis] - m_Origin.V[axis]) * rcprdt;
			const float t1 = (collider.m_Max.V[axis] - m_Origin.V[axis]) * rcprdt;

			const float nr = MIN(t0, t1);
			const float fr = MAX(t0, t1);
			tmin = MAX(tmin, nr);
			tmax = MIN(fr, tmax);
			/*const float nr = (t0 < t1) ? t0 : t1, fr = (t0 < t1) ? t1 : t0;
			tmin = (tmin < nr) ? nr : tmin, tmax = (fr <  tmax) ? fr : tmax;*/
		}
		if (tmin <= tmax)
		{
			dist = tmin;
			return tmin >= 0.0f;
		}
		else
		{
			return false;
		}
	}


	bool PrimRay::Test( const PrimAABB& collider, float& dist , float& distMax ) const
	{
#define TEST_AXIS(axis) \
		{\
		const float rcprdt = 1.0f / m_Direction.V[axis];\
		const float t0 = (collider.m_Min.V[axis] - m_Origin.V[axis]) * rcprdt;\
		const float t1 = (collider.m_Max.V[axis] - m_Origin.V[axis]) * rcprdt;\
		\
		const float nr = (t0 < t1) ? t0 : t1;\
		const float fr = (t0 < t1) ? t1 : t0;\
		tmin = (tmin < nr) ? nr : tmin;\
		tmax = (tmax > fr) ? fr : tmax;\
	}


	//

		// based on Jacco Bikker's BVH tree gpu_box intersection code (Arauna Raytracer)
		float tmin = dist, tmax = distMax;
		TEST_AXIS(0);
		TEST_AXIS(1);
		TEST_AXIS(2);
		#undef TEST_AXIS

		if (tmin <= tmax) 
		{
			dist = tmin;
			distMax = tmax;
			return tmax > 0.0f;
		}
		else
			return false;

	}

	bool PrimRay::Test( const PrimOBB& collider, float& dist ) const
	{
		vec3f RayOriginTrans = collider.m_InvTrans * m_Origin , RayDirTrans = collider.m_InvTrans.Rotation() * m_Direction;

		// Jacco's BVH intersection code, with ray to OBB space code by Seniltai (seen above).
		float tmin = 0.00001f, tmax = INFINITY;
		for ( int axis = 0; axis < 3; ++axis ) 
		{
			const float rcprdt = 1.0f / RayDirTrans.V[axis];
			const float t0 = (-collider.m_HalfExtent.V[axis] - RayOriginTrans.V[axis]) * rcprdt;
			const float t1 = (collider.m_HalfExtent.V[axis] - RayOriginTrans.V[axis]) * rcprdt;
			const float nr = (t0 < t1) ? t0 : t1, fr = (t0 < t1) ? t1 : t0;
			tmin = (tmin < nr) ? nr : tmin, tmax = (fr <  tmax) ? fr : tmax;
		}
		if (tmin <= tmax) 
		{
			dist = tmin;
			return dist > 0.0f;
		}
		else
			return false;
	}

	bool PrimRay::Test( const PrimPlane& collider, float& dist ) const
	{
		float cosAngle = (m_Direction.Dot(collider.m_Normal));
		dist = -(m_Origin.Dot(collider.m_Normal) + collider.D) / cosAngle ;
		if(dist < 0.0f)
			return false;
		return true;
	}


	void PrimTri::Set( const vec3f& a_Vertex0, const vec3f& a_Vertex1, const vec3f& a_Vertex2 )
	{
		m_Vertices[0] = a_Vertex0;
		m_Vertices[1] = a_Vertex1;
		m_Vertices[2] = a_Vertex2;
		m_Dirty = true;
	}

	bool PrimTri::Test( const PrimSphere& collider ) const
	{
		 // not 100 procent correct (when the points are outside the circle, just pass though for example), but it's a very fast method though..
		for(int i=0; i<3; i++)
		{
			if(collider.Test(m_Vertices[i]) == true)
				return true;
		}
		return false;
	}

	bool PrimTri::Test( const PrimPlane& collider ) const
	{
		for(int i=0; i<3; i++)
		{
			if(m_Vertices[i].Dot(collider.m_Normal) - collider.D >= 0.0f)
				return true; // if one point is on the positive of the plane, it succeeds (it either intersects or is on positive side)
		}

		return false; // no point made it through the plane test.
	}


	collision::CollisionInOutResult PrimTri::TestInOut( const PrimPlane& collider ) const
	{
		int ptCount = 0;
		for(int i=0; i<3; i++)
		{
			if(m_Vertices[i].Dot(collider.m_Normal) - collider.D >= 0.0f)
				ptCount++;
		}

		if(ptCount == 0)
			return CIO_OUTSIDE;
		else if(ptCount == 3)
			return CIO_INSIDE;
		else 
			return CIO_INTERSECTING;
	}


#define EDGE_EDGE_TEST(V0,U0,U1)                      \
	Bx=U0[i0]-U1[i0];                                   \
	By=U0[i1]-U1[i1];                                   \
	Cx=V0[i0]-U0[i0];                                   \
	Cy=V0[i1]-U0[i1];                                   \
	f=Ay*Bx-Ax*By;                                      \
	d=By*Cx-Bx*Cy;                                      \
	if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
	{                                                   \
	e=Ax*Cy-Ay*Cx;                                    \
	if(f>0)                                           \
	{                                                 \
	if(e>=0 && e<=f) return 1;                      \
	}                                                 \
	else                                              \
	{                                                 \
	if(e<=0 && e>=f) return 1;                      \
	}                                                 \
	}

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
	{                                                \
		float Ax,Ay,Bx,By,Cx,Cy,e,d,f;               \
		Ax = V1[i0] - V0[i0];                        \
		Ay = V1[i1] - V0[i1];                        \
		/* test edge U0,U1 against V0,V1 */          \
		EDGE_EDGE_TEST(V0,U0,U1);                    \
		/* test edge U1,U2 against V0,V1 */          \
		EDGE_EDGE_TEST(V0,U1,U2);                    \
		/* test edge U2,U1 against V0,V1 */          \
		EDGE_EDGE_TEST(V0,U2,U0);                    \
	}

#define POINT_IN_TRI(V0,U0,U1,U2)           \
	{                                           \
	float a,b,c,d0,d1,d2;                     \
	/* is T1 completly inside T2? */          \
	/* check if V0 is inside tri(U0,U1,U2) */ \
	a = U1[i1] - U0[i1];                          \
	b=-(U1[i0]-U0[i0]);                       \
	c=-a*U0[i0]-b*U0[i1];                     \
	d0=a*V0[i0]+b*V0[i1]+c;                   \
	\
	a=U2[i1]-U1[i1];                          \
	b=-(U2[i0]-U1[i0]);                       \
	c=-a*U1[i0]-b*U1[i1];                     \
	d1=a*V0[i0]+b*V0[i1]+c;                   \
	\
	a=U0[i1]-U2[i1];                          \
	b=-(U0[i0]-U2[i0]);                       \
	c=-a*U2[i0]-b*U2[i1];                     \
	d2=a*V0[i0]+b*V0[i1]+c;                   \
	if(d0*d1>0.0)                             \
	{                                         \
	if(d0*d2>0.0) return 1;                 \
	}                                         \
	}

int coplanar_tri_tri(const vec3f& a_Normal, const vec3f& a_Vertex0, const vec3f& a_Vertex1, const vec3f& a_Vertex2, 
					 const vec3f& a_ColVertex0, const vec3f& a_ColVertex1, const vec3f& a_ColVertex2)
{
	vec3f A;
	short i0,i1;
	/* first project onto an axis-aligned plane, that maximizes the area */
	/* of the triangles, compute indices: i0,i1. */
	A.V[0]=fabsf(a_Normal.V[0]);
	A.V[1]=fabsf(a_Normal.V[1]);
	A.V[2]=fabsf(a_Normal.V[2]);
	if(A.V[0]>A.V[1])
	{
		if(A.V[0]>A.V[2])
		{
			i0=1;      /* A[0] is greatest */
			i1=2;
		}
		else
		{
			i0=0;      /* A[2] is greatest */
			i1=1;
		}
	}
	else   /* A[0]<=A[1] */
	{
		if(A.V[2]>A.V[1])
		{
			i0=0;      /* A[2] is greatest */
			i1=1;
		}
		else
		{
			i0=0;      /* A[1] is greatest */
			i1=2;
		}
	}

	/* test all edges of triangle 1 against the edges of triangle 2 */
	EDGE_AGAINST_TRI_EDGES(a_Vertex0,a_Vertex1,a_ColVertex0,a_ColVertex1,a_ColVertex2);
	EDGE_AGAINST_TRI_EDGES(a_Vertex1,a_Vertex2,a_ColVertex0,a_ColVertex1,a_ColVertex2);
	EDGE_AGAINST_TRI_EDGES(a_Vertex2,a_Vertex0,a_ColVertex0,a_ColVertex1,a_ColVertex2);

	/* finally, test if tri1 is totally contained in tri2 or vice versa */
	POINT_IN_TRI(a_Vertex0,a_ColVertex0,a_ColVertex1,a_ColVertex2);
	POINT_IN_TRI(a_ColVertex0,a_Vertex0,a_Vertex1,a_Vertex2);

	return 0;
}

#define SORT(a,b)       \
	if(a>b)    \
	{          \
		float c; \
		c=a;     \
		a=b;     \
		b=c;     \
	}

	bool PrimTri::Test( const PrimTri& collider ) const
	{
		if(m_Dirty) { PrimTri &pt=const_cast<PrimTri&>(*this); pt.Precompute();	}

		float d1,d2;
		float du0,du1,du2,dv0,dv1,dv2;
		vec3f D;
		float isect1[2], isect2[2];
		float du0du1,du0du2,dv0dv1,dv0dv2;
		short index;
		float vp0,vp1,vp2;
		float up0,up1,up2;
		float bb,cc,max;

		d1 = -m_Normal.Dot(m_Vertices[0]);
		du0 = m_Normal.Dot(collider.m_Vertices[0]) + d1;
		du1 = m_Normal.Dot(collider.m_Vertices[1]) + d1;
		du2 = m_Normal.Dot(collider.m_Vertices[2]) + d1;

		du0du1 = du0 * du1;
		du0du2 = du0 * du2;

		if (du0du1 > 0.0f && du0du2 > 0.0f)
			return false;

		d2 = -collider.m_Normal.Dot(collider.m_Vertices[0]);
		dv0 = collider.m_Normal.Dot(m_Vertices[0]) + d2;
		dv1 = collider.m_Normal.Dot(m_Vertices[1]) + d2;
		dv2 = collider.m_Normal.Dot(m_Vertices[2]) + d2;

		dv0dv1 = dv0 * dv1;
		dv0dv2 = dv0 * dv2;

		if (dv0dv1 > 0.0f && dv0dv2 > 0.0f)
			return false;

		D = m_Normal.Cross(collider.m_Normal);

		max = fabsf(D.V[0]);
		index = 0;
		bb = fabsf(D.V[1]);
		cc = fabsf(D.V[2]);
		if(bb > max) max = bb,index = 1;
		if(cc > max) max = cc,index = 2;

		vp0 = m_Vertices[0].V[index];
		vp1 = m_Vertices[1].V[index];
		vp2 = m_Vertices[2].V[index];

		up0 = collider.m_Vertices[0].V[index];
		up1 = collider.m_Vertices[1].V[index];
		up2 = collider.m_Vertices[2].V[index];

		float a,b,c,x0,x1;
		if (dv0dv1 > 0.0f || dv2 != 0.0f) {
			a = vp2; b = (vp0 - vp2) * dv2; c = (vp1 - vp2) * dv2; x0 = dv1 - dv0; x1 = dv2 - dv1;
		}
		else if (dv0dv2 > 0.0f || dv1 != 0.0f) {
			a = vp1; b = (vp0 - vp1) * dv1; c = (vp2 - vp1) * dv1; x0 = dv1 - dv0; x1 = dv1 - dv2;
		}
		else if ((dv1 * dv2) > 0.0f || dv0 != 0.0f) {
			a = vp0; b = (vp1 - vp0) * dv0; c = (vp2 - vp0) * dv0; x0 = dv0 - dv1; x1 = dv0 - dv2;
		}
		else
		{
			// FIX: This function always returns 0, so always return false
			return (coplanar_tri_tri(m_Normal,m_Vertices[0],m_Vertices[1],m_Vertices[2],collider.m_Vertices[0],collider.m_Vertices[1],collider.m_Vertices[2]) == 0) ? true : false;
		}

		float d,e,f,y0,y1;
		if (du0du1 > 0.0f || du2 != 0.0f) {
			d = up2; e = (up0 - up2) * du2; f = (up1 - up2) * du2; y0 = du1 - du0; y1 = du2 - du1;
		}
		else if (du0du2 > 0.0f || du1 != 0.0f) {
			d = up1; e = (up0 - up1) * du1; f = (up2 - up1) * du1; y0 = du1 - du0; y1 = du1 - du2;
		}
		else if ((du1 * du2) > 0.0f || du0 != 0.0f) {
			d = up0; e = (up1 - up0) * du0; f = (up2 - up0) * du0; y0 = du0 - du1; y1 = du0 - du2;
		}
		else
		{
			return (coplanar_tri_tri(m_Normal,m_Vertices[0],m_Vertices[1],m_Vertices[2],collider.m_Vertices[0],collider.m_Vertices[1],collider.m_Vertices[2]) == 0) ? true : false;
		}

		float xx,yy,xxyy,tmp;
		xx = x0 * x1;
		yy = y0 * y1;
		xxyy =xx * yy;

		tmp = a * xxyy;
		isect1[0] =tmp + b * x1 * yy;
		isect1[1] =tmp + c * x0 * yy;

		tmp = d * xxyy;
		isect2[0] = tmp + e * xx * y1;
		isect2[1] = tmp + f * xx * y0;

		SORT(isect1[0],isect1[1]);
		SORT(isect2[0],isect2[1]);

		if(isect1[1]<isect2[0] || isect2[1]<isect1[0]) return 0;
		return 1;

	}



	bool PrimTri::Test( const PrimAABB& collider ) const
	{
		vec3f c=collider.Center(), ex=collider.Extent();
		double bc[3] = {c.x, c.y, c.z};
		double bhe[3] = {ex.x, ex.y, ex.z};
		return (intersect_tri_cube(bc, bhe, *this) == 1) ? true : false;
	}

	bool PrimTri::Test( const PrimOBB& collider ) const
	{
		// transform tri so it's like an aabb
		PrimTri pttf;
		pttf.Set(collider.m_InvTrans * m_Vertices[0] , collider.m_InvTrans * m_Vertices[1], collider.m_InvTrans * m_Vertices[2]);
		
		double bc[3] = {0.0, 0.0, 0.0};
		double bhe[3] = {collider.m_HalfExtent.x, collider.m_HalfExtent.y, collider.m_HalfExtent.z};
		return (intersect_tri_cube(bc, bhe, pttf) == 1) ? true : false;

		/*if(m_Dirty) { PrimTri &pt=const_cast<PrimTri&>(*this); pt.Precompute();	}

		

		float min0, max0, min1, max1;
		vec3f edge2;

		// Test direction of triangle normal.
		min0 = m_Normal.Dot(m_Vertices[0]);
		max0 = min0;
		GetAxisProjection(D, *mBox, min1, max1);
		if (max1 < min0 || max0 < min1)
		{
			return false;
		}

		// Test direction of gpu_box faces.
		for (int i = 0; i < 3; ++i)
		{
			D = mBox->Axis[i];
			GetAxisProjection(D, *mTriangle, min0, max0);
			float DdC = D.Dot(mBox->Center);
			min1 = DdC - mBox->Extent[i];
			max1 = DdC + mBox->Extent[i];
			if (max1 < min0 || max0 < min1)
			{
				return false;
			}
		}

		// Test direction of triangle-gpu_box edge cross products.
		edge[2] = edge[1] - edge[0];
		for (int i0 = 0; i0 < 3; ++i0)
		{
			for (int i1 = 0; i1 < 3; ++i1)
			{
				D = edge[i0].Cross(mBox->Axis[i1]);
				GetAxisProjection(D, *mTriangle, min0, max0);
				GetAxisProjection(D, *mBox, min1, max1);
				if (max1 < min0 || max0 < min1)
				{
					return false;
				}
			}
		}

		return true;*/
	}


	bool PrimOBB::Test( const vec3f& InP ) const
	{
		// Rotate the point into the gpu_box's coordinates
		vec3f P = InP * m_InvTrans;

		// Now just use an axis-aligned check
		if ( fabs(P.x) < m_HalfExtent.x && fabs(P.y) < m_HalfExtent.y && fabs(P.z) < m_HalfExtent.z ) 
			return true;

		return false;
	}

	bool PrimOBB::Test( const PrimSphere& collider ) const
	{
		float fDist;
		float fDistSq = 0;
		vec3f P = m_InvTrans * collider.m_Position;

		// Add distance squared from sphere centerpoint to gpu_box for each axis
		for ( int i = 0; i < 3; i++ )
		{
			if ( fabs(P[i]) > m_HalfExtent[i] )
			{
				fDist = fabs(P[i]) - m_HalfExtent[i];
				fDistSq += fDist*fDist;
			}
		}
		return ( fDistSq <= collider.m_Radius2 );
	}

	bool PrimOBB::Test( const PrimPlane& InP ) const
	{
		return !BoxOutsidePlane(InP);
	}



	bool PrimOBB::Test( const PrimOBB& InP ) const
	{
		// Original source http://www.geometrictools.com/LibMathematics/Intersection/Intersection.html
		// Modified to work with libcollision..

		// Cutoff for cosine of angles between gpu_box axes.  This is used to catch
		// the cases when at least one pair of axes are parallel.  If this
		// happens, there is no need to test for separation along the
		// Cross(A[i],B[j]) directions.
		const float cutoff = (float)1.0f - 0.0001f;
		bool existsParallelPair = false;
		int i;

		// Convenience variables.
		vec3f A[3]; A[0] = m_Trans.ViewLocalX(); A[1] = m_Trans.ViewLocalY(); A[2] = m_Trans.ViewLocalZ();
		vec3f B[3]; B[0] = InP.m_Trans.ViewLocalX(); B[1] = InP.m_Trans.ViewLocalY(); B[2] = InP.m_Trans.ViewLocalZ();
		const float* EA = m_HalfExtent.V;
		const float* EB = InP.m_HalfExtent.V;

		// Compute difference of gpu_box centers, D = C1-C0.
		vec3f D = InP.m_Trans.Translation() - m_Trans.Translation();

		float C[3][3];     // matrix C = A^T B, c_{ij} = Dot(A_i,B_j)
		float AbsC[3][3];  // |c_{ij}|
		float AD[3];       // Dot(A_i,D)
		float r0, r1, r;   // interval radii and distance between centers
		float r01;         // = R0 + R1

		// axis C0+t*A0
		for (i = 0; i < 3; ++i)
		{
			C[0][i] = A[0].Dot(B[i]);
			AbsC[0][i] = FastFabsf(C[0][i]);
			if (AbsC[0][i] > cutoff)
			{
				existsParallelPair = true;
			}
		}
		AD[0] = A[0].Dot(D);
		r = FastFabsf(AD[0]);
		r1 = EB[0]*AbsC[0][0] + EB[1]*AbsC[0][1] + EB[2]*AbsC[0][2];
		r01 = EA[0] + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*A1
		for (i = 0; i < 3; ++i)
		{
			C[1][i] = A[1].Dot(B[i]);
			AbsC[1][i] = FastFabsf(C[1][i]);
			if (AbsC[1][i] > cutoff)
			{
				existsParallelPair = true;
			}
		}
		AD[1] = A[1].Dot(D);
		r = FastFabsf(AD[1]);
		r1 = EB[0]*AbsC[1][0] + EB[1]*AbsC[1][1] + EB[2]*AbsC[1][2];
		r01 = EA[1] + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*A2
		for (i = 0; i < 3; ++i)
		{
			C[2][i] = A[2].Dot(B[i]);
			AbsC[2][i] = FastFabsf(C[2][i]);
			if (AbsC[2][i] > cutoff)
			{
				existsParallelPair = true;
			}
		}
		AD[2] = A[2].Dot(D);
		r = FastFabsf(AD[2]);
		r1 = EB[0]*AbsC[2][0] + EB[1]*AbsC[2][1] + EB[2]*AbsC[2][2];
		r01 = EA[2] + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*B0
		r = FastFabsf(B[0].Dot(D));
		r0 = EA[0]*AbsC[0][0] + EA[1]*AbsC[1][0] + EA[2]*AbsC[2][0];
		r01 = r0 + EB[0];
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*B1
		r = FastFabsf(B[1].Dot(D));
		r0 = EA[0]*AbsC[0][1] + EA[1]*AbsC[1][1] + EA[2]*AbsC[2][1];
		r01 = r0 + EB[1];
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*B2
		r = FastFabsf(B[2].Dot(D));
		r0 = EA[0]*AbsC[0][2] + EA[1]*AbsC[1][2] + EA[2]*AbsC[2][2];
		r01 = r0 + EB[2];
		if (r > r01)
		{
			return false;
		}

		// At least one pair of gpu_box axes was parallel, so the separation is
		// effectively in 2D where checking the "edge" normals is sufficient for
		// the separation of the boxes.
		if (existsParallelPair)
		{
			return true;
		}

		// axis C0+t*A0xB0
		r = FastFabsf(AD[2]*C[1][0] - AD[1]*C[2][0]);
		r0 = EA[1]*AbsC[2][0] + EA[2]*AbsC[1][0];
		r1 = EB[1]*AbsC[0][2] + EB[2]*AbsC[0][1];
		r01 = r0 + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*A0xB1
		r = FastFabsf(AD[2]*C[1][1] - AD[1]*C[2][1]);
		r0 = EA[1]*AbsC[2][1] + EA[2]*AbsC[1][1];
		r1 = EB[0]*AbsC[0][2] + EB[2]*AbsC[0][0];
		r01 = r0 + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*A0xB2
		r = FastFabsf(AD[2]*C[1][2] - AD[1]*C[2][2]);
		r0 = EA[1]*AbsC[2][2] + EA[2]*AbsC[1][2];
		r1 = EB[0]*AbsC[0][1] + EB[1]*AbsC[0][0];
		r01 = r0 + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*A1xB0
		r = FastFabsf(AD[0]*C[2][0] - AD[2]*C[0][0]);
		r0 = EA[0]*AbsC[2][0] + EA[2]*AbsC[0][0];
		r1 = EB[1]*AbsC[1][2] + EB[2]*AbsC[1][1];
		r01 = r0 + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*A1xB1
		r = FastFabsf(AD[0]*C[2][1] - AD[2]*C[0][1]);
		r0 = EA[0]*AbsC[2][1] + EA[2]*AbsC[0][1];
		r1 = EB[0]*AbsC[1][2] + EB[2]*AbsC[1][0];
		r01 = r0 + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*A1xB2
		r = FastFabsf(AD[0]*C[2][2] - AD[2]*C[0][2]);
		r0 = EA[0]*AbsC[2][2] + EA[2]*AbsC[0][2];
		r1 = EB[0]*AbsC[1][1] + EB[1]*AbsC[1][0];
		r01 = r0 + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*A2xB0
		r = FastFabsf(AD[1]*C[0][0] - AD[0]*C[1][0]);
		r0 = EA[0]*AbsC[1][0] + EA[1]*AbsC[0][0];
		r1 = EB[1]*AbsC[2][2] + EB[2]*AbsC[2][1];
		r01 = r0 + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*A2xB1
		r = FastFabsf(AD[1]*C[0][1] - AD[0]*C[1][1]);
		r0 = EA[0]*AbsC[1][1] + EA[1]*AbsC[0][1];
		r1 = EB[0]*AbsC[2][2] + EB[2]*AbsC[2][0];
		r01 = r0 + r1;
		if (r > r01)
		{
			return false;
		}

		// axis C0+t*A2xB2
		r = FastFabsf(AD[1]*C[0][2] - AD[0]*C[1][2]);
		r0 = EA[0]*AbsC[1][2] + EA[1]*AbsC[0][2];
		r1 = EB[0]*AbsC[2][1] + EB[1]*AbsC[2][0];
		r01 = r0 + r1;
		if (r > r01)
		{
			return false;
		}

		return true;

		// ---------------------------------
		// Old test (did not work properly with scaling > 1)
		// ---------------------------------

		/*vec3f SizeA = m_Extent;
		vec3f SizeB = InP.m_Extent;
		vec3f RotA[3], RotB[3];	

		const mat44f& mA = m_InvTrans;
		const mat44f& mB = InP.m_InvTrans;

		RotA[0] = mA.LocalX();
		RotA[1] = mA.LocalY();
		RotA[2] = mA.LocalZ();

		RotB[0] = mB.LocalX();
		RotB[1] = mB.LocalY();
		RotB[2] = mB.LocalZ();

		float R[3][3];  // Rotation from B to A
		float AR[3][3]; // absolute values of R matrix, to use with gpu_box extents
		float ExtentA, ExtentB, Separation;
		int i, k;

		// Calculate B to A rotation matrix
		for( i = 0; i < 3; i++ )
		{
		for( k = 0; k < 3; k++ )
		{
		R[i][k] = RotA[i].Dot(RotB[k]); 
		AR[i][k] = FastFabsf(R[i][k]);
		}
		}

		// Vector separating the centers of Box B and of Box A	
		vec3f vSepWS = InP.m_Trans.Translation() - m_Trans.Translation();
		// Rotated into Box A's coordinates
		vec3f vSepA( vSepWS.Dot(RotA[0]), vSepWS.Dot(RotA[1]), vSepWS.Dot(RotA[2]) );            

		// Test if any of A's basis vectors separate the gpu_box
		for( i = 0; i < 3; i++ )
		{
		ExtentA = SizeA[i];
		ExtentB = SizeB.Dot( vec3f( AR[i][0], AR[i][1], AR[i][2] ) );
		Separation = FastFabsf( vSepA[i] );

		if( Separation > ExtentA + ExtentB ) return false;
		}

		// Test if any of B's basis vectors separate the gpu_box
		for( k = 0; k < 3; k++ )
		{
		ExtentA = SizeA.Dot( vec3f( AR[0][k], AR[1][k], AR[2][k] ) );
		ExtentB = SizeB[k];
		Separation = FastFabsf( vSepA.Dot( vec3f(R[0][k],R[1][k],R[2][k]) ) );

		if( Separation > ExtentA + ExtentB ) return false;
		}

		// Now test Cross Products of each basis vector combination ( A[i], B[k] )
		for( i=0 ; i<3 ; i++ )
		{
		const int& i1 = modulo3p1lookup[i], &i2 = modulo3p2lookup[i];
		for( k=0 ; k<3 ; k++ )
		{
		const int& k1 = modulo3p1lookup[k], &k2 = modulo3p2lookup[k];
		ExtentA = SizeA[i1] * AR[i2][k]  +  SizeA[i2] * AR[i1][k];
		ExtentB = SizeB[k1] * AR[i][k2]  +  SizeB[k2] * AR[i][k1];
		Separation = FastFabsf( vSepA[i2] * R[i1][k]  -  vSepA[i1] * R[i2][k] );
		if( Separation > ExtentA + ExtentB ) return false;
		}
		}

		// No separating axis found, the boxes overlap	
		return true;*/

	}

	bool PrimOBB::Test( const PrimAABB& InP ) const
	{
		// convert to obb and test
		collision::PrimOBB InPObb;
		mat44f mat;
		mat.Translation(InP.Center());
		InPObb.Set(InP.Extent(), mat);
		return Test(InPObb);
	}

	bool PrimFrustum::Test( const PrimTri& collider ) const
	{
		for(int i=0; i<6; i++)
		{
			if( collider.Test(m_Planes[i]) == false)
				return false;
		}
		return true;
	}

	bool PrimFrustum::Test( const PrimSphere& collider ) const
	{
		for(int i=0; i<6; i++)
		{
			if( collider.TestInOut(m_Planes[i]) == collision::CIO_OUTSIDE)
				return false;
		}
		return true;
	}

	bool PrimFrustum::Test( const PrimAABB& collider ) const
	{
		for(int i=0; i<6; i++)
		{
			if( collider.Test(m_Planes[i]) == false)
				return false;
		}

		// more accurate test (based on http://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm)
		int out;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Corners[i].x > collider.m_Max.x) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Corners[i].x < collider.m_Min.x) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Corners[i].y > collider.m_Max.y) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Corners[i].y < collider.m_Min.y) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Corners[i].z > collider.m_Max.z) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Corners[i].z < collider.m_Min.z) ? 1 : 0); if (out == 8) return false;

		return true;
	}

	bool PrimFrustum::Test( const PrimOBB& collider ) const
	{
		for(int i=0; i<6; i++)
		{
			if( collider.Test(m_Planes[i]) == false)
				return false;
		}
		return true;
	}

	collision::CollisionInOutResult PrimFrustum::TestInOut( const PrimTri& collider ) const
	{
		int outsideCount = 0;
		for(int i=0; i<6; i++)
		{
			CollisionInOutResult iores = collider.TestInOut(m_Planes[i]);
			if(iores == CIO_OUTSIDE)
				++outsideCount;
			else if( iores == CIO_INTERSECTING)
				return CIO_INTERSECTING;
		}
		
		if(outsideCount == 6)
			return CIO_OUTSIDE;
		else if(outsideCount == 0)
			return CIO_INSIDE;
		else
			return CIO_INTERSECTING;
	}

	collision::CollisionInOutResult PrimFrustum::TestInOut( const PrimSphere& collider ) const
	{
		int outsideCount = 0;
		for(int i=0; i<6; i++)
		{
			CollisionInOutResult iores = collider.TestInOut(m_Planes[i]);
			if(iores == CIO_OUTSIDE)
				++outsideCount;
			else if( iores == CIO_INTERSECTING)
				return CIO_INTERSECTING;
		}

		if(outsideCount == 6)
			return CIO_OUTSIDE;
		else if(outsideCount == 0)
			return CIO_INSIDE;
		else
			return CIO_INTERSECTING;
	}

	collision::CollisionInOutResult PrimFrustum::TestInOut( const PrimAABB& collider ) const
	{
		int outsideCount = 0;
		for(int i=0; i<6; i++)
		{
			CollisionInOutResult iores = collider.TestInOut(m_Planes[i]);
			if(iores == CIO_OUTSIDE)
				++outsideCount;
			else if( iores == CIO_INTERSECTING)
				return CIO_INTERSECTING;
		}

		if(outsideCount == 6)
			return CIO_OUTSIDE;
		else if(outsideCount == 0)
			return CIO_INSIDE;
		else
			return CIO_INTERSECTING;
	}

	// ----------------------
	// MISCELLANEOUS FUNCTIONS
	// ---------------------

	void PrimSphere::Set( vec3f position, float radius )
	{
		m_Position = position;
		m_Radius = radius;
		m_Radius2 = radius * radius;
	}

	bool PrimSphere::ConvertToAABB( PrimAABB* var ) const
	{
		vec3f radVector(m_Radius, m_Radius, m_Radius);
		var->SetMinMax(m_Position - radVector, m_Position + radVector );
		return true;
	}

	void PrimPlane::Set( const vec3f& v0, const vec3f& v1, const vec3f& v2 )
	{
		m_Normal = v1 - v0;
		m_Normal = ( v2 - v0 ).Cross(m_Normal);
		m_Normal.Normalize();
		D = m_Normal.Dot( v0 );
	}

	int PrimFrustum::ClipPolygon( int vertCount, int vertStride, float* vertValues, float* vertOutValues, float distBias)
	{
		int cc=vertCount;
		float swapbuf[200];
		memcpy(vertOutValues, vertValues, sizeof(float)*vertStride*cc);

		cc=m_Planes[0].ClipPolygon(cc, vertStride, vertOutValues, swapbuf, distBias); 
		cc=m_Planes[1].ClipPolygon(cc, vertStride, swapbuf, vertOutValues, distBias); 
		cc=m_Planes[2].ClipPolygon(cc, vertStride, vertOutValues, swapbuf, distBias); 
		cc=m_Planes[3].ClipPolygon(cc, vertStride, swapbuf, vertOutValues, distBias); 
		cc=m_Planes[4].ClipPolygon(cc, vertStride, vertOutValues, swapbuf, distBias); 
		cc=m_Planes[5].ClipPolygon(cc, vertStride, swapbuf, vertOutValues, distBias);
		return cc;
	}

	int PrimPlane::ClipPolygon( int vertCount, int vertStride, float* vertValues, float* vertOutValues, float distBias /*= 0.00001f*/ )
	{
		int ncnt=0;
		float dist[32];
		bool isin[32];

		// calculate distances from plane
		for(int i=0; i<vertCount; i++)
		{
			int istride = i * vertStride;
		
			dist[i] =  m_Normal.x * vertValues[istride+0] + m_Normal.y * vertValues[istride+1] + m_Normal.z * vertValues[istride+2] - D;
			dist[i] -= distBias * fabsf(dist[i]);
			if(dist[i] >= 0.0f)
				isin[i] = true;
			else
				isin[i] = false;
		}

		for(int i=0; i<vertCount; i++)
		{
			int nexti = (i + 1) % vertCount;

			bool onein = isin[i];
			bool twoin = isin[nexti];

			bool oneouttwoin = (!onein && twoin);

			if((!onein && !twoin))
				continue; // type 4; emit nothing

			if((onein && !twoin) || oneouttwoin)
			{
				// emit intersect
				float factor = (dist[i] / (dist[i] - dist[nexti]));

				for(int c=0; c<vertStride; c++)
					vertOutValues[ncnt*vertStride+c] = vertValues[i*vertStride+c] + ( vertValues[nexti*vertStride+c] - vertValues[i*vertStride+c]) * factor;
				ncnt++;
			}

			if((onein && twoin) || oneouttwoin)
			{
				// emit v2
				for(int c=0; c<vertStride; c++)
					vertOutValues[ncnt*vertStride+c] = vertValues[nexti*vertStride+c];
				ncnt++;
			}

		}
		return ncnt;
	}

	void PrimAABB::Set( vec3f center, vec3f halfExtent )
	{
		m_Min = center - halfExtent;
		m_Max = center + halfExtent;
	}

	void PrimAABB::SetMinMax( const vec3f& min, const vec3f& max )
	{
		m_Min = min;
		m_Max = max;
		//Set((min + max) / 2.0f, (max - min) / 2.0f);
	}

	void PrimAABB::Merge( PrimAABB& other )
	{
		vec3f newMin, newMax;

		newMin.x = MIN(m_Min.x, other.m_Min.x);
		newMin.y = MIN(m_Min.y, other.m_Min.y);
		newMin.z = MIN(m_Min.z, other.m_Min.z);

		newMax.x = MAX(m_Max.x, other.m_Max.x);
		newMax.y = MAX(m_Max.y, other.m_Max.y);
		newMax.z = MAX(m_Max.z, other.m_Max.z);

		m_Min = newMin;
		m_Max = newMax;
	}

	PrimAABB& PrimAABB::operator=( const PrimTri& t )
	{
		vec3f min = t.Vertices()[0];
		vec3f max = t.Vertices()[0];

		vec3f tv[2];
		tv[0] = t.Vertices()[1];
		tv[1] = t.Vertices()[2];

		for(int d=0; d<3; d++)
		{
			for(int i=0; i<2; i++)
			{
				if(tv[i].V[d] < min.V[d])
					min.V[d] = tv[i].V[d];
				else if(tv[i].V[d] > max.V[d])
					max.V[d] = tv[i].V[d];
			}
		}

		SetMinMax(min, max);
		return (*this);
	}

	bool PrimAABB::ConvertToAABB( PrimAABB* var ) const
	{
		var->SetMinMax(m_Min, m_Max);
		return true;
	}

	void PrimAABB::GetVertices( vec3f* outVectors )
	{
		outVectors[0].Set(m_Min.x, m_Min.y, m_Min.z);
		outVectors[1].Set(m_Max.x, m_Min.y, m_Min.z);
		outVectors[2].Set(m_Min.x, m_Max.y, m_Min.z);
		outVectors[3].Set(m_Max.x, m_Max.y, m_Min.z);
		outVectors[4].Set(m_Min.x, m_Min.y, m_Max.z);
		outVectors[5].Set(m_Max.x, m_Min.y, m_Max.z);
		outVectors[6].Set(m_Min.x, m_Max.y, m_Max.z);
		outVectors[7].Set(m_Max.x, m_Max.y, m_Max.z);
	}

	vec3f PrimTri::UVToPoint( float u, float v )
	{
		vec3f v0 = m_Vertices[2] - m_Vertices[0];
		vec3f v1 = m_Vertices[1] - m_Vertices[0];

		vec3f r;
		r.x = m_Vertices[0].x + v0.x*u + v1.x*v;
		r.y = m_Vertices[0].y + v0.y*u + v1.y*v;
		r.z = m_Vertices[0].z + v0.z*u + v1.z*v;
		return r;
	}

	vec3f PrimTri::PointToUV(vec3f point)
	{
		vec3f v0 = m_Vertices[2] - m_Vertices[0];
		vec3f v1 = m_Vertices[1] - m_Vertices[0];
		vec3f v2 = point - m_Vertices[0];

		// Compute dot products
		float dot00 = v0.Dot(v0);
		float dot01 = v0.Dot(v1);
		float dot02 = v0.Dot(v2);
		float dot11 = v1.Dot(v1);
		float dot12 = v1.Dot(v2);

		// Compute barycentric coordinates
		float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
		vec3f uv;
		uv.x = (dot11 * dot02 - dot01 * dot12) * invDenom;
		uv.y = (dot00 * dot12 - dot01 * dot02) * invDenom;

		// (u > 0) && (v > 0) && (u + v < 1)
		// Check if point is in triangle
		return uv;
	}

	void PrimTri::Precompute()
	{
		m_Edge0 = m_Vertices[1] - m_Vertices[0];
		m_Edge1 = m_Vertices[2] - m_Vertices[0];
		m_Normal = m_Edge0.Cross(m_Edge1);
		m_Normal.Normalize();

		m_Dirty = false;
	}

	bool PrimTri::ConvertToAABB( PrimAABB* var ) const
	{
		vec3f min=m_Vertices[0], max = m_Vertices[0];
		min.x = MIN(min.x, m_Vertices[1].x);
		min.y = MIN(min.y, m_Vertices[1].y);
		min.z = MIN(min.z, m_Vertices[1].z);

		min.x = MIN(min.x, m_Vertices[2].x);
		min.y = MIN(min.y, m_Vertices[2].y);
		min.z = MIN(min.z, m_Vertices[2].z);

		max.x = MAX(max.x, m_Vertices[1].x);
		max.y = MAX(max.y, m_Vertices[1].y);
		max.z = MAX(max.z, m_Vertices[1].z);

		max.x = MAX(max.x, m_Vertices[2].x);
		max.y = MAX(max.y, m_Vertices[2].y);
		max.z = MAX(max.z, m_Vertices[2].z);

		var->SetMinMax(min, max);

		return true;
	}

	void PrimOBB::SetMatrix( const mat44f& m )
	{
		m_Trans = m; 
		m_InvTrans = m.Inverted();
	}


	bool PrimOBB::IsLineInBox( const vec3f& L1, const vec3f& L2 )
	{
		// Put line in gpu_box space
		vec3f LB1 = m_InvTrans * L1;
		vec3f LB2 = m_InvTrans * L2;

		// Get line midpoint and extent
		vec3f LMid = (LB1 + LB2) * 0.5f; 
		vec3f L = (LB1 - LMid);
		vec3f LExt = vec3f( fabs(L.x), fabs(L.y), fabs(L.z) );

		// Use Separating Axis Test
		// Separation vector from gpu_box center to line center is LMid, since the line is in gpu_box space
		if ( fabs( LMid.x ) > m_HalfExtent.x + LExt.x ) return false;
		if ( fabs( LMid.y ) > m_HalfExtent.y + LExt.y ) return false;
		if ( fabs( LMid.z ) > m_HalfExtent.z + LExt.z ) return false;
		// Crosss of line and each axis
		if ( fabs( LMid.y * L.z - LMid.z * L.y)  >  (m_HalfExtent.y * LExt.z + m_HalfExtent.z * LExt.y) ) return false;
		if ( fabs( LMid.x * L.z - LMid.z * L.x)  >  (m_HalfExtent.x * LExt.z + m_HalfExtent.z * LExt.x) ) return false;
		if ( fabs( LMid.x * L.y - LMid.y * L.x)  >  (m_HalfExtent.x * LExt.y + m_HalfExtent.y * LExt.x) ) return false;
		// No separating axis, the line intersects
		return true;
	}

	bool PrimOBB::BoxOutsidePlane( const PrimPlane& p ) const
	{
		vec3f InP=m_Trans.Translation();
		InP = p.FindClosestPoint(InP);

		// Plane Normal in Box Space
		vec3f Norm = m_InvTrans.Rotation() * p.m_Normal;
		Norm = vec3f( fabs( Norm.x ), fabs( Norm.y ), fabs( Norm.z ) );

		float Extent = Norm.Dot( m_HalfExtent ); // Box Extent along the plane normal
		float Distance = p.m_Normal.Dot( m_Trans.Translation() - InP ); // Distance from Box Center to the Plane

		// If Box Centerpoint is behind the plane further than its extent, the Box is outside the plane
		if ( Distance < -Extent ) return true;
		return false;
	}

	bool PrimOBB::ConvertToAABB( PrimAABB* var ) const
	{
		vec3f Center;
		Center = m_Trans.Translation();
		var->SetMinMax(Center - m_HalfExtent, Center + m_HalfExtent);
		return true;
	}

	void PrimFrustum::SetMatrix( const mat44f& ProjectionView )
	{
		const mat44f& mat = ProjectionView;
		m_SourceMatrix = mat;

		float tx = mat.MM[0][3];
		float ty = mat.MM[1][3];
		float tz = mat.MM[2][3];
		float tw = mat.MM[3][3];

		m_Planes[FRP_LEFT].m_Normal.x = tx + mat.MM[0][0];
		m_Planes[FRP_LEFT].m_Normal.y = ty + mat.MM[1][0];
		m_Planes[FRP_LEFT].m_Normal.z = tz + mat.MM[2][0];
		m_Planes[FRP_LEFT].D = tw + mat.MM[3][0];

		m_Planes[FRP_RIGHT].m_Normal.x = tx - mat.MM[0][0];
		m_Planes[FRP_RIGHT].m_Normal.y = ty - mat.MM[1][0];
		m_Planes[FRP_RIGHT].m_Normal.z = tz - mat.MM[2][0];
		m_Planes[FRP_RIGHT].D = tw - mat.MM[3][0];

		m_Planes[FRP_BOTTOM].m_Normal.x = tx + mat.MM[0][1];
		m_Planes[FRP_BOTTOM].m_Normal.y = ty + mat.MM[1][1];
		m_Planes[FRP_BOTTOM].m_Normal.z = tz + mat.MM[2][1];
		m_Planes[FRP_BOTTOM].D = tw + mat.MM[3][1];

		m_Planes[FRP_TOP].m_Normal.x = tx - mat.MM[0][1];
		m_Planes[FRP_TOP].m_Normal.y = ty - mat.MM[1][1];
		m_Planes[FRP_TOP].m_Normal.z = tz - mat.MM[2][1];
		m_Planes[FRP_TOP].D = tw - mat.MM[3][1];

		m_Planes[FRP_NEAR].m_Normal.x = tx + mat.MM[0][2];
		m_Planes[FRP_NEAR].m_Normal.y = ty + mat.MM[1][2];
		m_Planes[FRP_NEAR].m_Normal.z = tz + mat.MM[2][2];
		m_Planes[FRP_NEAR].D = tw + mat.MM[3][2];

		m_Planes[FRP_FAR].m_Normal.x = tx - mat.MM[0][2];
		m_Planes[FRP_FAR].m_Normal.y = ty - mat.MM[1][2];
		m_Planes[FRP_FAR].m_Normal.z = tz - mat.MM[2][2];
		m_Planes[FRP_FAR].D = tw - mat.MM[3][2];

		for(int i=0; i<6; i++)
		{
			m_Planes[i].D = -m_Planes[i].D;
			m_Planes[i].Normalize();
		}

		float w;
		mat44f mm = ProjectionView.Inverted();
		vec3f corner;

		w=1.0f; corner = mm.TransformVector4D( vec3f(-1, -1, -1), w );
		m_Corners[0] = vec3f( corner.x / w, corner.y / w, corner.z / w );
		w=1.0f; corner = mm.TransformVector4D( vec3f( 1, -1, -1), w );
		m_Corners[1] = vec3f( corner.x / w, corner.y / w, corner.z / w );
		w=1.0f; corner = mm.TransformVector4D( vec3f( 1,  1, -1), w );
		m_Corners[2] = vec3f( corner.x / w, corner.y / w, corner.z / w );
		w=1.0f; corner = mm.TransformVector4D( vec3f(-1,  1, -1), w );
		m_Corners[3] = vec3f( corner.x / w, corner.y / w, corner.z / w );
		w=1.0f; corner = mm.TransformVector4D( vec3f(-1, -1,  1), w );
		m_Corners[4] = vec3f( corner.x / w, corner.y / w, corner.z / w );
		w=1.0f; corner = mm.TransformVector4D( vec3f( 1, -1,  1), w );
		m_Corners[5] = vec3f( corner.x / w, corner.y / w, corner.z / w );
		w=1.0f; corner = mm.TransformVector4D( vec3f( 1,  1,  1), w );
		m_Corners[6] = vec3f( corner.x / w, corner.y / w, corner.z / w );
		w=1.0f; corner = mm.TransformVector4D( vec3f(-1,  1,  1), w );
		m_Corners[7] = vec3f( corner.x / w, corner.y / w, corner.z / w );
	}


	void PrimFrustum::SetView( const mat44f& transMat, float fov, float aspect, float vnear, float vfar )
	{
		float ymax = vnear * tanf( DEG2RAD( fov / 2.0f ) );
		float xmax = ymax * aspect;

		SetView( transMat, -xmax, xmax, -ymax, ymax, vnear, vfar );
	}

	void PrimFrustum::SetView( const mat44f& transMat, float left, float right, float bottom, float top, float vnear, float vfar )
	{
		// Use intercept theorem to get params for far plane
		float left_f = left * vfar / vnear;
		float right_f = right * vfar / vnear;
		float bottom_f = bottom * vfar / vnear;
		float top_f = top * vfar / vnear;

		// Get points on near plane
		m_Corners[0] = vec3f( left, bottom, -vnear );
		m_Corners[1] = vec3f( right, bottom, -vnear );
		m_Corners[2] = vec3f( right, top, -vnear );
		m_Corners[3] = vec3f( left, top, -vnear );

		// Get points on far plane
		m_Corners[4] = vec3f( left_f, bottom_f, -vfar );
		m_Corners[5] = vec3f( right_f, bottom_f, -vfar );
		m_Corners[6] = vec3f( right_f, top_f, -vfar );
		m_Corners[7] = vec3f( left_f, top_f, -vfar );

		// Transform points to fit camera position and rotation
		vec3f _origin = transMat * vec3f( 0, 0, 0 );
		for( unsigned int i = 0; i < 8; ++i )
			m_Corners[i] = transMat * m_Corners[i];

		// Build planes
		m_Planes[FRP_LEFT].Set( _origin, m_Corners[3], m_Corners[0] );		// Left
		m_Planes[FRP_RIGHT].Set( _origin, m_Corners[1], m_Corners[2] );		// Right
		m_Planes[FRP_BOTTOM].Set( _origin, m_Corners[0], m_Corners[1] );		// Bottom
		m_Planes[FRP_TOP].Set( _origin, m_Corners[2], m_Corners[3] );		// Top
		m_Planes[FRP_NEAR].Set( m_Corners[0], m_Corners[1], m_Corners[2] );	// Near
		m_Planes[FRP_FAR].Set( m_Corners[5], m_Corners[4], m_Corners[7] );	// Far
	}

	void PrimFrustum::SetBox( const mat44f& transMat, float left, float right, float bottom, float top, float front, float back )
	{
		// Get points on front plane
		m_Corners[0] = vec3f( left, bottom, front );
		m_Corners[1] = vec3f( right, bottom, front );
		m_Corners[2] = vec3f( right, top, front );
		m_Corners[3] = vec3f( left, top, front );

		// Get points on far plane
		m_Corners[4] = vec3f( left, bottom, back );
		m_Corners[5] = vec3f( right, bottom, back );
		m_Corners[6] = vec3f( right, top, back );
		m_Corners[7] = vec3f( left, top, back );

		// Transform points to fit camera position and rotation
		vec3f _origin = transMat * vec3f( 0, 0, 0 );
		for( unsigned int i = 0; i < 8; ++i )
			m_Corners[i] = transMat * m_Corners[i];

		// Build planes
		m_Planes[FRP_LEFT].Set( m_Corners[0], m_Corners[3], m_Corners[7] );	// Left
		m_Planes[FRP_RIGHT].Set( m_Corners[2], m_Corners[1], m_Corners[6] );	// Right
		m_Planes[FRP_BOTTOM].Set( m_Corners[0], m_Corners[4], m_Corners[5] );	// Bottom
		m_Planes[FRP_TOP].Set( m_Corners[3], m_Corners[2], m_Corners[6] );	// Top
		m_Planes[FRP_NEAR].Set( m_Corners[0], m_Corners[1], m_Corners[2] );	// Front
		m_Planes[FRP_FAR].Set( m_Corners[4], m_Corners[7], m_Corners[6] );	// Back
	}

	bool PrimFrustum::ConvertToAABB( PrimAABB* var ) const
	{
		vec3f min=m_Corners[0], max=m_Corners[0];
		for(int i=1; i<8; i++)
		{
			min.x = MIN(min.x, m_Corners[i].x);
			min.y = MIN(min.y, m_Corners[i].y);
			min.z = MIN(min.z, m_Corners[i].z);

			max.x = MAX(max.x, m_Corners[i].x);
			max.y = MAX(max.y, m_Corners[i].y);
			max.z = MAX(max.z, m_Corners[i].z);
		}
		var->SetMinMax(min, max);
		return true;
	}

	void PrimFrustum::SetCorners( const vec3f& front_leftBottom, const vec3f& front_rightBottom, const vec3f& front_rightTop, const vec3f& front_leftTop, const vec3f& back_leftBottom, const vec3f& back_rightBottom, const vec3f& back_rightTop, const vec3f& back_leftTop )
	{
		m_Corners[0] = front_leftBottom;
		m_Corners[1] = front_rightBottom;
		m_Corners[2] = front_rightTop;
		m_Corners[3] = front_leftTop;
		m_Corners[4] = back_leftBottom;
		m_Corners[5] = back_rightBottom;
		m_Corners[6] = back_rightTop;
		m_Corners[7] = back_leftTop;

		// Build planes
		m_Planes[FRP_LEFT].Set( m_Corners[0], m_Corners[3], m_Corners[7] );	// Left
		m_Planes[FRP_RIGHT].Set( m_Corners[2], m_Corners[1], m_Corners[6] );	// Right
		m_Planes[FRP_BOTTOM].Set( m_Corners[0], m_Corners[4], m_Corners[5] );	// Bottom
		m_Planes[FRP_TOP].Set( m_Corners[3], m_Corners[2], m_Corners[6] );	// Top
		m_Planes[FRP_NEAR].Set( m_Corners[0], m_Corners[1], m_Corners[2] );	// Front
		m_Planes[FRP_FAR].Set( m_Corners[4], m_Corners[7], m_Corners[6] );	// Back
	}

}

namespace collision
{



#define X 0
#define Y 1
#define Z 2

#define CROSS(dest,v1,v2) \
	dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0]; 

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
	dest[0]=v1[0]-v2[0]; \
	dest[1]=v1[1]-v2[1]; \
	dest[2]=v1[2]-v2[2]; 

#define FINDMINMAX(x0,x1,x2,min,max) \
	min = max = x0;   \
	if(x1<min) min=x1;\
	if(x1>max) max=x1;\
	if(x2<min) min=x2;\
	if(x2>max) max=x2;

	int planeBoxOverlap(double normal[3], double d, double maxbox[3])
	{
		int q;
		double vmin[3], vmax[3];
		for (q = X; q <= Z; q++)
		{
			if (normal[q] > 0.0f)
			{
				vmin[q] = -maxbox[q];
				vmax[q] = maxbox[q];
			}
			else
			{
				vmin[q] = maxbox[q];
				vmax[q] = -maxbox[q];
			}
		}

		//  --|->---[------|------]------|->---
		//    d n   vmin   0      vmax   d n
		// adjustment in second line brings in d closer by eps*d

		//  if(DOT(normal,vmin)+d>0.0f) return 0;
		//#define EPSILON 1.0e-7
		//if(DOT(normal,vmin)+d*(1.0-EPSILON)>0.0f) return 0;
		//if(DOT(normal,vmin) > -d*(1.0-EPSILON)) return 0;
		if (DOT(normal, vmin) > -d) return 0;

		//  if(DOT(normal,vmax)+d>=0.0f-ABSF(d)*1.0e-7) return 1; // attempt to fix
		//if(DOT(normal,vmax)+d*(1.0-EPSILON)>=0.0f) return 1; // attempt to fix
		//if(DOT(normal,vmax) >= -d*(1.0-EPSILON)) return 1; // attempt to fix
		// numerical problem
		if (DOT(normal, vmax) >= -d) return 1;

		return 0;
	}


	//======================== X-tests ========================
#define AXISTEST_X01(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			       	   \
	p2 = a*v2[Y] - b*v2[Z];			       	   \
	if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			           \
	p1 = a*v1[Y] - b*v1[Z];			       	   \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

	//======================== Y-tests ========================
#define AXISTEST_Y02(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p2 = -a*v2[X] + b*v2[Z];	       	       	   \
	if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p1 = -a*v1[X] + b*v1[Z];	     	       	   \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

	//======================== Z-tests ========================

#define AXISTEST_Z12(a, b, fa, fb)			   \
	p1 = a*v1[X] - b*v1[Y];			           \
	p2 = a*v2[X] - b*v2[Y];			       	   \
	if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)			   \
	p0 = a*v0[X] - b*v0[Y];				   \
	p1 = a*v1[X] - b*v1[Y];			           \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return 0;

	int intersect_tri_cube(double boxcenter[3], double boxhalfsize[3], const PrimTri &t)
	{

		//    use separating axis theorem to test overlap between triangle and gpu_box 
		//    need to test for overlap in these directions: 
		//    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle 
		//       we do not even need to test these) 
		//    2) normal of the triangle 
		//    3) Cross(edge from tri, {x,y,z}-directin) 
		//       this gives 3x3=9 more tests 
		double v0[3], v1[3], v2[3];
		double min, max, d, p0, p1, p2, rad, fex, fey, fez;
		double normal[3], e0[3], e1[3], e2[3];

		// 1) first test overlap in the {x,y,z}-directions 
		//    find min, max of the triangle each direction, and test for overlap in 
		//    that direction -- this is equivalent to testing a minimal AABB around 
		//    the triangle against the AABB 
		// This is the fastest branch on Sun 
		// move everything so that the boxcenter is in (0,0,0) 
		SUB(v0, t.Vertices()[0], boxcenter);
		SUB(v1, t.Vertices()[1], boxcenter);
		SUB(v2, t.Vertices()[2], boxcenter);

		// test in X-direction
		FINDMINMAX(v0[X], v1[X], v2[X], min, max);
		if (min > boxhalfsize[X] || max < -boxhalfsize[X]) return 0;

		// test in Y-direction
		FINDMINMAX(v0[Y], v1[Y], v2[Y], min, max);
		if (min > boxhalfsize[Y] || max < -boxhalfsize[Y]) return 0;

		// test in Z-direction
		FINDMINMAX(v0[Z], v1[Z], v2[Z], min, max);
		if (min > boxhalfsize[Z] || max < -boxhalfsize[Z]) return 0;

		//    2)
		//    test if the gpu_box intersects the plane of the triangle 
		//    compute plane equation of triangle: normal*x+d=0 
		SUB(e0, v1, v0);      // tri edge 0 
		SUB(e1, v2, v1);      // tri edge 1 
		CROSS(normal, e0, e1);
		d = -DOT(normal, v0);  // plane eq: normal.x+d=0 
		if (!planeBoxOverlap(normal, d, boxhalfsize)) return 0;

		//    compute the last triangle edge 
		SUB(e2, v0, v2);

		//    3) 
		fex = fabs(e0[X]);
		fey = fabs(e0[Y]);
		fez = fabs(e0[Z]);
		AXISTEST_X01(e0[Z], e0[Y], fez, fey);
		AXISTEST_Y02(e0[Z], e0[X], fez, fex);
		AXISTEST_Z12(e0[Y], e0[X], fey, fex);

		fex = fabs(e1[X]);
		fey = fabs(e1[Y]);
		fez = fabs(e1[Z]);
		AXISTEST_X01(e1[Z], e1[Y], fez, fey);
		AXISTEST_Y02(e1[Z], e1[X], fez, fex);
		AXISTEST_Z0(e1[Y], e1[X], fey, fex);


		fex = fabs(e2[X]);
		fey = fabs(e2[Y]);
		fez = fabs(e2[Z]);
		AXISTEST_X2(e2[Z], e2[Y], fez, fey);
		AXISTEST_Y1(e2[Z], e2[X], fez, fex);
		AXISTEST_Z12(e2[Y], e2[X], fey, fex);

		return 1;
	}

#undef X
#undef Y
#undef Z
#undef SUB
#undef CROSS
#undef DOT

	////////////////////////////////////////////////////////////////////////////
	//
	// File: intersect_ray_tri.cxx
	// Description: Pane class (Core Ray Tracing Engine) header
	// Created: 2003/07/17 09:33:17
	// Author: Kevin Beason <beason@cs.fsu.edu>
	//
	// Copyright (c) 2003-2004 Kevin Beason
	// 
	// ray-triangle intersection subroutine
	// for the Pane ray tracer.
	// For more information on Pane, visit:
	// http://www.csit.fsu.edu/~beason/pane/
	// or see the accompanying documentation
	// in the distribution.
	//
	// Quickly determines whether a given
	// ray intersects a given triangle.
	//
	// This code is from
	// http://www.ce.chalmers.se/staff/tomasm/raytri/raytri.c
	// with slight modifications.
	//
	// $Id: intersect_ray_tri.cxx 1158 2007-05-11 06:18:18Z beason $
	//
	////////////////////////////////////////////////////////////////////////////

#define EPSILON 0.000000000001
#define CROSS(dest,v1,v2) \
	dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
	dest[0]=v1[0]-v2[0]; \
	dest[1]=v1[1]-v2[1]; \
	dest[2]=v1[2]-v2[2]; 


	// code rewritten to do tests on the sign of the determinant 
	// the division is at the end in the code                    
	LIBCOLLISIONAPI int intersect_ray_tri(const float* orig, const float* dir,
		const float* vert0, const float* vert1, const float* vert2,
		double *t, double *u, double *v)
	{

		// the precision of the types below can impact speed and accuracy
		// greatly. tweak if you have problems with cracks (or don't).
		double edge1[3], edge2[3];
		double tvec[3];
		double pvec[3];
		double qvec[3];
		double det;
		double inv_det;

		// find vectors for two edges sharing vert0 
		SUB(edge1, vert1, vert0);
		SUB(edge2, vert2, vert0);

		// begin calculating determinant - also used to calculate U parameter
		CROSS(pvec, dir, edge2);

		// if determinant is near zero, ray lies in plane of triangle
		det = DOT(edge1, pvec);

		if (det > EPSILON) {

			// calculate distance from vert0 to ray origin 
			SUB(tvec, orig, vert0);

			// calculate U parameter and test bounds
			*u = DOT(tvec, pvec);
			if (*u < 0.0 || *u > det)
				return 0;

			// prepare to test V parameter
			CROSS(qvec, tvec, edge1);

			// calculate V parameter and test bounds
			*v = DOT(dir, qvec);
			if (*v < 0.0 || *u + *v > det)
				return 0;

		}
		else if (det < -EPSILON) {

			// calculate distance from vert0 to ray origin
			SUB(tvec, orig, vert0);

			// calculate U parameter and test bounds
			*u = DOT(tvec, pvec);
			if (*u > 0.0 || *u < det)
				return 0;

			// prepare to test V parameter
			CROSS(qvec, tvec, edge1);

			// calculate V parameter and test bounds
			*v = DOT(dir, qvec);
			if (*v > 0.0 || *u + *v < det)
				return 0;
		}
		else return 0;  // ray is parallel to the plane of the triangle

		inv_det = 1.0 / det;

		// calculate t, ray intersects triangle
		*t = DOT(edge2, qvec) * inv_det;
		(*u) *= inv_det;
		(*v) *= inv_det;

		return 1;
	}


	LIBCOLLISIONAPI int intersect_ray_tri_nonegativet(const float* orig, const float* dir, const float* vert0, const float* vert1, const float* vert2, double *t, double *u, double *v)
	{
		// the precision of the types below can impact speed and accuracy
		// greatly. tweak if you have problems with cracks (or don't).
		double edge1[3], edge2[3];
		double tvec[3];
		double pvec[3];
		double qvec[3];
		double det;
		double inv_det;

		// find vectors for two edges sharing vert0 
		SUB(edge1, vert1, vert0);
		SUB(edge2, vert2, vert0);

		// begin calculating determinant - also used to calculate U parameter
		CROSS(pvec, dir, edge2);

		// if determinant is near zero, ray lies in plane of triangle
		det = DOT(edge1, pvec);

		if (det > EPSILON) {

			// calculate distance from vert0 to ray origin 
			SUB(tvec, orig, vert0);

			// calculate U parameter and test bounds
			*u = DOT(tvec, pvec);
			if (*u < 0.0 || *u > det)
				return 0;

			// prepare to test V parameter
			CROSS(qvec, tvec, edge1);

			// calculate V parameter and test bounds
			*v = DOT(dir, qvec);
			if (*v < 0.0 || *u + *v > det)
				return 0;

		}
		else if (det < -EPSILON) {

			// calculate distance from vert0 to ray origin
			SUB(tvec, orig, vert0);

			// calculate U parameter and test bounds
			*u = DOT(tvec, pvec);
			if (*u > 0.0 || *u < det)
				return 0;

			// prepare to test V parameter
			CROSS(qvec, tvec, edge1);

			// calculate V parameter and test bounds
			*v = DOT(dir, qvec);
			if (*v > 0.0 || *u + *v < det)
				return 0;
		}
		else return 0;  // ray is parallel to the plane of the triangle

		inv_det = 1.0 / det;

		// calculate t, ray intersects triangle
		*t = DOT(edge2, qvec) * inv_det;
		(*u) *= inv_det;
		(*v) *= inv_det;

		if ((*t) >= 0.0)
			return 1;
		else
			return 0;
	}


}; // end namespace collision

#endif
