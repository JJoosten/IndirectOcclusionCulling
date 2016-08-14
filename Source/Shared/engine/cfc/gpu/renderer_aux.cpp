#include "renderer_aux.h"
#include <cfc/stl/stl_vector.hpp>
#include <cfc/math/math.h>

namespace cfc {

bool s_dbgFontInited = false;
unsigned int s_imgFontBuffer[128 * 5 * 5];
int s_Transl[256];

void s_initFont_setChar(int cidx, const char* a, const char* b, const char* c, const char* d, const char* e)
{
	int fontbufferw = 128 * 5;
	int dstx = cidx * 5;
	const char* sources[] = { a,b,c,d,e };
	for (int y = 0; y<5; y++)
	{
		const char* src = sources[y];
		for (int x = 0; x<5; x++)
		{
			int dstidx = (x + dstx) + (y*fontbufferw);
			if (src[x] == 'o')
				s_imgFontBuffer[dstidx] = 0xffffffff;
		}
	}
}

void s_initFont()
{
	if (s_dbgFontInited)
		return;

	s_dbgFontInited = true;

	memset(s_imgFontBuffer, 0, sizeof(s_imgFontBuffer));

	s_initFont_setChar(0, ":ooo:", "o:::o", "ooooo", "o:::o", "o:::o"); // a
	s_initFont_setChar(1, "oooo:", "o:::o", "oooo:", "o:::o", "oooo:"); // b
	s_initFont_setChar(2, ":oooo", "o::::", "o::::", "o::::", ":oooo"); // c
	s_initFont_setChar(3, "oooo:", "o:::o", "o:::o", "o:::o", "oooo:"); // d
	s_initFont_setChar(4, "ooooo", "o::::", "oooo:", "o::::", "ooooo"); // e
	s_initFont_setChar(5, "ooooo", "o::::", "ooo::", "o::::", "o::::"); // f
	s_initFont_setChar(6, ":oooo", "o::::", "o:ooo", "o:::o", ":ooo:"); // g
	s_initFont_setChar(7, "o:::o", "o:::o", "ooooo", "o:::o", "o:::o"); // h
	s_initFont_setChar(8, "::o::", ":::::", "::o::", "::o::", "::o::"); // i
	s_initFont_setChar(9, ":::o:", ":::o:", ":::o:", ":::o:", "ooo::"); // j
	s_initFont_setChar(10, "o::o:", "o:o::", "oo:::", "o:o::", "o::o:"); // k
	s_initFont_setChar(11, "o::::", "o::::", "o::::", "o::::", "ooooo"); // l
	s_initFont_setChar(12, "oo:o:", "o:o:o", "o:o:o", "o:::o", "o:::o"); // m
	s_initFont_setChar(13, "o:::o", "oo::o", "o:o:o", "o::oo", "o:::o"); // n
	s_initFont_setChar(14, ":ooo:", "o:::o", "o:::o", "o:::o", ":ooo:"); // o
	s_initFont_setChar(15, "oooo:", "o:::o", "oooo:", "o::::", "o::::"); // p
	s_initFont_setChar(16, ":ooo:", "o:::o", "o:::o", "o::oo", ":oooo"); // q
	s_initFont_setChar(17, "oooo:", "o:::o", "oooo:", "o:o::", "o::o:"); // r
	s_initFont_setChar(18, ":oooo", "o::::", ":ooo:", "::::o", "oooo:"); // s
	s_initFont_setChar(19, "ooooo", "::o::", "::o::", "::o::", "::o::"); // t
	s_initFont_setChar(20, "o:::o", "o:::o", "o:::o", "o:::o", ":oooo"); // u
	s_initFont_setChar(21, "o:::o", "o:::o", ":o:o:", ":o:o:", "::o::"); // v
	s_initFont_setChar(22, "o:::o", "o:::o", "o:o:o", "o:o:o", ":o:o:"); // w
	s_initFont_setChar(23, "o:::o", ":o:o:", "::o::", ":o:o:", "o:::o"); // x
	s_initFont_setChar(24, "o:::o", "o:::o", ":oooo", "::::o", ":ooo:"); // y
	s_initFont_setChar(25, "ooooo", ":::o:", "::o::", ":o:::", "ooooo"); // z
	s_initFont_setChar(26, ":ooo:", "o::oo", "o:o:o", "oo::o", ":ooo:"); // 0
	s_initFont_setChar(27, "::o::", ":oo::", "::o::", "::o::", ":ooo:"); // 1
	s_initFont_setChar(28, ":ooo:", "o:::o", "::oo:", ":o:::", "ooooo"); // 2
	s_initFont_setChar(29, "oooo:", "::::o", "::oo:", "::::o", "oooo:"); // 3
	s_initFont_setChar(30, "o::::", "o::o:", "ooooo", ":::o:", ":::o:"); // 4
	s_initFont_setChar(31, "ooooo", "o::::", "oooo:", "::::o", "oooo:"); // 5
	s_initFont_setChar(32, ":oooo", "o::::", "oooo:", "o:::o", ":ooo:"); // 6
	s_initFont_setChar(33, "ooooo", "::::o", ":::o:", "::o::", "::o::"); // 7
	s_initFont_setChar(34, ":ooo:", "o:::o", ":ooo:", "o:::o", ":ooo:"); // 8
	s_initFont_setChar(35, ":ooo:", "o:::o", ":oooo", "::::o", ":ooo:"); // 9
	s_initFont_setChar(36, "::o::", "::o::", "::o::", ":::::", "::o::"); // !
	s_initFont_setChar(37, ":ooo:", "::::o", ":::o:", ":::::", "::o::"); // ?
	s_initFont_setChar(38, ":::::", "::o::", ":::::", "::o::", ":::::"); // :
	s_initFont_setChar(39, ":::::", ":ooo:", ":::::", ":ooo:", ":::::"); // =
	s_initFont_setChar(40, ":::::", ":::::", ":::::", ":::o:", "::o::"); // ,
	s_initFont_setChar(41, ":::::", ":::::", ":::::", ":::::", "::o::"); // .
	s_initFont_setChar(42, ":::::", ":::::", ":ooo:", ":::::", ":::::"); // -
	s_initFont_setChar(43, ":::o:", "::o::", "::o::", "::o::", ":::o:"); // (
	s_initFont_setChar(44, "::o::", ":::o:", ":::o:", ":::o:", "::o::"); // )
	s_initFont_setChar(45, ":::::", ":::::", ":::::", ":::::", ":::::"); // <space>
	s_initFont_setChar(46, ":o:o:", "ooooo", ":o:o:", "ooooo", ":o:o:"); // #
	s_initFont_setChar(47, "::o::", "::o::", ":::::", ":::::", ":::::"); // '
	s_initFont_setChar(48, "o:o:o", ":ooo:", "ooooo", ":ooo:", "o:o:o"); // *
	s_initFont_setChar(49, "::::o", ":::o:", "::o::", ":o:::", "o::::"); // /
	s_initFont_setChar(50, "::o::", "::o::", "ooooo", "::o::", "::o::"); // +
	s_initFont_setChar(51, ":::::", ":::::", ":::::", ":::::", "ooooo"); // _
	s_initFont_setChar(52, "::oo:", "::o::", "::o::", "::o::", "::oo:"); // [
	s_initFont_setChar(53, "::oo:", ":::o:", ":::o:", ":::o:", "::oo:"); // ]
	s_initFont_setChar(54, ":o::o", ":::o:", "::o::", ":o:::", "o::o:"); // %
	s_initFont_setChar(55, "::o::", ":o:o:", "o:::o", ":::::", ":::::"); // ^
	s_initFont_setChar(56, ":::::", "::o::", ":::::", "::o::", ":o:::"); // ;
	s_initFont_setChar(57, ":::o:", "::o::", ":o:::", "::o::", ":::o:"); // <
	s_initFont_setChar(58, ":o:::", "::o::", ":::o:", "::o::", ":o:::"); // >
	s_initFont_setChar(59, "o::::", ":o:::", "::o::", ":::o:", "::::o"); // '\'
	s_initFont_setChar(60, ":oo:o", "o::o:", ":::::", ":::::", ":::::"); // ~
	s_initFont_setChar(61, ":ooo:", "o:::o", "o:ooo", "o:o:o", "::ooo"); // @
	s_initFont_setChar(62, ":oooo", "o:o::", ":ooo:", "::o:o", "oooo:"); // $
	s_initFont_setChar(63, ":oo::", ":o:::", ":oo::", "o::o:", ":oo:o"); // &
	s_initFont_setChar(64, "::oo:", "::o::", ":oo::", "::o::", "::oo:"); // {
	s_initFont_setChar(65, "::oo:", ":::o:", ":::oo", ":::o:", "::oo:"); // }
	s_initFont_setChar(66, "::o::", "::o::", ":::::", ":::::", ":::::"); // '
	s_initFont_setChar(67, ":o:o:", ":o:o:", ":::::", ":::::", ":::::"); // "
	s_initFont_setChar(68, "::o::", "::o::", "::o::", "::o::", "::o::"); // |
	s_initFont_setChar(69, ":o:::", "::o::", ":::::", ":::::", ":::::"); // `
	char c[] = "abcdefghijklmnopqrstuvwxyz0123456789!?:=,.-() #'*/+_[]%^;<>\\~@$&{}'\"|`";
	int i;
	for (i = 0; i < 256; i++) s_Transl[i] = 45;
	for (i = 0; i <= 70; i++) s_Transl[(unsigned char)c[i]] = i;

}

#define START_ADD_VERTEX(verts, primType) { stl_assert(verticesCurrent + (verts) < verticesEnd); i32 __vertIdx = static_cast<i32>(verticesCurrent - verticesBase);
#define ADD_VERTEX(X,Y,Z,U,V,Color) verticesCurrent->x = X; verticesCurrent->y = Y; verticesCurrent->z = Z; verticesCurrent->u = U; verticesCurrent->v = V; verticesCurrent->color = Color; ++verticesCurrent;
#define ADD_INDICES_2(i0, i1) stl_assert(indicesCurrent + 2 < indicesEnd); *(indicesCurrent++) = __vertIdx + i0; *(indicesCurrent++) = __vertIdx + i1; 
#define ADD_INDICES_3(i0, i1, i2) stl_assert(indicesCurrent + 3 < indicesEnd); *(indicesCurrent++) = __vertIdx + i0; *(indicesCurrent++) = __vertIdx + i1; *(indicesCurrent++) = __vertIdx + i2; 
#define ADD_INDICES_3R(i0, i2, i1) stl_assert(indicesCurrent + 3 < indicesEnd); *(indicesCurrent++) = __vertIdx + i0; *(indicesCurrent++) = __vertIdx + i1; *(indicesCurrent++) = __vertIdx + i2; 
#define END_ADD_VERTEX() if(currentTransform) _TransformVertices(verticesBase + __vertIdx, verticesCurrent - (verticesBase + __vertIdx), currentTransform); }

void geogenerator::AddQuad(const float* xyz, const float* xyzRight, const float* xyzBottom, unsigned int color /*= 0xffffffff*/, float u, float v, float u2, float v2)
{
	const float* fcolor = (float*)&color;
	float xyz2[3] = { xyz[0] + xyzRight[0], xyz[1] + xyzRight[1], xyz[2] + xyzRight[2] };
	
	START_ADD_VERTEX(4, gpu_primitive_type::TriangleList);
	ADD_VERTEX(xyz[0], xyz[1], xyz[2], u, v, color);
	ADD_VERTEX(xyz2[0], xyz2[1], xyz2[2], u, v, color);
	ADD_VERTEX(xyz2[0] + xyzBottom[0], xyz2[1] + xyzBottom[1], xyz2[2] + xyzBottom[2], u, v, color);
	ADD_VERTEX(xyz[0] + xyzBottom[0], xyz[1] + xyzBottom[1], xyz[2] + xyzBottom[2], u, v, color);
	ADD_INDICES_3(0, 2, 1);
	ADD_INDICES_3(0, 3, 2);
	END_ADD_VERTEX();
}


void geogenerator::AddRectangleLine(const float* xyz, const float* xyz2, unsigned int color /*= 0xffffffff*/, float u /*= 0.0f*/, float v /*= 0.0f*/, float u2 /*= 1.0f*/, float v2 /*= 1.0f*/)
{
	float xyzdisplacement[] = { xyz2[0] - xyz[0], 0.0f, 0.0f, 0.0f, xyz2[1] - xyz[1], 0.0f };
	AddQuadLine(xyz, xyzdisplacement, xyzdisplacement + 3, color, u, v, u2, v2);
}

void geogenerator::AddQuadLine(const float* xyz, const float* xyzRight, const float* xyzBottom, unsigned int color /*= 0xffffffff*/, float u, float v, float u2, float v2)
{
	const float* fcolor = (float*)&color;
	float xyz2[3] = { xyz[0] + xyzRight[0], xyz[1] + xyzRight[1], xyz[2] + xyzRight[2] };

	START_ADD_VERTEX(4, gpu_primitive_type::LineList);
	ADD_VERTEX(xyz[0], xyz[1], xyz[2], u, v, color);
	ADD_VERTEX(xyz2[0], xyz2[1], xyz2[2], u2, v, color);
	ADD_VERTEX(xyz2[0] + xyzBottom[0], xyz2[1] + xyzBottom[1], xyz2[2] + xyzBottom[2], u2, v2, color);
	ADD_VERTEX(xyz[0] + xyzBottom[0], xyz[1] + xyzBottom[1], xyz[2] + xyzBottom[2], u, v2, color);
	ADD_INDICES_2(0, 1);
	ADD_INDICES_2(1, 2);
	ADD_INDICES_2(2, 3);
	ADD_INDICES_2(3, 0);
	END_ADD_VERTEX();
}


void geogenerator::AddLine(const float* xyz, const float* xyz1, unsigned int color /*= 0xffffffff*/, float u /*= 0.0f*/, float v /*= 0.0f */, float u2/*=0.0f*/, float v2/*=0.0f*/)
{
	const float* fcolor = (float*)&color;

	START_ADD_VERTEX(2, gpu_primitive_type::LineList);
	ADD_VERTEX(xyz[0], xyz[1], xyz[2], u, v, color);
	ADD_VERTEX(xyz1[0], xyz1[1], xyz1[2], u2, v2, color);
	ADD_INDICES_2(0, 1);
	END_ADD_VERTEX();
}


void geogenerator::AddSegmentedLine(const float* xyz, int numVertices, unsigned int color /*= 0xffffffff*/, float u, float v)
{
	const float* fcolor = (float*)&color;
	int numIndices = (numVertices - 1) * 2;
	int vtx = 0, i = 0;

	START_ADD_VERTEX(numVertices, gpu_primitive_type::LineList);
	for (int i = 0; i < numVertices; i++)
	{
		ADD_VERTEX(xyz[0], xyz[1], xyz[2], u, v, color);
		xyz += 3;
	}

	while (i < numIndices)
	{
		ADD_INDICES_2(vtx, vtx+1);
		++vtx;
	}
	END_ADD_VERTEX();
}

void geogenerator::AddPolygonLine(const float* xyz, int numVertices, unsigned int color /*= 0xffffffff*/, float u /*= 0.0f*/, float v /*= 0.0f*/)
{
	const float* fcolor = (float*)&color;
	int numIndices = (numVertices - 1) * 2;
	int vtx = 0, i = 0;

	START_ADD_VERTEX(numVertices, gpu_primitive_type::LineList);
	for (int i = 0; i < numVertices; i++)
	{
		ADD_VERTEX(xyz[0], xyz[1], xyz[2], u, v, color);
		xyz += 3;
	}

	while (i < numIndices)
	{
		ADD_INDICES_2(vtx, vtx + 1);

		++vtx;
	}
	// close primitive
	ADD_INDICES_2(vtx, 0);

	END_ADD_VERTEX();
}

void geogenerator::AddRectangle(const float* xyz, const float* xyz2, unsigned int color /*= 0xffffffff*/, float u /*= 0.0f*/, float v /*= 0.0f*/, float u2 /*= 1.0f*/, float v2 /*= 1.0f*/)
{
	float xyzdisplacement[] = { xyz2[0] - xyz[0], 0.0f, 0.0f, 0.0f, xyz2[1] - xyz[1], 0.0f };
	AddQuad(xyz, xyzdisplacement, xyzdisplacement + 3, color, u, v, u2, v2);
}

void geogenerator::AddCubeLine(const float* v1, const float* v2, unsigned int color /*= 0xffffffff*/)
{
	START_ADD_VERTEX(8, gpu_primitive_type::LineList);
	ADD_VERTEX(v1[0], v1[1], v2[2], 0.000000, 0.000000, color);
	ADD_VERTEX(v2[0], v1[1], v2[2], 0.000000, 1.000000, color);
	ADD_VERTEX(v2[0], v2[1], v2[2], 0.000000, 0.000000, color);
	ADD_VERTEX(v1[0], v2[1], v2[2], 0.000000, 1.000000, color);
	ADD_VERTEX(v1[0], v1[1], v1[2], 1.000000, 0.000000, color);
	ADD_VERTEX(v2[0], v1[1], v1[2], 1.000000, 1.000000, color);
	ADD_VERTEX(v2[0], v2[1], v1[2], 1.000000, 0.000000, color);
	ADD_VERTEX(v1[0], v2[1], v1[2], 1.000000, 1.000000, color);
	
	ADD_INDICES_2(0, 1);
	ADD_INDICES_2(1, 2);
	ADD_INDICES_2(2, 3);
	ADD_INDICES_2(3, 0);
	// back						
	ADD_INDICES_2(4, 5);
	ADD_INDICES_2(5, 6);
	ADD_INDICES_2(6, 7);
	ADD_INDICES_2(7, 4);
	// vertical crossbeams		
	ADD_INDICES_2(0, 4);
	ADD_INDICES_2(1, 5);
	ADD_INDICES_2(2, 6);
	ADD_INDICES_2(3, 7);
	END_ADD_VERTEX();
}

void geogenerator::AddCube(const float* v1, const float* v2, unsigned int color /*= 0xffffffff*/)
{
	START_ADD_VERTEX(8, gpu_primitive_type::TriangleList);
	ADD_VERTEX(v1[0], v1[1], v2[2], 0.000000, 0.000000, color);
	ADD_VERTEX(v2[0], v1[1], v2[2], 0.000000, 1.000000, color);
	ADD_VERTEX(v2[0], v2[1], v2[2], 0.000000, 0.000000, color);
	ADD_VERTEX(v1[0], v2[1], v2[2], 0.000000, 1.000000, color);
	ADD_VERTEX(v1[0], v1[1], v1[2], 1.000000, 0.000000, color);
	ADD_VERTEX(v2[0], v1[1], v1[2], 1.000000, 1.000000, color);
	ADD_VERTEX(v2[0], v2[1], v1[2], 1.000000, 0.000000, color);
	ADD_VERTEX(v1[0], v2[1], v1[2], 1.000000, 1.000000, color);
	ADD_INDICES_3R(0, 1, 2);
	ADD_INDICES_3R(2, 3, 0);
	ADD_INDICES_3R(3, 2, 6);
	ADD_INDICES_3R(6, 7, 3);
	ADD_INDICES_3R(7, 6, 5);
	ADD_INDICES_3R(5, 4, 7);
	ADD_INDICES_3R(4, 5, 1);
	ADD_INDICES_3R(1, 0, 4);
	ADD_INDICES_3R(4, 0, 3);
	ADD_INDICES_3R(3, 7, 4);
	ADD_INDICES_3R(1, 5, 6);
	ADD_INDICES_3R(6, 2, 1);
	END_ADD_VERTEX();
}

void geogenerator::AddCubeWithUV(const float* v1, const float* v2, unsigned int color /*= 0xffffffff*/)
{
	START_ADD_VERTEX(24, gpu_primitive_type::TriangleList);
	
	ADD_VERTEX(v1[0], v1[1], v2[2], 0.000000, 0.000000, color);
	ADD_VERTEX(v2[0], v1[1], v2[2], 1.000000, 0.000000, color);
	ADD_VERTEX(v1[0], v2[1], v2[2], 0.000000, 1.000000, color);
	ADD_VERTEX(v2[0], v2[1], v2[2], 1.000000, 1.000000, color);
	ADD_VERTEX(v1[0], v2[1], v2[2], 0.000000, 0.000000, color);
	ADD_VERTEX(v2[0], v2[1], v2[2], 1.000000, 0.000000, color);
	ADD_VERTEX(v1[0], v2[1], v1[2], 0.000000, 1.000000, color);
	ADD_VERTEX(v2[0], v2[1], v1[2], 1.000000, 1.000000, color);
	
	ADD_VERTEX(v1[0], v2[1], v1[2], 1.000000, 1.000000, color);
	ADD_VERTEX(v2[0], v2[1], v1[2], 0.000000, 1.000000, color);
	ADD_VERTEX(v1[0], v1[1], v1[2], 1.000000, 0.000000, color);
	ADD_VERTEX(v2[0], v1[1], v1[2], 0.000000, 0.000000, color);
	ADD_VERTEX(v1[0], v1[1], v1[2], 0.000000, 0.000000, color);
	ADD_VERTEX(v2[0], v1[1], v1[2], 1.000000, 0.000000, color);
	ADD_VERTEX(v1[0], v1[1], v2[2], 0.000000, 1.000000, color);
	ADD_VERTEX(v2[0], v1[1], v2[2], 1.000000, 1.000000, color);
	
	ADD_VERTEX(v2[0], v1[1], v2[2], 0.000000, 0.000000, color);
	ADD_VERTEX(v2[0], v1[1], v1[2], 1.000000, 0.000000, color);
	ADD_VERTEX(v2[0], v2[1], v2[2], 0.000000, 1.000000, color);
	ADD_VERTEX(v2[0], v2[1], v1[2], 1.000000, 1.000000, color);
	ADD_VERTEX(v1[0], v1[1], v1[2], 0.000000, 0.000000, color);
	ADD_VERTEX(v1[0], v1[1], v2[2], 1.000000, 0.000000, color);
	ADD_VERTEX(v1[0], v2[1], v1[2], 0.000000, 1.000000, color);
	ADD_VERTEX(v1[0], v2[1], v2[2], 1.000000, 1.000000, color);

	ADD_INDICES_3R(0, 1, 2);
	ADD_INDICES_3R(2, 1, 3);
	ADD_INDICES_3R(4, 5, 6);
	ADD_INDICES_3R(6, 5, 7);
	ADD_INDICES_3R(8, 9, 10);
	ADD_INDICES_3R(10, 9, 11);
	ADD_INDICES_3R(12, 13, 14);
	ADD_INDICES_3R(14, 13, 15);
	ADD_INDICES_3R(16, 17, 18);
	ADD_INDICES_3R(18, 17, 19);
	ADD_INDICES_3R(20, 21, 22);
	ADD_INDICES_3R(22, 21, 23);
	END_ADD_VERTEX();
}

void geogenerator::AddDebugText(const float* xyz, const char* text, float scale, unsigned int color)
{
	const unsigned char* utext = (const unsigned char*)text;
	size_t ln = strlen(text);
	float uShift = 1.0f/128.0f;
	float xShift = scale * 1.2f;
	float pos[3] = {xyz[0], xyz[1], xyz[2]};
	float lx[3] = {scale, 0.0f, 0.0f };
	float ly[3] = {0.0f, scale, 0.0f };
	for (size_t i = 0; i<ln; i++)
	{
		int xpos=s_Transl[tolower(utext[i])];
		float u=xpos*uShift;
		float u2=u+uShift;
		AddQuad(pos, lx, ly, color, u, 0.0f, u2, 1.0f);
		pos[0] += xShift;
	}
}


float geogenerator::MeasureDebugText(const char* text, float scale)
{
	return strlen(text)*scale*8.0f;
}

void geogenerator::SetCPUMatrixTransform(const float* transformMatrix/*=0*/)
{
	if (transformMatrix == 0)
		currentTransform = 0;
	else
	{
		currentTransform = transform;
		memcpy(transform, transformMatrix, sizeof(float) * 16);
	}
}

void geogenerator::_TransformVertices(geogenerator_vtx* vertices, usize numVertices, const float* transformMatrix)
{
	float newx, newy, newz;
	for (int i = 0; i < numVertices; i++)
	{
		// v = m * v (column matrix transformation)
		float* _vtxData = &vertices[i].x;
		newx = transformMatrix[0] * _vtxData[0] + transformMatrix[0 + 4] * _vtxData[1] + transformMatrix[0 + 8] * _vtxData[2] + transformMatrix[0 + 12];
		newy = transformMatrix[1] * _vtxData[0] + transformMatrix[1 + 4] * _vtxData[1] + transformMatrix[1 + 8] * _vtxData[2] + transformMatrix[1 + 12];
		newz = transformMatrix[2] * _vtxData[0] + transformMatrix[2 + 4] * _vtxData[1] + transformMatrix[2 + 8] * _vtxData[2] + transformMatrix[2 + 12];
		_vtxData[0] = newx;
		_vtxData[1] = newy;
		_vtxData[2] = newz;
	}
}

}; // end namespace cfc