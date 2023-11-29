#include "Mesh.h"

#include <cassert>

#include "Utils.h"


namespace dae
{
	Mesh::Mesh(std::vector<VertexModel> vertices, std::vector<uint32_t> indices,
		std::vector<Material*> materials, PrimitiveTopology primitiveTopology) :
		m_VerticesModel(std::move(vertices)),
		m_VerticesTransformed(m_VerticesModel.size(), std::make_shared<VertexTransformed>()),
		m_Indices(std::move(indices)),
		m_MaterialPtrs(std::move(materials)),
		m_PrimitiveTopology(primitiveTopology),
		m_YawRotation(0.0f),
		m_Scale(1.0f, 1.0f, 1.0f),
		m_Position(0.0f, 0.0f, 0.0f)
	{
		InitializeVerticesTransformed();
		InitializeVertexColors();
		InitializeTriangles();
	}

	Mesh::Mesh(const std::string& name, std::vector<Material*> materials, PrimitiveTopology primitiveTopology) :
		m_MaterialPtrs(std::move(materials)),
		m_PrimitiveTopology(primitiveTopology),
		m_YawRotation(0.0f),
		m_Scale(1.0f, 1.0f, 1.0f),
		m_Position(0.0f, 0.0f, 0.0f)
	{
		Utils::ParseOBJ(name, m_VerticesModel, m_Indices);

		InitializeVerticesTransformed();
		InitializeVertexColors();
		InitializeTriangles();
	}


	void Mesh::SetPosition(Vector3 translate)
	{
		m_Position = translate;
		UpdateWorldMatrix();
	}

	void Mesh::SetScale(Vector3 scale)
	{
		m_Scale = scale;
		UpdateWorldMatrix();
	}

	void Mesh::SetYawRotation(float yawRotation)
	{
		m_YawRotation = yawRotation;
		UpdateWorldMatrix();
	}

	void Mesh::AddYawRotation(float yawDelta)
	{
		m_YawRotation += yawDelta;
		UpdateWorldMatrix();
	}


	void Mesh::InitializeVerticesTransformed()
	{
		m_VerticesTransformed.resize(m_VerticesModel.size());
		for (size_t i = 0; i < m_VerticesModel.size(); i++)
			m_VerticesTransformed[i] = std::make_shared<VertexTransformed>();
	}

	void Mesh::InitializeVertexColors()
	{
		// Color world vertex
		int vertexIndex{ 0 };
		for (VertexModel& vertex : m_VerticesModel)
		{
			if (vertexIndex == 0)
				vertex.color = colors::Red;

			if (vertexIndex == 1)
				vertex.color = colors::Green;

			if (vertexIndex == 2)
				vertex.color = colors::Blue;

			vertexIndex++;
			vertexIndex %= 3;
		}
	}

	void Mesh::InitializeTriangles()
	{
		m_Triangles.clear();

		if (m_PrimitiveTopology == PrimitiveTopology::TriangleList)
		{
			for (int triangleIndex{}; triangleIndex < static_cast<int>(m_Indices.size()); triangleIndex += 3)
			{
				m_Triangles.push_back(
				{
					m_VerticesTransformed[m_Indices[triangleIndex]],
					m_VerticesTransformed[m_Indices[triangleIndex + 1]],
					m_VerticesTransformed[m_Indices[triangleIndex + 2]],
				});
			}
		}
		else
		{
			for (int triangleIndex{}; triangleIndex < static_cast<int>(m_Indices.size() - 2); triangleIndex++)
			{
				if (triangleIndex % 2 == 0)
				{
					m_Triangles.push_back(
					{
						m_VerticesTransformed[m_Indices[triangleIndex]],
						m_VerticesTransformed[m_Indices[triangleIndex + 1]],
						m_VerticesTransformed[m_Indices[triangleIndex + 2]]
					});
				}
				else
				{
					m_Triangles.push_back(
					{
						m_VerticesTransformed[m_Indices[triangleIndex]],
						m_VerticesTransformed[m_Indices[triangleIndex + 2]],
						m_VerticesTransformed[m_Indices[triangleIndex + 1]]
					});
				}
			}
		}
	}


	void Mesh::UpdateWorldMatrix()
	{
		const Matrix translationMatrix
		{
			{1.0f,0.0f,0.0f},
			{0.0f,1.0f,0.0f},
			{0.0f,0.0f,1.0f},
			{m_Position}
		};

		const Matrix rotateMatrix
		{
			{ std::cos(m_YawRotation), 0, -std::sin(m_YawRotation), 0},
			{0.f, 1.f, 0.f, 0.f},
			{std::sin(m_YawRotation),0.f, std::cos(m_YawRotation), 0.f},
			{0.f, 0.f, 0.f, 1.f}
		};

		const Matrix scaleMatrix
		{
			{m_Scale.x,0.f,0.f,0.f},
			{0.f,m_Scale.y,0.f,0.f},
			{0.f,0.f,m_Scale.z,0.f},
			{0.f,0.f,0.f,1.f}
		};

		m_WorldMatrix = scaleMatrix * rotateMatrix * translationMatrix;
	}

	void Mesh::ResetTransformedVertices()
	{
		assert(m_VerticesModel.size() == m_VerticesTransformed.size());

		for (size_t i = 0; i < m_VerticesModel.size(); i++)
		{
			m_VerticesTransformed[i]->pos = { m_VerticesModel[i].pos.x,m_VerticesModel[i].pos.y,m_VerticesModel[i].pos.z,1.0f };
			m_VerticesTransformed[i]->uv = m_VerticesModel[i].uv;
			m_VerticesTransformed[i]->normal = m_VerticesModel[i].normal;
			m_VerticesTransformed[i]->tangent = m_VerticesModel[i].tangent;
			m_VerticesTransformed[i]->viewDirection = m_VerticesModel[i].viewDirection;
			m_VerticesTransformed[i]->materialIndex = m_VerticesModel[i].materialIndex;
			m_VerticesTransformed[i]->color = m_VerticesModel[i].color;
		}
	}
}
