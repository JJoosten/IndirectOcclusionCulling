#include <cfc/base.h>
#include <cfc/math/math.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

namespace cfc
{
namespace math
{

bool vector3f::operator< (const vector3f &v) const
{
	if (x!=v.x) return x<v.x;
	if (y!=v.y) return y<v.y;
	return z<v.z;
}

float vector3f::AngleBetween(const vector3f& v2) const
{
	const vector3f& v1=*this;
	float angle;
	angle = v1.Dot(v2) / (v1.Length()*v2.Length());
	angle = acosf(angle);
	return angle;
}

vector3f vector3f::operator* (const vector3f dest) const
{
	vector3f tmp;

	tmp.V[0] = V[0]* dest.V[0];
	tmp.V[1] = V[1]* dest.V[1];
	tmp.V[2] = V[2]* dest.V[2];

	return tmp;
}

vector3f vector3f::operator+ (const vector3f dest2) const
{
	vector3f tmp;
	tmp.V[0] = V[0]+ dest2.V[0];
	tmp.V[1] = V[1]+ dest2.V[1];
	tmp.V[2] = V[2]+ dest2.V[2];

	return vector3f(tmp.V[0],tmp.V[1],tmp.V[2]);
}


vector3f vector3f::operator- (const vector3f dest3) const
{
	vector3f tmp;
	tmp.V[0] = V[0]- dest3.V[0];
	tmp.V[1] = V[1]- dest3.V[1];
	tmp.V[2] = V[2]- dest3.V[2];

	return tmp;
}

vector3f vector3f::operator+ (const float scale) const
{
	vector3f tmp;
	tmp.V[0] = V[0]+ scale;
	tmp.V[1] = V[1]+ scale;
	tmp.V[2] = V[2]+ scale;

	return vector3f(tmp.V[0],tmp.V[1],tmp.V[2]);
}

vector3f vector3f::operator- (const float scale) const
{
	vector3f tmp;
	tmp.V[0] = V[0]- scale;
	tmp.V[1] = V[1]- scale;
	tmp.V[2] = V[2]- scale;

	return vector3f(tmp.V[0],tmp.V[1],tmp.V[2]);
}

vector3f vector3f::operator* (const float scale) const
{
	vector3f tmp;
	tmp.V[0] = V[0]* scale;
	tmp.V[1] = V[1]* scale;
	tmp.V[2] = V[2]* scale;

	return vector3f(tmp.V[0],tmp.V[1],tmp.V[2]);
}
vector3f vector3f::operator/ (const float scale) const
{
	vector3f tmp;
	tmp.V[0] = V[0]/ scale;
	tmp.V[1] = V[1]/ scale;
	tmp.V[2] = V[2]/ scale;

	return vector3f(tmp.V[0],tmp.V[1],tmp.V[2]);
}

vector3f vector3f::operator/( const vector3f dest ) const
{
	vector3f tmp;

	tmp.V[0] = V[0]/ dest.V[0];
	tmp.V[1] = V[1]/ dest.V[1];
	tmp.V[2] = V[2]/ dest.V[2];

	return tmp;
}

vector3f vCross(const vector3f& v1,const vector3f& v2)
{
	vector3f result( (v1.V[1] * v2.V[2]) - (v1.V[2] * v2.V[1]), (v1.V[2] * v2.V[0]) - (v1.V[0] * v2.V[2]),	(v1.V[0] * v2.V[1]) - (v1.V[1] * v2.V[0]));
	return result;
}

vector3f vNormalize(const vector3f& n)
{
	vector3f tmp(n.V[0],n.V[1],n.V[2]);
	tmp.Normalize();
	return tmp;
}

float vDot(const vector3f &v1,const vector3f &v2)
{
	return (v1.V[0] * v2.V[0]) + (v1.V[1] * v2.V[1]) + (v1.V[2] * v2.V[2]);
}

float vLength(const vector3f &v1)
{
	return (float)sqrt((v1.V[0]*v1.V[0])+(v1.V[1]*v1.V[1])+(v1.V[2]*v1.V[2])); 
}

vector3f vLerp( const vector3f &v1,const vector3f &v2, float fac )
{
	return v1+(v2-v1)*fac;
}


vector3f vector3f::operator * (const matrix4f& Mat) const
{
	vector3f NewVector=*this;
	NewVector *= Mat;
	return NewVector;
}

vector3f& vector3f::operator *= (const matrix4f& Mat)
{
	// row ordered vector * matrix

	//         m00 m10 m20 m30                      m00*x+m01*y+m02*z+m03*1
	// x y z * m01 m11 m21 m31 = newx newy newz 1 = m10*x+m11*y+m12*z+m13*1
	//         m02 m12 m22 m32                      m20*x+m21*y+m22*z+m23*1
	//         0   0   0   1                          0*x+  0*y+  0*z+  1*1

	vector3f Vec=(*this);
	V[0] = Vec.V[0]*Mat.MM[0][0] + Vec.V[1]*Mat.MM[0][1] + Vec.V[2]*Mat.MM[0][2] + Mat.MM[0][3];
	V[1] = Vec.V[0]*Mat.MM[1][0] + Vec.V[1]*Mat.MM[1][1] + Vec.V[2]*Mat.MM[1][2] + Mat.MM[1][3];
	V[2] = Vec.V[0]*Mat.MM[2][0] + Vec.V[1]*Mat.MM[2][1] + Vec.V[2]*Mat.MM[2][2] + Mat.MM[2][3];
	return (*this);
}

vector3f vector3f::operator * (const quatf& Q) const
{
	vector3f NewVector=*this;
	NewVector *= Q;
	return NewVector;
}
vector3f& vector3f::operator *= (const quatf& Q)
{
	quatf self(x,y,z, 0.0f);
	quatf Qcjg(-Q.x, -Q.y, -Q.z, Q.w);
	//self = (Q * (self * Qcjg) );
	self = (Q *  self) * Qcjg;

	x = self.x;
	y = self.y;
	z = self.z;

	return (*this);
}

float vector3f::Length() const
{
	return sqrtf(V[0] * V[0] + V[1] * V[1] + V[2] * V[2]);
}

vector3f vector3f::InterpolateRotation( const vector3f& To, float factor ) const
{
	/*trsf trs1, trs2, trsFinal;
	trs1.FromMatrix(matrix4f::View(vector3f(0.0f), *this, vector3f));
	trs2.FromMatrix(matrix4f::View(vector3f(0.0f), To));
	trsFinal = trsf::Interpolate(trs1, trs2, factor, false);
	return trsFinal.ToMatrix().GetLocalX();*/

	quatf q1,q2, qfinal;
	q1.VectorToQuat((*this));
	q2.VectorToQuat(To);
	qfinal = quatf::Nlerp(q1,q2, factor);
	return qfinal.GetLocalX();
}

vector3f vector3f::UnprojectVector( vector3f vec, const matrix4f& mat, float viewportWidth, float viewportHeight )
{
	vec.x = (((vec.x) * 2.0f) / viewportWidth) - 1.0f;
	vec.y = (((vec.y) * 2.0f) / viewportHeight) - 1.0f;
	vec.z = (vec.z * 2.0f) - 1.0f;

	float w=1.0f;
	vec = mat.TransformVector4D(vec, w);
	vec /= w; // perspective divide

	return vec;
}

vector3f vector3f::ProjectVector( vector3f vec, const matrix4f& m, float viewportWidth, float viewportHeight )
{
	vector3f Res;
	Res.x = vec.x*m.MM[0][0] + vec.y*m.MM[1][0] + vec.z*m.MM[2][0] + m.MM[3][0];
	Res.y = vec.x*m.MM[0][1] + vec.y*m.MM[1][1] + vec.z*m.MM[2][1] + m.MM[3][1];
	Res.z = vec.x*m.MM[0][2] + vec.y*m.MM[1][2] + vec.z*m.MM[2][2] + m.MM[3][2];
	float w =  vec.x*m.MM[0][3] + vec.y*m.MM[1][3] + vec.z*m.MM[2][3] + m.MM[3][3];

	Res /= w;

	Res.x = viewportWidth * (Res.x + 1.0f) / 2.0f;
	Res.y = viewportHeight * (Res.y + 1.0f) / 2.0f;
	Res.z = (Res.z + 1.0f) / 2.0f;

	return Res;
}

cfc::math::vector3f vector3f::InterpolateTo(const vector3f& To, float maximumVelocity) const
{
	cfc::math::vector3f ret = To - *this;
	float len = ret.Length();
	if (len > maximumVelocity)
	{
		ret *= maximumVelocity / len;
	}
	ret += *this;
	return ret;
}

float vector3f::SqrLength() const
{
	return Dot(*this);
}


void vector3f::SetToMatrixLocalX( const matrix4f& m )
{
	(*this) = m.GetLocalX();
}

void vector3f::SetToMatrixLocalY( const matrix4f& m )
{
	(*this) = m.GetLocalY();
}

void vector3f::SetToMatrixLocalZ( const matrix4f& m )
{
	(*this) = m.GetLocalZ();
}

void vector3f::SetToMatrixViewLocalX( const matrix4f& m )
{
	(*this) = m.ViewLocalX();
}

void vector3f::SetToMatrixViewLocalY( const matrix4f& m )
{
	(*this) = m.ViewLocalY();
}

void vector3f::SetToMatrixViewLocalZ( const matrix4f& m )
{
	(*this) = m.ViewLocalZ();
}

void vector3f::SetToMatrixTranslation( const matrix4f& m )
{
	(*this) = m.Translation();
}

void vector3f::Transform( const matrix4f& pre_m )
{
	(*this) = pre_m * (*this);
}

void vector3f::SetToQuatLocalX( const quatf& m )
{
	auto vec=m.GetLocalX();
	*this = vec;
}

void vector3f::SetToQuatLocalY( const quatf& m )
{
	auto vec=m.GetLocalY();
	*this = vec;
}

void vector3f::SetToQuatLocalZ( const quatf& m )
{
	auto vec=m.GetLocalZ();
	*this = vec;
}

vector3f vector3f::Zero(0.0f, 0.0f, 0.0f);

planef planef::GetNormalizedPlane() const
{
	planef P;
	float magni=sqrtf(a*a+b*b+c*c);
	P.a = a/magni;
	P.b = b/magni;
	P.c = c/magni;
	P.d = d/magni;
	return P;
}

bool matrix4f::operator == (const matrix4f& Mat) const
{
	if(memcmp(MM, Mat.MM, sizeof(float)*16) == 0)
		return true;
	else
		return false;

	return true;
}

/*
matrix4f& matrix4f::operator = (const matrix4f& Mat)
{
	// This is probably the fastest way to copy a matrix.
	for(int i=0; i<16; i++)
		FlatMatrix[i] = Mat.FlatMatrix[i];
	//memcpy(this->Matrix,Mat.Matrix, 64); // 4*4 = 16
	return *this;
}*/

vector3f matrix4f::operator * (const vector3f& Vec) const
{
	// matrix * column ordered vector

	// m00 m10 m20 m30   x   newx   m00*x+m10*y+m20*z+m30*1
	// m01 m11 m21 m31 * y = newy = m01*x+m11*y+m21*z+m31*1
	// m02 m12 m22 m32   z   newz   m01*x+m11*y+m21*z+m31*1
	//   0   0   0   1   1 =    1     0*x+  0*y+  0*z+  1*1

	vector3f Res;
	Res.V[0] = Vec.V[0]*MM[0][0] + Vec.V[1]*MM[1][0] + Vec.V[2]*MM[2][0] + MM[3][0];
	Res.V[1] = Vec.V[0]*MM[0][1] + Vec.V[1]*MM[1][1] + Vec.V[2]*MM[2][1] + MM[3][1];
	Res.V[2] = Vec.V[0]*MM[0][2] + Vec.V[1]*MM[1][2] + Vec.V[2]*MM[2][2] + MM[3][2];
	return Res;
}

matrix4f& matrix4f::operator *= (const matrix4f& Mat)
{
	// m00 m10 m20 m30   n00 n10 n20 n30
	// m01 m11 m21 m31 * n01 n11 n21 n31
	// m02 m12 m22 m32   n02 n12 n22 n32
	// m03 m13 m23 m33   n03 n13 n23 n33

	// for example calculate newxy (0=x, 1=y)
	//                   nx0    
	// m0y m1y m2y m3y * nx1    = m0y*nx0+m1y*nx1+m2y*nx2+m3y*nx3 = new01
	//                   nx2    
	//                   nx3    

#ifndef COMPILE_SSE2
	matrix4f OrigMat=*this;
	// Matrix multiplication
	// Based on http://www.math.csusb.edu/math110/src/matrices/basics.html
	for(int y=0; y<4; y++)
	{
		for(int x=0; x<4; x++)
		{
			MM[x][y] = OrigMat.MM[0][y] * Mat.MM[x][0] + 
						   OrigMat.MM[1][y] * Mat.MM[x][1] + 
						   OrigMat.MM[2][y] * Mat.MM[x][2] + 
						   OrigMat.MM[3][y] * Mat.MM[x][3];
		}
	}
#else
	// SSE version
	
	__m128 xr[4];
	__m128 yr[4];
	
	yr[0] = _mm_set_ps(M[0], M[4], M[8], M[12]);
	yr[1] = _mm_set_ps(M[1], M[5], M[9], M[13]);
	yr[2] = _mm_set_ps(M[2], M[6], M[10], M[14]);
	yr[3] = _mm_set_ps(M[3], M[7], M[11], M[15]);

	xr[0] = _mm_set_ps(Mat.M[0], Mat.M[1], Mat.M[2], Mat.M[3]);
	xr[1] = _mm_set_ps(Mat.M[4], Mat.M[5], Mat.M[6], Mat.M[7]);
	xr[2] = _mm_set_ps(Mat.M[8], Mat.M[9], Mat.M[10], Mat.M[11]);
	xr[3] = _mm_set_ps(Mat.M[12], Mat.M[13], Mat.M[14], Mat.M[15]);

	__m128 add4;
#define DO_MATRIX_ROW(x) \
	add4 = _mm_sum4_ps(_mm_mul_ps(yr[0], xr[x]), _mm_mul_ps(yr[1], xr[x]), _mm_mul_ps(yr[2], xr[x]), _mm_mul_ps(yr[3], xr[x])); \
	_mm_storeu_ps(MatrixFlat+x*4, add4)

	DO_MATRIX_ROW(0);
	DO_MATRIX_ROW(1);
	DO_MATRIX_ROW(2);
	DO_MATRIX_ROW(3);
#undef DO_MATRIX_ROW
#endif

	return *this;
}

matrix4f& matrix4f::operator^=( const matrix4f& Mat )
{
#ifndef COMPILE_SSE2
	matrix4f OrigMat=*this;
	for(int y=0; y<4; y++)
	{
		for(int x=0; x<4; x++)
		{
			MM[x][y] = Mat.MM[0][y] * OrigMat.MM[x][0] + 
						   Mat.MM[1][y] * OrigMat.MM[x][1] + 
						   Mat.MM[2][y] * OrigMat.MM[x][2] + 
						   Mat.MM[3][y] * OrigMat.MM[x][3];

		}
	}

#else
	// SSE version

	__m128 xr[4];
	__m128 yr[4];

	yr[0] = _mm_set_ps(Mat.M[0], Mat.M[4], Mat.M[8], Mat.M[12]);
	yr[1] = _mm_set_ps(Mat.M[1], Mat.M[5], Mat.M[9], Mat.M[13]);
	yr[2] = _mm_set_ps(Mat.M[2], Mat.M[6], Mat.M[10], Mat.M[14]);
	yr[3] = _mm_set_ps(Mat.M[3], Mat.M[7], Mat.M[11], Mat.M[15]);

	xr[0] = _mm_set_ps(M[0], M[1], M[2], M[3]);
	xr[1] = _mm_set_ps(M[4], M[5], M[6], M[7]);
	xr[2] = _mm_set_ps(M[8], M[9], M[10], M[11]);
	xr[3] = _mm_set_ps(M[12], M[13], M[14], M[15]);

	__m128 add4;
#define DO_MATRIX_ROW(x) \
	add4 = _mm_sum4_ps(_mm_mul_ps(yr[0], xr[x]), _mm_mul_ps(yr[1], xr[x]), _mm_mul_ps(yr[2], xr[x]), _mm_mul_ps(yr[3], xr[x])); \
	_mm_storeu_ps(MatrixFlat+x*4, add4)

	DO_MATRIX_ROW(0);
	DO_MATRIX_ROW(1);
	DO_MATRIX_ROW(2);
	DO_MATRIX_ROW(3);
#undef DO_MATRIX_ROW
#endif

	return (*this);
}
matrix4f& matrix4f::operator -= (const matrix4f& Mat)
{
	for(int y=0; y<4; y++)
	{
		for(int x=0; x<4; x++)
		{
			MM[x][y] = MM[x][y] - Mat.MM[x][y];
		}
	}

	return *this;
}

matrix4f& matrix4f::operator += (const matrix4f& Mat)
{
	for(int y=0; y<4; y++)
	{
		for(int x=0; x<4; x++)
		{
			MM[x][y] = MM[x][y] + Mat.MM[x][y];
		}
	}

	return *this;
}

void matrix4f::Zero(float Value)
{
	if(Value==0.0f)
		// Fastest way to clear a matrix.
		memset(MM,0,sizeof(float)*16);
	else
	{
		for(int y=0; y<4; y++)
		{
			for(int x=0; x<4; x++)
			{
				memcpy(&MM[x][y], &Value, sizeof(float));
			}
		}
	}
}

void matrix4f::Identity( float scale/*=1.0f*/ )
{
	Zero();
	MM[0][0] = MM[1][1] = MM[2][2] = scale;
	MM[3][3] = 1.0f;
}

void matrix4f::Transpose()
{
	// version 2, a lot faster by eliminating deep copy, and more than half of the swaps. (16 copys, and 16 sets first, now just 6 swaps..)
	float swap;
	
	for(int y=0; y<3; y++)
	{
		for(int x=y+1; x<4; x++)
		{
			// swap column to row and vice versa.
			swap = MM[x][y];
			MM[x][y] = MM[y][x];
			MM[y][x] = swap;
		}
	}
}

vector3f matrix4f::TransformVector4D(const vector3f& xyz, float& w) const
{
	float origw = w;
	vector3f Res;
	Res.V[0] = xyz.V[0]*MM[0][0] + xyz.V[1]*MM[1][0] + xyz.V[2]*MM[2][0] + origw*MM[3][0];
	Res.V[1] = xyz.V[0]*MM[0][1] + xyz.V[1]*MM[1][1] + xyz.V[2]*MM[2][1] + origw*MM[3][1];
	Res.V[2] = xyz.V[0]*MM[0][2] + xyz.V[1]*MM[1][2] + xyz.V[2]*MM[2][2] + origw*MM[3][2];
	w =		   xyz.V[0]*MM[0][3] + xyz.V[1]*MM[1][3] + xyz.V[2]*MM[2][3] + origw*MM[3][3];

	return Res;
}

vector3f matrix4f::TransformVectorPerspectiveDivide( const vector3f& xyz ) const
{
	vector3f Res;
	Res.V[0] = xyz.V[0]*MM[0][0] + xyz.V[1]*MM[1][0] + xyz.V[2]*MM[2][0] + MM[3][0];
	Res.V[1] = xyz.V[0]*MM[0][1] + xyz.V[1]*MM[1][1] + xyz.V[2]*MM[2][1] + MM[3][1];
	Res.V[2] = xyz.V[0]*MM[0][2] + xyz.V[1]*MM[1][2] + xyz.V[2]*MM[2][2] + MM[3][2];
	float w =  xyz.V[0]*MM[0][3] + xyz.V[1]*MM[1][3] + xyz.V[2]*MM[2][3] + MM[3][3];
	//w = fabsf(w);
	Res.V[0] /= w;
	Res.V[1] /= w;
	Res.V[2] /= w;

	return Res;
}

// row major
int invertTable[16][9] = {{5,6,7,9,10,11,13,14,15},
{4,6,7,8,10,11,12,14,15},
{4,5,7,8,9,11,12,13,15},
{4,5,6,8,9,10,12,13,14},
{1,2,3,9,10,11,13,14,15},
{0,2,3,8,10,11,12,14,15},
{0,1,3,8,9,11,12,13,15},
{0,1,2,8,9,10,12,13,14},
{1,2,3,5,6,7,13,14,15},
{0,2,3,4,6,7,12,14,15},
{0,1,3,4,5,7,12,13,15},
{0,1,2,4,5,6,12,13,14},
{1,2,3,5,6,7,9,10,11},
{0,2,3,4,6,7,8,10,11},
{0,1,3,4,5,7,8,9,11},
{0,1,2,4,5,6,8,9,10}};

float flipTable[16] = { 1.0f,-1.0f,1.0f,-1.0f,
                       -1.0f,1.0f,-1.0f,1.0f,
					    1.0f,-1.0f,1.0f,-1.0f,
					   -1.0f,1.0f,-1.0f,1.0f };

// column major
/*int invertTable[16][9] = {
	{5,9,13,6,10,14,7,11,15},
	{1,9,13,2,10,14,3,11,15},
	{1,5,13,2,6,14,3,7,15},
	{1,5,9,2,6,10,3,7,11},
	{4,8,12,6,10,14,7,11,15},
	{0,8,12,2,10,14,3,11,15},
	{0,4,12,2,6,14,3,7,15},
	{0,4,8,2,6,10,3,7,11},
	{4,8,12,5,9,13,7,11,15},
	{0,8,12,1,9,13,3,11,15},
	{0,4,12,1,5,13,3,7,15},
	{0,4,8,1,5,9,3,7,11},
	{4,8,12,5,9,13,6,10,14},
	{0,8,12,1,9,13,2,10,14},
	{0,4,12,1,5,13,2,6,14},
	{0,4,8,1,5,9,2,6,10}};*/

bool matrix4f::Inverse()
{
	float det = Determinant();
	if(det == 0.0f) // When the determinant is 0, there is no inverse.
		return false;

	static matrix4f Mat;
	Mat = *this;

	//float m3x3[9];
	int idx=0;
	for(int yy=0; yy<4; yy++)
	{
		for(int xx=0; xx<4; xx++)
		{
			
			MM[xx][yy] = (matrix4f::Determinant3x3(Mat.M[invertTable[idx][0]],
													  Mat.M[invertTable[idx][1]],
													  Mat.M[invertTable[idx][2]],
													  Mat.M[invertTable[idx][3]],
													  Mat.M[invertTable[idx][4]],
													  Mat.M[invertTable[idx][5]],
													  Mat.M[invertTable[idx][6]],
													  Mat.M[invertTable[idx][7]],
													  Mat.M[invertTable[idx][8]]) / det) * flipTable[idx]; // (((xx+yy)%2)==0?1.0f:-1.0f);

			idx++;

		}
	}
	//this->Transpose();
	return true;
}


void matrix4f::MultiplyValues(matrix4f& Mat)
{
	for(int y=0; y<4; y++)
	{
		for(int x=0; x<4; x++)
		{
			MM[x][y] *= Mat.MM[y][x];
		}
	}
}

void matrix4f::Translate3(const vector3f& Translate)
{
	// m00 m10 m20 m30
	// m01 m11 m21 m31
	// m02 m12 m22 m32
	// m03 m13 m23 m33

	matrix4f multiply;
	multiply.MM[3][0] = Translate.V[0];
	multiply.MM[3][1] = Translate.V[1];
	multiply.MM[3][2] = Translate.V[2];

	*this *= multiply;
}


void matrix4f::Scale3(const vector3f& Scale)
{
	// m00 m10 m20 m30
	// m01 m11 m21 m31
	// m02 m12 m22 m32
	// m03 m13 m23 m33

	matrix4f multiply;
	multiply.MM[0][0] = Scale.V[0];
	multiply.MM[1][1] = Scale.V[1];
	multiply.MM[2][2] = Scale.V[2];

	*this *= multiply;
}

void matrix4f::RotateX(float Theta)
{
	// m00 m10 m20 m30
	// m01 m11 m21 m31
	// m02 m12 m22 m32
	// m03 m13 m23 m33

	float sTheta = sinf(Theta);
	float cTheta = cosf(Theta);

	matrix4f multiply;
	multiply.MM[1][1] = cTheta;
	multiply.MM[2][2] = cTheta;
	multiply.MM[1][2] = sTheta;
	multiply.MM[2][1] = -sTheta;

	*this *= multiply;
}

void matrix4f::RotateY(float Theta)
{
	// m00 m10 m20 m30
	// m01 m11 m21 m31
	// m02 m12 m22 m32
	// m03 m13 m23 m33

	float sTheta = sinf(Theta);
	float cTheta = cosf(Theta);

	matrix4f multiply;
	multiply.MM[0][0] = cTheta;
	multiply.MM[2][2] = cTheta;
	multiply.MM[2][0] = sTheta;
	multiply.MM[0][2] = -sTheta;

	*this *= multiply;
}

void matrix4f::RotateZ(float Theta)
{
	// m00 m10 m20 m30
	// m01 m11 m21 m31
	// m02 m12 m22 m32
	// m03 m13 m23 m33

	// c s
	// -s c

	float sTheta = sinf(Theta);
	float cTheta = cosf(Theta);

	matrix4f multiply;
	multiply.MM[0][0] = cTheta;
	multiply.MM[1][1] = cTheta;
	multiply.MM[1][0] = sTheta;
	multiply.MM[0][1] = -sTheta;

	*this *= multiply;
}

void matrix4f::RotateEuler(float RotX, float RotY, float RotZ)
{
	// thanks to http://web.archive.org/web/20041029003853/http:/www.j3d.org/matrix_faq/matrfaq_latest.html#Q33

	// Think it as M = ZRotM * YRotM * XRotM (try it, it gives the same result, this is only faster...)

	matrix4f mat;
	float A,B,C,D,E,F,AD,BD;

    A       = cosf(RotX);
    B       = sinf(RotX);
    C       = cosf(RotY);
    D       = sinf(RotY);
    E       = cosf(RotZ);
    F       = sinf(RotZ);
    AD      =   A * D;
    BD      =   B * D;
	mat.MM[0][0]  =   C * E;
    mat.MM[1][0]  =  -C * F;
    mat.MM[2][0]  =   D;
    mat.MM[0][1]  =  BD * E + A * F;
    mat.MM[1][1]  = -BD * F + A * E;
    mat.MM[2][1]  =  -B * C;
    mat.MM[0][2]  = -AD * E + B * F;
    mat.MM[1][2]  =  AD * F + B * E;
    mat.MM[2][2] =   A * C;

	*this *= mat;
}

void matrix4f::RotateAxisAngle(const vector3f& Axis, float angle)
{
	matrix4f mulw;
	float c=cosf(angle), s=sinf(angle), t=1-c;
	
	// first column
	mulw.MM[0][0] = t*Axis.x*Axis.x + c;
	mulw.MM[0][1] = t*Axis.x*Axis.y + s*Axis.z;
	mulw.MM[0][2] = t*Axis.x*Axis.z - s*Axis.y;

	// second column
	mulw.MM[1][0] = t*Axis.x*Axis.y - s*Axis.z;
	mulw.MM[1][1] = t*Axis.y*Axis.y + c;
	mulw.MM[1][2] = t*Axis.y*Axis.z + s*Axis.x;

	// third column
	mulw.MM[2][0] = t*Axis.x*Axis.z + s*Axis.y;
	mulw.MM[2][1] = t*Axis.y*Axis.z - s*Axis.x;
	mulw.MM[2][2] = t*Axis.z*Axis.z + c;

	(*this) *= mulw;
}

vector3f matrix4f::RotationEuler() const
{
	// we use quaternion math to obtain the matrix rotation in euler coordinates. If you might know a native function, or wish to resolve the math to optimize this, be my guest...
	vector3f ret;
	quatf q;
	matrix4f m=Rotation();
	m.Normalized();
	q.FromMatrix(m);
	q.NormalizeQuat();
	q.QuatToEuler(ret.x, ret.y, ret.z);
	return ret;
}
float matrix4f::Determinant()
{
	// m00 m10 m20 m30
	// m01 m11 m21 m31
	// m02 m12 m22 m32
	// m03 m13 m23 m33
	// Based on http://mcraeclan.com/MathHelp/MatrixDeterminant.htm

    return 
	 MM[0][0]*MM[1][1]*MM[2][2]*MM[3][3]
    -MM[0][0]*MM[1][1]*MM[2][3]*MM[3][2]
    +MM[0][0]*MM[1][2]*MM[2][3]*MM[3][1]
    -MM[0][0]*MM[1][2]*MM[2][1]*MM[3][3]
    +MM[0][0]*MM[1][3]*MM[2][1]*MM[3][2]
    -MM[0][0]*MM[1][3]*MM[2][2]*MM[3][1]
    -MM[0][1]*MM[1][2]*MM[2][3]*MM[3][0]
    +MM[0][1]*MM[1][2]*MM[2][0]*MM[3][3]
    -MM[0][1]*MM[1][3]*MM[2][0]*MM[3][2]
    +MM[0][1]*MM[1][3]*MM[2][2]*MM[3][0]
    -MM[0][1]*MM[1][0]*MM[2][2]*MM[3][3]
    +MM[0][1]*MM[1][0]*MM[2][3]*MM[3][2]
    +MM[0][2]*MM[1][3]*MM[2][0]*MM[3][1]
    -MM[0][2]*MM[1][3]*MM[2][1]*MM[3][0]
    +MM[0][2]*MM[1][0]*MM[2][1]*MM[3][3]
    -MM[0][2]*MM[1][0]*MM[2][3]*MM[3][1]
    +MM[0][2]*MM[1][1]*MM[2][3]*MM[3][0]
    -MM[0][2]*MM[1][1]*MM[2][0]*MM[3][3]
    -MM[0][3]*MM[1][0]*MM[2][1]*MM[3][2]
    +MM[0][3]*MM[1][0]*MM[2][2]*MM[3][1]
    -MM[0][3]*MM[1][1]*MM[2][2]*MM[3][0]
    +MM[0][3]*MM[1][1]*MM[2][0]*MM[3][2]
    -MM[0][3]*MM[1][2]*MM[2][0]*MM[3][1]
    +MM[0][3]*MM[1][2]*MM[2][1]*MM[3][0]; // Likey likey? No? Too bad for you! :D
}

void matrix4f::Normalize()
{
	vector3f norm;
	
	//norm.Set(Matrix[0][1], Matrix[1][1], Matrix[2][1]); norm.Normalize(); Matrix[0][1] = norm.x; Matrix[1][1] = norm.y; Matrix[2][1] = norm.z;  // row 1.
	//norm.Set(Matrix[0][2], Matrix[1][2], Matrix[2][2]); norm.Normalize(); Matrix[0][2] = norm.x; Matrix[1][2] = norm.y; Matrix[2][2] = norm.z;  // row 2.
	norm.Set(MM[0][0], MM[1][0], MM[2][0]); norm.Normalize(); MM[0][0] = norm.x; MM[1][0] = norm.y; MM[2][0] = norm.z;  // row 0.
	norm.Set(MM[0][1], MM[1][1], MM[2][1]); norm.Normalize(); MM[0][1] = norm.x; MM[1][1] = norm.y; MM[2][1] = norm.z;  // row 1.
	norm.Set(MM[0][2], MM[1][2], MM[2][2]); norm.Normalize(); MM[0][2] = norm.x; MM[1][2] = norm.y; MM[2][2] = norm.z;  // row 2.

	vector3f newz, newy;
	newz.SetToCross(ViewLocalX(), ViewLocalY());
	MM[0][2] = newz.x;
	MM[1][2] = newz.y;
	MM[2][2] = newz.z;
	newy.SetToCross(ViewLocalZ(), ViewLocalX());
	MM[0][1] = newy.x;
	MM[1][1] = newy.y;
	MM[2][1] = newy.z;
}

void matrix4f::SetViewDirection( const vector3f& Position, const vector3f& Direction, const vector3f& Up)
{
	vector3f f = Direction;
	f.Normalize();

	vector3f s = f.Cross(Up);
	vector3f u = s.Cross(f);
	s.Normalize();
	u.Normalize();

	matrix4f& viewMatrix = (*this);
	viewMatrix.SetTranslation(0.0f, 0.0f, 0.0f);
	viewMatrix.MM[0][0] = s.V[0];
	viewMatrix.MM[1][0] = s.V[1];
	viewMatrix.MM[2][0] = s.V[2];

	viewMatrix.MM[0][1] = u.V[0];
	viewMatrix.MM[1][1] = u.V[1];
	viewMatrix.MM[2][1] = u.V[2];

	viewMatrix.MM[0][2] = -f.V[0];
	viewMatrix.MM[1][2] = -f.V[1];
	viewMatrix.MM[2][2] = -f.V[2];

	viewMatrix.Translate3(Position * -1.0f);
	
}

matrix4f matrix4f::ProjectionOrtho(float Width, float Height)
{
	matrix4f projection(false);
	int r=0;
	float Near=-1000.0f, Far=1000.0f;

#define _M(x,y) projection.MM[x][y]
	_M(0,r)=2.0f/Width;	_M(1,r)=0.0f;			_M(2,r)=0.0f;				_M(3,r)=-1.0f; ++r;
	_M(0,r)=0.0f;		_M(1,r)=2.0f/-Height;	_M(2,r)=0.0f;				_M(3,r)=1.0f; ++r;
//  _M(0,r)=0.0f;		_M(1,r)=2.0f/Height;	_M(2,r)=0.0f;				_M(3,r)=-1.0f; ++r; // opengl style coords (0,0 left bottom)
	_M(0,r)=0.0f;		_M(1,r)=0.0f;			_M(2,r)=((Height > 0.0f)?-2.0f:2.0f)/(Far-Near);	_M(3,r)=(Far+Near)/(Far-Near); ++r;
	_M(0,r)=0.0f;		_M(1,r)=0.0f;			_M(2,r)=0.0f;				_M(3,r)=1.0f;
#undef _M

	return projection;
}

matrix4f matrix4f::Projection(float FieldOfViewYDegrees, float AspectRatio, float Near, float Far)
{
	float fovradians = DEG2RAD(FieldOfViewYDegrees);
	float f = 1.0f / tanf(fovradians / 2.0f);

	matrix4f projection(0.0f);

	projection.MM[0][0] = f / AspectRatio;
	projection.MM[1][1] = f;
	projection.MM[2][2] = (Far + Near) / (Near - Far);
	projection.MM[2][3] = -1;
	projection.MM[3][2] = (2 * Far * Near) / (Near - Far); // translation z

	return projection;
}

matrix4f matrix4f::View( const vector3f& Eye, const vector3f& LookAt, const vector3f& Up )
{
	//Up.Normalize();

	vector3f f = LookAt - Eye;
	f.Normalize();

	vector3f s = f.Cross(Up); 
	vector3f u = s.Cross(f); 
	s.Normalize();
	u.Normalize();

	matrix4f viewMatrix;
	viewMatrix.MM[0][0] = s.V[0];
	viewMatrix.MM[1][0] = s.V[1];
	viewMatrix.MM[2][0] = s.V[2];

	viewMatrix.MM[0][1] = u.V[0];
	viewMatrix.MM[1][1] = u.V[1];
	viewMatrix.MM[2][1] = u.V[2];

	viewMatrix.MM[0][2] = -f.V[0];
	viewMatrix.MM[1][2] = -f.V[1];
	viewMatrix.MM[2][2] = -f.V[2];

	viewMatrix.Translate3(Eye * -1.0f);

	return viewMatrix;
}

void matrix4f::TransformVectorFloatArray( float* arr ) const
{
	float src[3] = { arr[0], arr[1], arr[2] };
	arr[0] = src[0]*MM[0][0] + src[1]*MM[1][0] + src[2]*MM[2][0] + MM[3][0];
	arr[1] = src[0]*MM[0][1] + src[1]*MM[1][1] + src[2]*MM[2][1] + MM[3][1];
	arr[2] = src[0]*MM[0][2] + src[1]*MM[1][2] + src[2]*MM[2][2] + MM[3][2];

}

matrix4f matrix4f::TransformIntoCameraMatrix( const matrix4f& m )
{
	static bool cached=false;
	static matrix4f xForwardToCamera;
	if(cached == false)
	{
		matrix4f cameraToXforward;
		cameraToXforward.RotateY(PI*0.5f);
		cameraToXforward.RotateX(-PI*0.5f);
		xForwardToCamera = cameraToXforward.Inverted();
		cached = true;
	}

	matrix4f view = m * xForwardToCamera;
	view.Inverse();
	return view;
}

void matrix4f::LerpWith( const matrix4f& b, float fac )
{
	for(int i=0; i<15; i++)
	{
		M[i] = M[i] + (b.M[i] - M[i]) * fac;
	}
}

void matrix4f::SetTo(const float* m)
{
	memcpy(M, m, sizeof(M[0]) * 16);
}


// Quaternations Class
// Original from (c) by Heinrich Tillack 2002 (http://a128.x15.org)
// based on code by  by Alan Baylis 2001 and DigiBen
// modified by Seniltai for use in the TRoS Engine.

quatf::~quatf()
{
}




quatf quatf::Slerp(const quatf& q1, quatf q2, float k)
{
/*	quatf r(q1*k), r2(q2*(1-k));
	r += r2;
	r.NormalizeQuat();
	return r;*/
	// v0 and v1 should be unit length or else
	// something broken will happen.

	// Compute the cosine of the angle between the two vectors.
	float dot = q1.Dot(q2);

	if(dot < 0.0f )
	{
		q2.NegateQuat(); // make sure it never goes the long way around
		dot = q1.Dot(q2); // recompute dot product
	}

	const float DOT_THRESHOLD = 0.9995f;
	if (dot > DOT_THRESHOLD) {
		// If the inputs are too close for comfort, linearly interpolate
		// and normalize the result.

		quatf res = q1 + (q2 - q1) * k;
		res.NormalizeQuat();
		return res;
	}

	dot = CLAMP(dot, -1.0f, 1.0f);           // Robustness: Stay within domain of acos()
	float theta_0 = acosf(dot);  // theta_0 = angle between input vectors
	float theta = theta_0*k;    // theta = angle between v0 and result 

	quatf v2 = q2 - (q1*dot);
	v2.NormalizeQuat();              // { v0, v2 } is now an orthonormal basis

	return q1*cos(theta) + v2*sin(theta);
}

quatf quatf::Nlerp(const quatf& q1, const quatf& q2, float k)
{
	/*quatf r(q1*k), r2(q2*(1-k));
	r += r;*/
	// thanks a bunch to this forum page over here:
	// http://www.gamedev.net/community/forums/topic.asp?topic_id=575660
	// fixes "gimbal" in quaternions
	// basically says that when the dot product is negative, negate one of the quaternions to make the dot product positive again (and therefore be the short path)
	quatf aq2(q2);
	if(q1.Dot(q2) < 0.0f )
		aq2.NegateQuat();


	/*quatf r;
	r = q1 + (aq2-q1) * k;
	r.NormalizeQuat();
	return r;*/

	aq2 -= q1;
	aq2 *= k;
	aq2 += q1;
	aq2.NormalizeQuat();
	return aq2;
}

void quatf::Reset()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 1.0f;
}


quatf& quatf::operator = (quatf Qt)
{
    x = Qt.x;
    y = Qt.y;
    z = Qt.z;
    w = Qt.w;
	return (*this);
}

quatf& quatf::operator *=(const quatf& q)
{
	// Correct algorithm! Adapted from http://www.cprogramming.com/tutorial/3d/quaternions.html

	//(Q1 * Q2).x = (w1x2 + x1w2 + y1z2 - z1y2)
	//(Q1 * Q2).y = (w1y2 - x1z2 + y1w2 + z1x2)
	//(Q1 * Q2).z = (w1z2 + x1y2 - y1x2 + z1w2)
	//(Q1 * Q2).w = (w1w2 - x1x2 - y1y2 - z1z2)

	quatf newQ(
		(w * q.x + x * q.w + y * q.z - z * q.y),  // x
		(w * q.y - x * q.z + y * q.w + z * q.x),  // y
		(w * q.z + x * q.y - y * q.x + z * q.w),  // z
		(w * q.w - x * q.x - y * q.y - z * q.z)); // w

	x = newQ.x;
	y = newQ.y;
	z = newQ.z;
	w = newQ.w;


	return (*this);
}


quatf& quatf::operator /=(const quatf& r)
{
	quatf &q = *this;
	float optim=(r.x*r.x)+(r.y*r.y)+(r.z*r.z)+(r.w*r.w);
	w = (r.w*q.w + r.x*q.x + r.y*q.y + r.z*q.x)/optim;
	x = (r.w*q.x - r.x*q.w - r.y*q.z + r.z*q.y)/optim;
	y = (r.w*q.y + r.x*q.z - r.y*q.w - r.z*q.z)/optim;
	z = (r.w*q.z - r.x*q.y + r.y*q.x - r.z*q.w)/optim;

	return q;
}



////////////////////////////// CREATE MATRIX \\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function converts a quaternion to a rotation matrix
/////
////////////////////////////// CREATE MATRIX \\\\\\\\\\\\\\\\\\\\\\\\\\\\\*


void quatf::ToMatrix( matrix4f& M ) const
{
	float xx = x*x;
	float yy = y*y;
	float zz = z*z;
	float xy = x*y;
	float zw = z*w;
	float xz = x*z;
	float yw = y*w;
	float xw = x*w;
	float yz = y*z;

	// First row
	M.MM[ 0][ 0] = 1.0f - 2.0f * (yy + zz );
	M.MM[ 0][ 1] = 2.0f * ( xy - zw );
	M.MM[ 0][ 2] = 2.0f * ( xz + yw );

	// Second row
	M.MM[ 1][ 0] = 2.0f * (xy + zw );
	M.MM[ 1][ 1] = 1.0f - 2.0f * ( xx + zz );
	M.MM[ 1][ 2] = 2.0f * ( yz - xw );

	// Third row
	M.MM[ 2][ 0] = 2.0f * ( xz - yw );
	M.MM[ 2][ 1] = 2.0f * ( yz + xw );
	M.MM[ 2][ 2] = 1.0f - 2.0f * ( xx + yy );
}




matrix4f quatf::Matrix() const
{
	matrix4f M;
	ToMatrix(M);
    return M;
}

#ifndef _WIN32
#define _copysign copysign
#endif

void quatf::FromMatrixSecondary(const matrix4f& M)
{
	w = sqrtf(MAX(0.0f, 1.0f + M.MM[0][0] + M.MM[1][1] + M.MM[2][2])) / 2.0f;
	x = sqrtf(MAX(0.0f, 1.0f + M.MM[0][0] - M.MM[1][1] - M.MM[2][2])) / 2.0f;
	y = sqrtf(MAX(0.0f, 1.0f - M.MM[0][0] + M.MM[1][1] - M.MM[2][2])) / 2.0f;
	z = sqrtf(MAX(0.0f, 1.0f - M.MM[0][0] - M.MM[1][1] + M.MM[2][2])) / 2.0f;
	x = (float)_copysign( x, M.MM[2][1] - M.MM[1][2] );
	y = (float)_copysign( y, M.MM[0][2] - M.MM[2][0] );
	z = (float)_copysign( z, M.MM[1][0] - M.MM[0][1] );
}

void quatf::FromMatrix(const matrix4f &M)
{	
	float t = 1.0f + M.MM[0][0] + M.MM[1][1] + M.MM[2][2];

	// large enough
	if( t > 0.001f)
	{
		float s = sqrtf( t) * 2.0f;
		x = (M.MM[2][1] - M.MM[1][2]) / s;
		y = (M.MM[0][2] - M.MM[2][0]) / s;
		z = (M.MM[1][0] - M.MM[0][1]) / s;
		w = 0.25f * s;
	} // else we have to check several cases
	else if( M.MM[0][0] > M.MM[1][1] && M.MM[0][0] > M.MM[2][2] )  
	{	
		// Column 0: 
		float s = sqrtf( 1.0f + M.MM[0][0] - M.MM[1][1] - M.MM[2][2]) * 2.0f;
		x = 0.25f * s;
		y = (M.MM[1][0] + M.MM[0][1]) / s;
		z = (M.MM[0][2] + M.MM[2][0]) / s;
		w = (M.MM[2][1] - M.MM[1][2]) / s;
	} 
	else if( M.MM[1][1] > M.MM[2][2]) 
	{ 
		// Column 1: 
		float s = sqrtf( 1.0f + M.MM[1][1] - M.MM[0][0] - M.MM[2][2]) * 2.0f;
		x = (M.MM[1][0] + M.MM[0][1]) / s;
		y = 0.25f * s;
		z = (M.MM[2][1] + M.MM[1][2]) / s;
		w = (M.MM[0][2] - M.MM[2][0]) / s;
	} else 
	{ 
		// Column 2:
		float s = sqrtf( 1.0f + M.MM[2][2] - M.MM[0][0] - M.MM[1][1]) * 2.0f;
		x = (M.MM[0][2] + M.MM[2][0]) / s;
		y = (M.MM[2][1] + M.MM[1][2]) / s;
		z = 0.25f * s;
		w = (M.MM[1][0] - M.MM[0][1]) / s;
	}

	
}



bool quatf::QuatToAxisAngle(vector3f& v, float &angle)
{
	float	temp_angle;		// temp angle
	float	scale;			// temp vars

	temp_angle = acosf(w);

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Another version where scale is sqrt (x2 + y2 + z2)
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	scale = sqrtf(x*x+y*y+z*z); // couldn't use MagnitudeQuat() because it also takes w in the computation.
//	scale = (float)sin(temp_angle);

	if (scale == 0.0f)		// angle is 0 or 360 so just simply set axis to 0,0,1 with angle 0
	{
		angle = 0.0f;

		v.Set(0.0f, 0.0f, 1.0f);		// any axis will do
	}
	else
	{
		angle = (float)(temp_angle * 2.0);		// angle in radians

		v.Set(x / scale, y / scale, z / scale);
		v.Normalize();
	}

	return true;
}	// end void GetAxisAngle(..)


void quatf::AxisAngleToQuat(const vector3f& axis, float theta)
{
    float halfTheta = theta * 0.5f;
    float sinHalfTheta = sinf(halfTheta);
    x = axis.x * sinHalfTheta;
    y = axis.y * sinHalfTheta;
    z = axis.z * sinHalfTheta;
    w = cosf(halfTheta);
}

void quatf::EulerToQuat(float roll, float pitch, float yaw)
{
    float cr, cp, cy, sr, sp, sy, cpcy, spsy;  // calculate trig identities
    cr = cosf(roll/2);
    cp = cosf(pitch/2);
    cy = cosf(yaw/2);
    sr = sinf(roll/2);
    sp = sinf(pitch/2);
    sy = sinf(yaw/2);
    cpcy = cp * cy;
    spsy = sp * sy;
    
	// Code fix, all signs needed to be inverted because of angle alignment (now it has the same behaviour as matrices :D)

    x = sr * cpcy + cr * spsy;
    y = cr * sp * cy - sr * cp * sy;
    z = cr * cp * sy + sr * sp * cy;
	w = cr * cpcy - sr * spsy;
}

bool quatf::QuatToEuler(float &X, float &Y, float &Z) const
{
	// adapted from http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm

	//float test = x*y + z*w; // Original code - Inverted Z & Y
	float test = x*z + y*w;
	if (test > 0.4999) { // singularity at north pole
		X = 0.0f; // bank = roll = X
		Y = PI/2; // attitude = pitch = Y
		Z = 2.0f * atan2f(x,w); // heading = yaw = Z (up vector)

		return true;
	}
	if (test < -0.4999) { // singularity at south pole
		X = 0.0f;
		Y = -PI/2.0f;
		Z = -2.0f * atan2f(x,w);

		return true;
	}

	// // Old code - Inverted Z & Y
    //double sqx = x*x;
    //double sqy = y*y;
    //double sqz = z*z;
    //X = atan2f(2.0f*x*w-2.0f*y*z , 1.0f - 2.0f*sqx - 2.0f*sqz);
	//Y = asinf(2*test);
	//Z = atan2f(2*y*w-2*x*z , 1.0f - 2.0f*sqy - 2.0f*sqz);

    float sqx = x*x;
    float sqy = y*y;
    float sqz = z*z;
    X = atan2f(2.0f*x*w-2.0f*z*y , 1.0f - 2.0f*sqx - 2.0f*sqy);
	Y = asinf(2*test);
	Z = atan2f(2*z*w-2*x*y , 1.0f - 2.0f*sqz - 2.0f*sqy);

	return true;
}

void quatf::ShortestArcToQuat( const vector3f& From, const vector3f& To )
{
	float d = From.Dot(To);
	if(d < (1e-6f - 1.0f))
	{
		vector3f axis(0.0f, 0.0f, 1.0f);
	/*	axis = vector3f(1.0f, 0.0f, 0.0f).Cross(From);
		if (axis.Dot(axis) <= 0.0)
			 axis = vector3f(0.0f, 1.0f, 0.0f).Cross(From);
		axis.Normalize();*/

		AxisAngleToQuat(axis, PI);
	}
	else
	{
		// From Game Programming Gems pg. 217
		vector3f c = From.Cross(To);
		float s = sqrtf( ( 1.0f + d ) * 2.0f );

		x = c.x / s;
		y = c.y / s;
		z = c.z / s;
		w = s / 2.0f; 
		NormalizeQuat();
	}
}

void quatf::VectorToQuat( const vector3f& vdir, const vector3f& up/*=vector3f(0.0f, 0.0f, 1.0f)*/ )
{
	vector3f vDirection = vdir.Normalized();
	// Step 1. Setup basis vectors describing the rotation given the input vector and assuming an initial up direction of (0, 1, 0)
	vector3f vUp(up);    // Z Up vector
	vector3f vRight = vUp.Cross(vDirection);    // The perpendicular vector to Up and Direction
	vUp = vDirection.Cross(vRight);            // The actual up vector given the direction and the right vector

	float m11 = vDirection.x, m12 = vDirection.y, m13 = vDirection.z;
	float m21 = vRight.x,     m22 = vRight.y,     m23 = vRight.z;
	float m31 = vUp.x,        m32 = vUp.y,        m33 = vUp.z;

	// Step 3. Build a quaternion from the matrix
	w = sqrtf(1.0f + m11 + m22 + m33) / 2.0f;
	double dfWScale = w * 4.0;
	x = (float)((m32 - m23) / dfWScale);
	y = (float)((m13 - m31) / dfWScale);
	z = (float)((m21 - m12) / dfWScale);
}

void quatf::Mul( quatf& q )
{
	*this *= q;
}

void quatf::MulInto( quatf& q, quatf& outQ )
{
	outQ = *this;
	outQ *= q;
}

void quatf::MulVector( vector3f& v )
{
	v *= *this;
}

void quatf::MulVectorInto( vector3f& v, vector3f& into )
{
	into = v * *this;
}


CFC_API float AngularDiff(float a, float b)
{
	a = fmodf(a, 360.0f);
	b = fmodf(b, 360.0f);
	float dist[3] = {(b + 360.0f) - a, b - (a+ 360.0f), b-a };
	float distfabs[3] = {fabsf(dist[0]), fabsf(dist[1]), fabsf(dist[2]) };

	// return smallest (shortest distance)
	if(distfabs[0] < distfabs[1])
		if(distfabs[0] < distfabs[2]) return dist[0]; else return dist[2];
	else
		if(distfabs[1] < distfabs[2]) return dist[1]; else return dist[2];
}
CFC_API float AngularDiffRadians(float a, float b)
{
	float twopi = PI*2.0f;
	a = fmodf(a, twopi);
	b = fmodf(b, twopi);
	float dist[3] = {(b + twopi) - a, b - (a+ twopi), b-a };
	float distfabs[3] = {fabsf(dist[0]), fabsf(dist[1]), fabsf(dist[2]) };

	// return smallest (shortest distance)
	if(distfabs[0] < distfabs[1])
		if(distfabs[0] < distfabs[2]) return dist[0]; else return dist[2];
	else
		if(distfabs[1] < distfabs[2]) return dist[1]; else return dist[2];
}


void trsf::FromMatrix( const matrix4f& m )
{
	// code taken & adapted from Asset Importer (AssIMP)
	const matrix4f& _this = m;

	// extract translation
	Translation.x = _this.MM[3][0];
	Translation.y = _this.MM[3][1];
	Translation.z = _this.MM[3][2];

	// extract the rows of the matrix
	vector3f vRows[3];
	vRows[0] = vector3f(_this.MM[0][0],_this.MM[0][1],_this.MM[0][2]);
	vRows[1] = vector3f(_this.MM[1][0],_this.MM[1][1],_this.MM[1][2]);
	vRows[2] = vector3f(_this.MM[2][0],_this.MM[2][1],_this.MM[2][2]);


	// extract the scaling factors
	Scale.x = vRows[0].Length();
	Scale.y = vRows[1].Length();
	Scale.z = vRows[2].Length();

	// and remove all scaling from the matrix
	if(Scale.x)
	{
		vRows[0] /= Scale.x;
	}
	if(Scale.y)
	{
		vRows[1] /= Scale.y;
	}
	if(Scale.z)
	{
		vRows[2] /= Scale.z;
	}

	// build a 3x3 rotation matrix
	matrix4f mN;
	mN.MM[0][0] = vRows[0].x;
	mN.MM[0][1] = vRows[0].y;
	mN.MM[0][2] = vRows[0].z;
	mN.MM[1][0] = vRows[1].x;
	mN.MM[1][1] = vRows[1].y;
	mN.MM[1][2] = vRows[1].z;
	mN.MM[2][0] = vRows[2].x;
	mN.MM[2][1] = vRows[2].y;
	mN.MM[2][2] = vRows[2].z;

	// and generate the rotation quaternion from it
	Rotation.FromMatrix(mN);
}

trsf trsf::Interpolate( const trsf& f1, const trsf& f2, float factor, bool slerp/*=false*/ )
{
	trsf ret;
	ret.Translation = ((f2.Translation - f1.Translation) * factor) + f1.Translation;
	ret.Scale = ((f2.Scale - f1.Scale) * factor) + f1.Scale ;
	if(slerp == false)
		ret.Rotation = quatf::Nlerp(f1.Rotation, f2.Rotation, factor);
	else
		ret.Rotation = quatf::Slerp(f1.Rotation, f2.Rotation, factor);
	return ret;
}

void trsf::ToExistingMatrix( matrix4f& m ) const
{
	m.Translation(Translation);
	Rotation.ToMatrix(m);
	m.Scale3(Scale);
}

void trsf::Concatenate( const trsf& other )
{
	Translation += other.Translation * Rotation;
	Scale *= other.Scale;
	Rotation *= other.Rotation;
}

bool approximateVectorCompare::operator()( const vector3f& _Left, const vector3f& _Right ) const
{
	for(int i=0; i<3; i++)
	{
		float diff = fabsf(_Right.V[i]-_Left.V[i]);
		if(diff > 0.0001f)
			return _Left.V[i] < _Right.V[i];
	}	
	return false;
}

float cos(float ang)
{
	return ::cosf(ang);
}

float sin(float ang)
{
	return ::sinf(ang);
}

unsigned int colorf::ToRGBA32()
{
	float vrgba[4] = { CLAMP(rgba[0], 0.0f, 1.0f), CLAMP(rgba[1], 0.0f, 1.0f), CLAMP(rgba[2], 0.0f, 1.0f), CLAMP(rgba[3], 0.0f, 1.0f) };
	return	static_cast<unsigned int>(vrgba[0] * 255.0f) | 
			static_cast<unsigned int>(vrgba[1] * 255.0f) << 8 | 
			static_cast<unsigned int>(vrgba[2] * 255.0f) << 16 |
			static_cast<unsigned int>(vrgba[3] * 255.0f) << 24;
}

} // end namespace math
} // end namespace engine
