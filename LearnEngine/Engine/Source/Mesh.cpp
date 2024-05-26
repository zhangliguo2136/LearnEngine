#include "Mesh.h"
#include "MyMath.h"

TMesh::TMesh()
{
	InputLayoutName = "DefaultInputLayout";
}

void TMesh::GenerateIndices16()
{
	if (Indices16.empty())
	{
		Indices16.resize(Indices32.size());
		for (size_t i = 0; i < Indices32.size(); ++i)
		{
			Indices16[i] = static_cast<uint16_t>(Indices32[i]);
		}
	}
}

void TMesh::CreateSphere(float Radius, uint32_t SliceCount, uint32_t StackCount)
{
	TVertex topVertex(0.0f, +Radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	TVertex bottomVertex(0.0f, -Radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	Vertices.push_back(topVertex);

	float phiStep = TMath::PI / StackCount;
	float thetaStep = 2.0f * TMath::PI / SliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32_t i = 1; i <= StackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (uint32_t j = 0; j <= SliceCount; ++j)
		{
			float theta = j * thetaStep;

			TVertex v;

			// spherical to cartesian
			v.Position.x = Radius * sinf(phi) * cosf(theta);
			v.Position.y = Radius * cosf(phi);
			v.Position.z = Radius * sinf(phi) * sinf(theta);

			// Partial derivative of P with respect to theta
			v.TangentU.x = -Radius * sinf(phi) * sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +Radius * sinf(phi) * cosf(theta);
			v.TangentU.Normalize();

			v.Normal = v.Position;
			v.Normal.Normalize();

			v.TexC.x = theta / TMath::PI * 2;
			v.TexC.y = phi / TMath::PI;

			Vertices.push_back(v);
		}
	}

	Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (uint32_t i = 1; i <= SliceCount; ++i)
	{
		Indices32.push_back(0);
		Indices32.push_back(i + 1);
		Indices32.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	uint32_t baseIndex = 1;
	uint32_t ringVertexCount = SliceCount + 1;
	for (uint32_t i = 0; i < StackCount - 2; ++i)
	{
		for (uint32_t j = 0; j < SliceCount; ++j)
		{
			Indices32.push_back(baseIndex + i * ringVertexCount + j);
			Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint32_t southPoleIndex = (uint32_t)Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32_t i = 0; i < SliceCount; ++i)
	{
		Indices32.push_back(southPoleIndex);
		Indices32.push_back(baseIndex + i);
		Indices32.push_back(baseIndex + i + 1);
	}

	GenerateIndices16();
}

void TMesh::CreateQuad(float x, float y, float w, float h, float depth)
{
	Vertices.resize(4);
	Indices32.resize(6);

	// Position coordinates specified in NDC space.
	Vertices[0] = TVertex(
		x, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	Vertices[1] = TVertex(
		x, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	Vertices[2] = TVertex(
		x + w, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	Vertices[3] = TVertex(
		x + w, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	Indices32[0] = 0;
	Indices32[1] = 1;
	Indices32[2] = 2;

	Indices32[3] = 0;
	Indices32[4] = 2;
	Indices32[5] = 3;

	GenerateIndices16();
}
