#pragma once

#ifndef SWIG
#ifndef WIN32
	// implement this
	#ifndef _MM_ALIGN16
	#define _MM_ALIGN16 __attribute__ ((aligned (16)))
	#endif
#endif

#ifdef _MSC_VER
//#define __forceinline __forceinline
#else
#ifndef __forceinline
#define __forceinline __attribute__((always_inline))
#endif
#endif
#endif

#include <cfc/base.h>

#include <math.h>
#include <string.h>

#ifndef PI
#define PI 3.1415926535897932384626433832795f
#endif
#ifndef DEG2RAD
# define DEG2RAD(val) ((PI/180.0f)*val)
# define RAD2DEG(val) ((val*180.0f)/PI)
# define M3D_COMPAREFP(val, val2, epsilon) (fabsf(val2-val) <= epsilon)
# define MIN(x,y) ((x)<(y)?(x):(y))
# define MAX(x,y) ((x)>(y)?(x):(y))
# define CLAMP(val, min, max) MIN(MAX(val, min), max)
# define ROUNDUPDIVI(val, round) (((val) + (round) - 1) / (round))
# define ROUNDUPI(val, round) (ROUNDUPDIVI(val, round) * (round))
#endif

namespace cfc {
namespace math {
	
	// predefinitions
	class matrix4f;
	class quatf;
	class vector3f;
	class planef;
	class trsf;
	class colorf;
	
	// functions
	static inline bool isPow2(unsigned int x) { return (x != 0) && ((x & (x - 1)) == 0); }
	CFC_API float AngularDiffRadians(float a, float b);
	CFC_API float AngularDiff(float a, float b);

	float cos(float ang);
	float sin(float ang);

	// classes
	class CFC_API vector3f
	{
	public:
		union { struct { float x, y, z; }; float V[3]; };	

		static vector3f Zero;

		vector3f::vector3f()														{ V[0] = V[1] = V[2] = 0.0f; }
		vector3f::vector3f(float X, float Y=0.0f, float Z=0.0f) : x(X), y(Y), z(Z)	{ }
		vector3f::vector3f(const float* XYZ) : x(XYZ[0]), y(XYZ[1]), z(XYZ[2])		{ }
		vector3f(bool DontInitialize) { }

		operator const float*() const { return V; }

		// for sorting/listing purposes.
		bool operator< (const vector3f &v) const;

		vector3f operator / (const vector3f) const;
		vector3f operator * (const vector3f) const;
		vector3f operator + (const vector3f) const;
		vector3f operator - (const vector3f) const;
		vector3f operator- () const													{ return vector3f( -x, -y, -z ); }

		inline vector3f& operator *= (const vector3f& a)							 { V[0] *= a.V[0]; V[1] *= a.V[1]; V[2] *= a.V[2]; return (*this); }
		inline vector3f& operator /= (const vector3f& a)							 { V[0] /= a.V[0]; V[1] /= a.V[1]; V[2] /= a.V[2]; return (*this); }
		inline vector3f& operator += (const vector3f& a)							 { V[0] += a.V[0]; V[1] += a.V[1]; V[2] += a.V[2]; return (*this); }
		inline vector3f& operator -= (const vector3f& a)							 { V[0] -= a.V[0]; V[1] -= a.V[1]; V[2] -= a.V[2]; return (*this); }

		vector3f operator * (const float) const;
		vector3f operator + (const float) const;
		vector3f operator - (const float) const;
		vector3f operator / (const float) const;

		inline vector3f& operator *= (const float a) { return (*this) = (*this) * a; }
		inline vector3f& operator += (const float a) { return (*this) = (*this) + a; }
		inline vector3f& operator -= (const float a) { return (*this) = (*this) - a; }
		inline vector3f& operator /= (const float a) { return (*this) = (*this) / a; }

		//! \name Row-ordered vector * matrix (DirectX Style)
		//@{
		vector3f operator * (const matrix4f& Mat) const;
		vector3f& operator *= (const matrix4f& Mat);
		//@}

		vector3f operator * (const quatf& Q) const;
		vector3f& operator *= (const quatf& Q);

		vector3f& operator= (const vector3f& v) { for(int i=0;i<3;++i) V[i] = v.V[i]; return *this; }

		bool operator== (const vector3f& v2) const { if(x != v2.x) return false;  if(y != v2.y) return false; if(z != v2.z) return false; return true; /*return M3D_COMPAREFP(x, v2.x, 0.001) && M3D_COMPAREFP(y, v2.y, 0.001) && M3D_COMPAREFP(z, v2.z, 0.001);*/ }
		bool operator!= (const vector3f& v2) const { return !((*this) == v2); }
		inline float& operator[] (int index) { return V[index]; }
		inline const float& operator[] (int index) const { return V[index]; }

		//! Make the vector's length 1, retaining direction.
		inline vector3f Normalized() const { vector3f clone=(*this); clone.Normalize(); return clone; }
		//! Simple yet effective, mirrors a vector over a specified point. (v = mirrorpoint + (v-mirrorpoint), v' = mirrorpoint - (v-mirrorpoint) = 2*mirrorpoint - v)
		inline vector3f Mirror(const vector3f& MirrorPoint) const { return (MirrorPoint*2.0f) - (*this); }
		inline vector3f Cross(const vector3f& v2) const { return vector3f((V[1] * v2.V[2]) - (V[2] * v2.V[1]), (V[2] * v2.V[0]) - (V[0] * v2.V[2]), (V[0] * v2.V[1]) - (V[1] * v2.V[0]));}

		inline vector3f VecX() const { vector3f ret(x, 0.0f, 0.0f); return ret; }
		inline vector3f VecY() const { vector3f ret(0.0f, y, 0.0f); return ret; }
		inline vector3f VecZ() const { vector3f ret(0.0f, 0.0f, z); return ret; }

		static vector3f UnprojectVector(vector3f vec, const matrix4f& mat, float viewportWidth, float viewportHeight);
		static vector3f ProjectVector(vector3f vec, const matrix4f& mat, float viewportWidth, float viewportHeight);

		vector3f		InterpolateTo(const vector3f& To, float maximumVelocity) const;
		vector3f		InterpolateRotation(const vector3f& To, float factor) const;
		inline vector3f Project(const vector3f& v2) { return (*this) * ((this->Dot(v2))/(this->Dot(*this))); }

		static vector3f Create() { return vector3f(); }
		static vector3f Create(float x, float y, float z) { return vector3f(x,y,z); }
		static float Dot(const vector3f& v1, const vector3f& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
	
		float GetX() { return x; }
		float GetY() { return y; }
		float GetZ() { return z; }

		void SetX(float v) { x = v; }
		void SetY(float v) { y = v; }
		void SetZ(float v) { z = v; }

		inline void Set(const vector3f& v1) { V[0]=v1[0]; V[1]=v1[1]; V[2]=v1[2]; }
		inline void Set(float v1[3]) { V[0]=v1[0]; V[1]=v1[1]; V[2]=v1[2]; }
		inline void Set(float X=0.0f, float Y=0.0f, float Z=0.0f) { x=X; y=Y; z=Z; }

		void SetTo(const vector3f& other) { (*this) = other; }
		void SetToMirror(const vector3f& v0, const vector3f& mirrorPoint) {(*this) = v0.Mirror(mirrorPoint); }
		void SetToCross(const vector3f& v1, const vector3f& v2) { (*this) = v1.Cross(v2); }
		void SetToUnproject(const vector3f& v0, const matrix4f& mat, float viewportWidth, float viewportHeight) { (*this) = vector3f::UnprojectVector(v0, mat, viewportWidth, viewportHeight); }
		void SetToProject(const vector3f& v0, const matrix4f& mat, float viewportWidth, float viewportHeight) { (*this) = vector3f::ProjectVector(v0, mat, viewportWidth, viewportHeight); }
		void SetToInterpolatedRotation(const vector3f& v0, const vector3f& v1, float factor) { (*this) = v0.InterpolateRotation(v1, factor); }
		void SetToMatrixLocalX(const matrix4f& m);
		void SetToMatrixLocalY(const matrix4f& m);
		void SetToMatrixLocalZ(const matrix4f& m);
		void SetToMatrixViewLocalX(const matrix4f& m);
		void SetToMatrixViewLocalY(const matrix4f& m);
		void SetToMatrixViewLocalZ(const matrix4f& m);
		void SetToMatrixTranslation(const matrix4f& m);
		void SetToQuatLocalX(const quatf& m);
		void SetToQuatLocalY(const quatf& m);
		void SetToQuatLocalZ(const quatf& m);
		void Transform(const matrix4f& pre_m);
		void Add(const vector3f& v) { (*this) += v; }
		void Sub(const vector3f& v) { (*this) -= v; }
		void Mul(const vector3f& v) { (*this) *= v; }
		void MulNumber(float v) { (*this) *= v; }
		void Normalize() { float magnitude=1.0f/sqrtf(V[0] * V[0] + V[1] * V[1] + V[2] * V[2]); V[0] *= magnitude; V[1] *= magnitude; V[2] *= magnitude; }
		float Dot(const vector3f& v2) const { return x * v2.x + y * v2.y + z * v2.z; }

		//! Get the angle between two vectors. Uses Dot (Dot(a,b) = mag(a)*mag(b)*cos(betweenAngle)), returns betweenAngle.
		float AngleBetween(const vector3f& v2) const;
		float SqrLength() const;
		float Length() const;
	private:
	};


	class CFC_API planef
	{
	public:
		union { struct { float a, b, c, d; }; };

		planef() { a=b=c=d=0.0f; }

		inline void CreatePlane(vector3f PointOnPlane, vector3f PlaneNormal) { a = PlaneNormal.x; b = PlaneNormal.y; c = PlaneNormal.z; d = -PlaneNormal.Dot(PointOnPlane); }

		//! Find closest point to specified Point on the plane
		inline vector3f GetClosestPoint(const vector3f& Point) const { vector3f normal(a,b,c); return Point - normal * (d + Point.Dot(normal)); }
	
		//! Reflects vector over plane
		inline vector3f ReflectVector(const vector3f& Reflect) const { return Reflect.Mirror(GetClosestPoint(Reflect)); }

		//! Returns this plane, only normalized.
		planef GetNormalizedPlane() const;
		//! Gives the result of the Ax+By+Cz+D computation.
		inline float ClassifyPoint(const vector3f& pt) const { return a*pt.x + b*pt.y + c*pt.z + d; }
	};



	/*!
		\brief Column-major Matrix class. (Matrix[column (x)][row (y)])
		\author Seniltai
	*/
	class CFC_API matrix4f
	{
	public:
		matrix4f() 
		{ 
			Identity(); 
		}
		matrix4f(bool dontInitialize) 
		{ 
		}
		matrix4f(float Value) 
		{ 
			Zero(Value); 
		}
		matrix4f(const matrix4f& m) 
		{ 
			memcpy(M, m.M, 64); 
		}

		union
		{
			float MM[4][4]; // x<column>, y<row>
			float M[16]; // index formula x<column>*4+y<row>
		};

		inline const float operator[](int idx) const { return M[idx]; }
		inline float& operator[](int idx) { return M[idx]; }

		inline matrix4f operator * (const matrix4f& Mat) const { matrix4f mnew(*this); mnew *= Mat; return mnew; }
		inline matrix4f operator ^ (const matrix4f& Mat) const { matrix4f mnew(*this); mnew ^= Mat; return mnew; }
		inline matrix4f operator + (const matrix4f& Mat) const { matrix4f mnew(*this); mnew += Mat; return mnew; }
		inline matrix4f operator - (const matrix4f& Mat) const { matrix4f mnew(*this); mnew -= Mat; return mnew; }

		vector3f operator * (const vector3f& V) const;	//!< \name matrix * Column-ordered vector (OpenGL Style)
		matrix4f& operator *= (const matrix4f& Mat);		//!< Matrix Concatenation (opengl/mathematics style, also known as post-multiplication)(normal way, r11 = m11*n11+m21*n12+m31*n13+m41*n14, etc.)
		matrix4f& operator ^= (const matrix4f& Mat);		//!< Inverse Matrix Concatenation (directX style, also known as pre-multiplication)
		matrix4f& operator += (const matrix4f& Mat);		//!< Matrix Addition
		matrix4f& operator -= (const matrix4f& Mat);		//!< Matrix Substraction

		operator const float*() const { return M; }

		//! Matrix Comparison Operator
		bool operator == (const matrix4f& Mat) const;

		//! Matrix Inverse Comparison Operator
		inline bool operator != (const matrix4f& Mat) const { return !(*this == Mat); }

		//! Fill the matrix with zeroes
		void Zero(float Value=0.0f);
		//! Make the matrix an identity matrix (default at construction)
		void Identity(float scale=1.0f);
		//! Calculate the determinant of the matrix (when 0, the matrix has no inverse.)
		float Determinant();
		//! Transpose the matrix (switch row to column and vice versa)
		void Transpose();
	
		//! Matrix inverse (is a pretty costly task so use with care.)
		bool Inverse();

		//! Row-wise normalisation (Normalizes every axis matrix row to compensate for matrixial drift, shearing your values.)
		void Normalize();

		float Sum() const { float sum=0.0f; for(int y=0; y<4; y++) { for(int x=0;x<4;x++) { sum += fabsf(MM[x][y]); } } return sum; }


		void Translate3(const vector3f& Translate);
		void Scale3(const vector3f& Scale);

		void Translate3(float x, float y, float z) { Translate3(vector3f(x,y,z)); }
		void Scale3(float x, float y, float z) { Scale3(vector3f(x,y,z)); }
		void ScaleUniform(float xyz) { Scale3(vector3f(xyz,xyz,xyz)); }

		void RotateX(float Theta);
		void RotateY(float Theta);
		void RotateZ(float Theta);

		//! Euler coordinates -> Matrix
		void RotateEuler(float RotX, float RotY, float RotZ);
		inline void RotateEuler(vector3f Rot) { RotateEuler(Rot.x, Rot.y, Rot.z); }

		//! Rotate over an axis by a certain angle.
		void RotateAxisAngle(const vector3f& Axis, float angle);

		//! Matrix Multiplication, mostly used for multiplying with a matrix of values. (r11 = m11*n11)
		void MultiplyValues(matrix4f& Mat);

		void SetTo(const float* m);
		void SetTo(const matrix4f& other) { (*this) = other; }
		void Mul(const matrix4f& other) { (*this) *= other; }
		void MulVector(vector3f& other) { other = (*this) * other; }
		void MulVectorInto(const vector3f& other, vector3f& into) { into = (*this) * other; }

		void SetLocalX(const vector3f& a) { MM[0][0] = a.x; MM[0][1] = a.y; MM[0][2] = a.z; }
		void SetLocalY(const vector3f& a) {  MM[1][0] = a.x; MM[1][1] = a.y; MM[1][2] = a.z; }
		void SetLocalZ(const vector3f& a) { MM[2][0] = a.x; MM[2][1] = a.y; MM[2][2] = a.z; }

		float Get(int column, int row) { return MM[column][row]; } //!< Function used to get Lua the matrix values
		void Set(int column, int row, float value) { MM[column][row] = value; } //!< Function used to set matrix values in Lua.

		bool IsIdentity() const { static matrix4f IdentMatrix; if(*this == IdentMatrix) return true; else return false; }

		void LerpWith(const matrix4f& b, float fac);

		//! Calculates the determinant of a 3x3 matrix. (internally used but available for custom use as well)
		float Determinant3x3(float m11, float m21, float m31, float m12, float m22, float m32, float m13, float m23, float m33) { return (m21*m32*m13+m31*m12*m23+m11*m22*m33)-(m33*m12*m21+m23*m32*m11+m13*m22*m31); }

		void SetProjection(float FieldOfViewYDegrees, float AspectRatio, float Near, float Far) { (*this) = matrix4f::Projection(FieldOfViewYDegrees, AspectRatio, Near, Far); }
		void SetProjectionOrtho(float Width, float Height) { (*this) = matrix4f::ProjectionOrtho(Width, Height); }
		void SetView(const vector3f& Eye, const vector3f& LookAt, const vector3f& Up) { (*this) = matrix4f::View(Eye, LookAt, Up); }
		void SetTransformIntoCameraMatrix(const matrix4f& m) {(*this) = TransformIntoCameraMatrix(m); }
		void SetViewDirection(const vector3f& Position, const vector3f& Direction, const vector3f& Up = vector3f(0.0f, 0.0f, 1.0f));
		void SetTranslation(const vector3f& set) { MM[3][0] = set.x, MM[3][1] = set.y, MM[3][2] = set.z; }

		// unrolled vectorless functions
		void SetView(float Eye_x, float Eye_y, float Eye_z, float Lookat_x, float Lookat_y, float Lookat_z, float Up_x, float Up_y, float Up_z) { (*this) = matrix4f::View(vector3f(Eye_x, Eye_y, Eye_z), vector3f(Lookat_x, Lookat_y, Lookat_z), vector3f(Up_x, Up_y, Up_z)); }
		void SetTranslation(float x, float y, float z) { MM[3][0] = x, MM[3][1] = y, MM[3][2] = z; }
		void SetViewDirection(float Eye_x, float Eye_y, float Eye_z, float Direction_x, float Direction_y, float Direction_z, float Up_x, float Up_y, float Up_z, bool Transpose = true) { SetViewDirection(vector3f(Eye_x, Eye_y, Eye_z), vector3f(Direction_x, Direction_y, Direction_z), vector3f(Up_x, Up_y, Up_z)); }

		inline float GetTranslationX() const { return MM[3][0]; }
		inline float GetTranslationY() const { return MM[3][1]; }
		inline float GetTranslationZ() const { return MM[3][2]; }

		float* GetRawFloats() { return M; }

	#ifndef SWIG
		inline float& GetColumnMajor(int onedeeindex) { return MM[onedeeindex % 4][(int)floor(static_cast<float>(onedeeindex)/4.0f)]; } //! Column major (0, 1, 2, 3 = row 0, column 0, 1, 2, 3)
		inline float& GetRowMajor(int onedeeindex) { return MM[(int)floor(static_cast<float>(onedeeindex)/4.0f)][onedeeindex % 4]; } //!< Row major (0, 1, 2, 3 = row 0, 1, 2, 3, column 0)

		void TransformVectorFloatArray( float* arr ) const;
		vector3f TransformVector4D(const vector3f& xyz, float& w) const;
		vector3f TransformVectorPerspectiveDivide(const vector3f& xyz) const;

		inline matrix4f Transposed() const { matrix4f mCopy(*this); mCopy.Transpose(); return mCopy; }
		inline matrix4f Inverted() const { matrix4f mCopy(*this); mCopy.Inverse(); return mCopy; }
		inline matrix4f Normalized() const { matrix4f mCopy(*this); mCopy.Normalize(); return mCopy; }

		static matrix4f TransformIntoCameraMatrix(const matrix4f& m);
		static matrix4f Projection(float FieldOfViewYDegrees, float AspectRatio, float Near, float Far);
		static matrix4f ProjectionOrtho(float Width, float Height);
		static matrix4f View(const vector3f& Eye, const vector3f& LookAt, const vector3f& Up);

		//! \name Matrix Information
		//@{

		inline vector3f ViewLocalX() const { return vector3f(MM[0][0],MM[1][0], MM[2][0]); }
		inline vector3f ViewLocalY() const  { return vector3f(MM[0][1],MM[1][1], MM[2][1]); }
		inline vector3f ViewLocalZ() const { return vector3f(MM[0][2],MM[1][2], MM[2][2]); }
		inline vector3f GetLocalX() const { return vector3f(MM[0][0],MM[0][1], MM[0][2]); }
		inline vector3f GetLocalY() const  { return vector3f(MM[1][0],MM[1][1], MM[1][2]); }
		inline vector3f GetLocalZ() const { return vector3f(MM[2][0],MM[2][1], MM[2][2]); }

		inline vector3f Translation() const { return vector3f(MM[3][0],MM[3][1], MM[3][2]); }
		inline matrix4f& Translation(const vector3f& set) { MM[3][0] = set.x, MM[3][1] = set.y, MM[3][2] = set.z; return (*this); }

		inline vector3f GetTranslation() const { return vector3f(MM[3][0],MM[3][1], MM[3][2]); }
	
		inline matrix4f Rotation(bool normalize=false) const { matrix4f Rot=(*this); Rot.MM[3][0] = Rot.MM[3][1] = Rot.MM[3][2] = 0.0f; if(normalize) Rot.Normalize(); return Rot; }
		vector3f RotationEuler() const;
		//@}

	#endif

	
	};

	class CFC_API quatf
	{
	public:
		quatf(float sx = 0.0f, float sy = 0.0f, float sz = 0.0f, float sw = 1.0f) : x(sx), y(sy), z(sz), w(sw) {}
		~quatf();
	
		inline void Set(float sx, float sy, float sz, float sw) {x = sx, y = sy, z = sz, w = sw;}
		quatf& operator = (quatf Qt);
		void Reset();																							//!< Reset the quaternion to the identity quaternion.

		//! Spherical linear interpolation between quaternion q1 and q2 by factor k <0 .. 1>
		static quatf Slerp(const quatf& q1, quatf q2, float k);
		//! Normalised linear interpolation between quaternion q1 and q2 by factor k <0 .. 1>
		static quatf Nlerp(const quatf& q1, const quatf& q2, float k);

		float Dot(const quatf &q2) const { return w*q2.w+x*q2.x+y*q2.y+z*q2.z; }
		quatf Cross(const quatf &q2) const { quatf cr; cr.w = 0; cr.x = (y * q2.z) - (z * q2.y); cr.y = (x * q2.z) - (z * q2.x); cr.z = (x * q2.y) - (y * q2.x); return cr; }

		void ShortestArcToQuat(const vector3f& From, const vector3f& To );
		void AxisAngleToQuat(const vector3f& Axis, float Angle);
		void EulerToQuat(float X, float Y, float Z);
		void VectorToQuat(const vector3f& vec, const vector3f& up=vector3f(0.0f, 0.0f, 1.0f));

		bool QuatToEuler(float &X, float &Y, float &Z) const;
		bool QuatToAxisAngle(vector3f& Axis, float& Angle);

		void NegateQuat() { x = -x; y = -y; z = -z; w = -w; } // gives the same rotation but a different quaternion.
		inline void ConjugateQuat() { x = -x; y = -y; z = -z; }
		inline void InverseQuat() { NormalizeQuat(); ConjugateQuat();  }
		void NormalizeQuat() { float rMag = 1.0f/MagnitudeQuat(); w *= rMag; x *= rMag; y *= rMag; z *= rMag; }		//!< set length to 1
		float MagnitudeQuat() { return( sqrtf(w*w+x*x+y*y+z*z)); }													//!< Get the length of the quaternion.

		void ToMatrix(matrix4f& M) const;																						//!< get 4x4 rotation matrix
		void FromMatrix(const matrix4f &M);																			//!< 4x4 rotation matrix to quat
		void FromMatrixSecondary(const matrix4f& M);																	//!< 4x4 rotation matrix to quat - second algorithm

		void Mul(quatf& q);
		void MulInto(quatf& q, quatf& outQ);
		void MulVector(vector3f& v);
		void MulVectorInto(vector3f& v, vector3f& into);

	#ifndef SWIG
		static quatf CreateShortestArquatf(const vector3f& From, const vector3f& To) { quatf q; q.ShortestArcToQuat(From, To); return q;}
		static quatf CreateAxisAngleQuat(const vector3f& Axis, float Angle) { quatf q; q.AxisAngleToQuat(Axis, Angle); return q;}
		static quatf CreateEulerQuat(const vector3f& XYZ) { quatf q; q.EulerToQuat(XYZ.x, XYZ.y, XYZ.z); return q; }
		static quatf CreateEulerQuat(float X, float Y, float Z) { quatf q; q.EulerToQuat(X, Y, Z); return q;}
		static quatf CreateVectorQuat(const vector3f& vec, const vector3f& up=vector3f(0.0f, 0.0f, 1.0f)) { quatf q; q.VectorToQuat(vec, up); return q; }

		matrix4f Matrix() const;																						//!< get 4x4 rotation matrix
		inline vector3f EulerRotation() const { vector3f ret; QuatToEuler(ret.x, ret.y, ret.z); return ret; }
		inline vector3f GetLocalX() const { return vector3f(1.0f - 2.0f * ( y * y + z * z ), 2.0f * ( x * y - w * z ), 2.0f * ( x * z + w * y )); }
		inline vector3f GetLocalY() const { return vector3f(2.0f * ( x * y + w * z ), 1.0f - 2.0f * ( x * x + z * z ), 2.0f * ( y * z - w * x )); }
		inline vector3f GetLocalZ() const { return vector3f(2.0f * ( x * z - w * y ), 2.0f * ( y * z + w * x ), 1.0f - 2.0f * ( x * x + y * y )); }

		inline vector3f ViewLocalX() const { return vector3f(1.0f - 2.0f * (y * y + z * z), 2.0f * (x * y + w * z),        2.0f * (x * z - w * y)); }
		inline vector3f ViewLocalY() const { return vector3f(2.0f * (x * y - w * z)       , 1.0f - 2.0f * (x * x + z * z), 2.0f * (y * z + w * x)); }
		inline vector3f ViewLocalZ() const { return vector3f(2.0f * (x * z + w * y)       , 2.0f * (y * z - w * x),        1.0f - 2.0f * (x * x + y * y)); }

		inline quatf& operator += (const quatf& Qt) { x += Qt.x; y += Qt.y; z += Qt.z; w += Qt.w; return *this; }
		inline quatf operator + (const quatf& Qt) const { quatf qres=*this; qres += Qt; return qres; }
		inline quatf& operator -= (const quatf& Qt) { x -= Qt.x; y -= Qt.y; z -= Qt.z; w -= Qt.w; return *this; }
		inline quatf operator - (const quatf& Qt) const { quatf qres=*this; qres -= Qt; return qres; }
		inline quatf& operator *= (const float Val) { x *= Val; y *= Val; z *= Val; w *= Val; return *this; }
		inline quatf operator * (const float Val) const { quatf qres=*this; qres *= Val; return qres; }
		quatf& operator /= (const quatf& Qt);
		inline quatf operator / (const quatf& Qt) const { quatf qres=*this; qres /= Qt; return qres; }
		quatf& operator *= (const quatf& Qt);
		inline quatf operator * (const quatf& Qt) const { quatf qres=*this; qres *= Qt; return qres; }

		inline quatf& operator ^= (const quatf& Qt) { quatf qRes=Qt; qRes *= (*this); (*this) = qRes; return (*this); }
		inline quatf operator ^ (const quatf& Qt) const { quatf qres=*this; qres ^= Qt; return qres; }

		union
		{
			struct
			{
				float x, y, z, w;
			};
			float xyzw[4];
		};
	#else // swig wrapper code
		float xyzw[4];
		float x,y,z,w;
	#endif
	};

	class CFC_API trsf
	{
	public:
		trsf() { Scale.Set(1.0f, 1.0f, 1.0f); }
		trsf(const matrix4f& m) { FromMatrix(m); }

		vector3f Translation;
		quatf Rotation;
		vector3f Scale;

		void FromMatrix(const matrix4f& m);
		void ToExistingMatrix( matrix4f& m ) const;
		matrix4f ToMatrix() const { matrix4f m; ToExistingMatrix(m); return m;}
		void Concatenate(const trsf& other);

		static trsf Interpolate(const trsf& f1, const trsf& f2, float factor, bool slerp=false);
	};

	class CFC_API colorf
	{
	public:
		colorf(): r(0.0f), g(0.0f), b(0.0f), a(0.0f) {  }
		colorf(bool NoInit) { }
		colorf(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}

		union
		{
			float rgba[4];
			struct { float r, g, b, a; };
		};

		float operator [] (int idx) const { return rgba[idx]; }
		
		colorf& operator += (const colorf& o) { r += o.r; g += o.g; b += o.b; a += o.a; return *this; }
		colorf& operator -= (const colorf& o) { r -= o.r; g -= o.g; b -= o.b; a -= o.a; return *this; }
		colorf& operator *= (const colorf& o) { r *= o.r; g *= o.g; b *= o.b; a *= o.a; return *this; }
		colorf& operator /= (const colorf& o) { r /= o.r; g /= o.g; b /= o.b; a /= o.a; return *this; }
			  
		colorf& operator += (const float o) { r += o; g += o; b += o; a += o; return *this; }
		colorf& operator -= (const float o) { r -= o; g -= o; b -= o; a -= o; return *this; }
		colorf& operator *= (const float o) { r *= o; g *= o; b *= o; a *= o; return *this; }
		colorf& operator /= (const float o) { r /= o; g /= o; b /= o; a /= o; return *this; }

		colorf operator + (const colorf& o) const { colorf c = *this; c += o; return c; }
		colorf operator - (const colorf& o) const { colorf c = *this; c -= o; return c; }
		colorf operator * (const colorf& o) const { colorf c = *this; c *= o; return c; }
		colorf operator / (const colorf& o) const { colorf c = *this; c /= o; return c; }

		colorf operator + (const float o) const { colorf c = *this; c += o; return c; }
		colorf operator - (const float o) const { colorf c = *this; c -= o; return c; }
		colorf operator * (const float o) const { colorf c = *this; c *= o; return c; }
		colorf operator / (const float o) const { colorf c = *this; c /= o; return c; }

		colorf WithoutAlpha() const { colorf clr; clr.r = r; clr.g = g; clr.b = b; clr.a = 0.0f; return clr; }
		unsigned int ToRGBA32();
	};

	struct CFC_API approximateVectorCompare
	{	// functor for operator<
		bool operator()(const vector3f& _Left, const vector3f& _Right) const;
	};

	
	class lerp
	{
	public:
		template <class TResult, class TInterpolator> static inline TResult	Lerp(const TResult v0, const TResult v1, const TInterpolator fac) { return v0 + (v1 - v0) * fac; }
		static inline cfc::math::vector3f	Lerp(const cfc::math::vector3f& v0, const cfc::math::vector3f& v1, const float fac)		{ return v0 + (v1 - v0) * fac; }
		static inline cfc::math::colorf		Lerp(const cfc::math::colorf& v0, const cfc::math::colorf& v1, const float fac)			{ return v0 + (v1 - v0) * fac; }
		static inline cfc::math::quatf		Lerp(const cfc::math::quatf& v0, const cfc::math::quatf& v1, const float fac)			{ return cfc::math::quatf::Nlerp(v0, v1, fac); }
	};
}; // end namespace math
}; // end namespace engine

