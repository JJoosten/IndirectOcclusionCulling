#pragma once

#include <cfc/base.h>
#include <cfc/core/context.h>
#include <cfc/gpu/gpu.h>
#include <cfc/gpu/gfx.h>

#include <cfc/stl/stl_vector.hpp>

namespace cfc
{
#pragma pack(push,1)
	struct geogenerator_vtx
	{
		float x;
		float y;
		float z;
		float u;
		float v;
		unsigned int color;
	};
#pragma pack(pop)

	class geogenerator_inst
	{
	public:
		int indexCount = 0;
		int indexOffset = 0;
		gpu_primitive_type primType = gpu_primitive_type::TriangleList;
	};

	class geogenerator
	{
	public:
		geogenerator(geogenerator_vtx* vertexBuffer, usize vertexBufferCapacity, int* indexBuffer, usize indexBufferCapacity)
		{
			verticesBase = verticesEnd = verticesCurrent = vertexBuffer;
			indicesBase = indicesEnd = indicesCurrent = indexBuffer;
			indicesEnd += indexBufferCapacity;
			verticesEnd += vertexBufferCapacity;
		}

		usize GetNumIndices() { return indicesCurrent - indicesBase; }
		usize GetNumVertices() { return verticesCurrent - verticesBase; }
		usize GetNumIndicesBytes() { return (indicesCurrent - indicesBase) * sizeof(*indicesBase); }
		usize GetNumVerticesBytes() { return (verticesCurrent - verticesBase) * sizeof(*verticesBase); }
		usize GetInstructionCount() { return instructions.size(); }
		geogenerator_inst& GetInstruction(usize idx) { return instructions[idx]; }

		void SetCPUMatrixTransform(const float* transformMatrix = 0);
		void AddLine(const float* xyz, const float* xyz1, unsigned int color = 0xffffffff, float u = 0.0f, float v = 0.0f, float u2 = 1.0f, float v2 = 1.0f);
		void AddSegmentedLine(const float* xyz, int numVertices, unsigned int color = 0xffffffff, float u = 0.0f, float v = 0.0f);
		void AddPolygonLine(const float* xyz, int numVertices, unsigned int color = 0xffffffff, float u = 0.0f, float v = 0.0f);
		void AddQuadLine(const float* xyz, const float* xyzRight, const float* xyzBottom, unsigned int color = 0xffffffff, float u = 0.0f, float v = 0.0f, float u2 = 1.0f, float v2 = 1.0f);
		void AddQuad(const float* xyz, const float* xyzRight, const float* xyzBottom, unsigned int color = 0xffffffff, float u = 0.0f, float v = 0.0f, float u2 = 1.0f, float v2 = 1.0f);
		void AddRectangleLine(const float* xyz, const float* xyz2, unsigned int color = 0xffffffff, float u = 0.0f, float v = 0.0f, float u2 = 1.0f, float v2 = 1.0f);
		void AddRectangle(const float* xyz, const float* xyz2, unsigned int color = 0xffffffff, float u = 0.0f, float v = 0.0f, float u2 = 1.0f, float v2 = 1.0f);
		void AddDebugText(const float* xyz, const char* text, float scale = 5.0f, unsigned int color = 0xffffffff);

		void AddCube(const float* xyz, const float* xyz2, unsigned int color = 0xffffffff);
		void AddCubeLine(const float* v1, const float* v2, unsigned int color = 0xffffffff);
		void AddCubeWithUV(const float* xyz, const float* xyz2, unsigned int color = 0xffffffff);

		float MeasureDebugText(const char* text, float scale = 5.0f);
	protected:
		void _TransformVertices(geogenerator_vtx* vertices, usize numVertices, const float* transformMatrix);

		geogenerator_vtx *verticesBase=nullptr, *verticesEnd = nullptr, *verticesCurrent = nullptr;
		int *indicesBase = nullptr, *indicesEnd = nullptr, *indicesCurrent = nullptr;
		stl_vector<geogenerator_inst> instructions;
		float* currentTransform = nullptr;
		float transform[16];
	};

}; // end namespace cfc