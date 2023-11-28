#include "Mesh.h"
#include "Utils.h"


namespace dae
{
	Mesh::Mesh(const std::vector<VertexModel>& vertices, const std::vector<uint32_t>& indices,
		const std::vector<Material*>& materials, PrimitiveTopology primitiveTopology) :
		m_VerticesModel(vertices),
		m_Indices(indices),
		m_MaterialPtrs(materials),
		m_PrimitiveTopology(primitiveTopology)
	{
	}

	Mesh::Mesh(const std::string& name, const std::vector<Material*>& materials, PrimitiveTopology primitiveTopology) :
		m_MaterialPtrs(materials),
		m_PrimitiveTopology(primitiveTopology)
	{
		Utils::ParseOBJ(name, m_VerticesModel, m_Indices);
	}

	void Mesh::Translate(Vector3 translate)
	{
		const Matrix translateMatrix{

			{1.0f,0.0f,0.0f},
			{0.0f,1.0f,0.0f},
			{0.0f,0.0f,1.0f},
			{translate}
		};

		for (VertexModel& vertex : m_VerticesModel)
			vertex.position = translateMatrix.TransformPoint(vertex.position);
	}

	void Mesh::Scale(Vector3 scale)
	{
		const Matrix scaleMatrix{

				{scale.x,0.f,0.f,0.f},
				{0.f,scale.y,0.f,0.f},
				{0.f,0.f,scale.z,0.f},
				{0.f,0.f,0.f,1.f}
		};

		for (VertexModel& vertex : m_VerticesModel)
			vertex.position = scaleMatrix.TransformPoint(vertex.position);
	}

	void Mesh::Rotate(float yaw)
	{
		const Matrix rotateMatrix{
			{ cosf(yaw), 0, -sinf(yaw), 0},
			{0.f, 1.f, 0.f, 0.f},
			{sinf(yaw),0.f, cosf(yaw), 0.f},
			{0.f, 0.f, 0.f, 1.f}
		};

		for (VertexModel& vertex : m_VerticesModel)
			vertex.position = rotateMatrix.TransformPoint(vertex.position);
	}

	void Mesh::ResetTransformedVertices()
	{
		m_VerticesTransformed.clear();
		m_VerticesTransformed.resize(m_VerticesModel.size());

		for (size_t i = 0; i < m_VerticesModel.size(); ++i)
		{
			m_VerticesTransformed[i].position = {m_VerticesModel[i].position.x;
			m_VerticesTransformed[i].uv = m_VerticesModel[i].uv;
			m_VerticesTransformed[i].normal = m_VerticesModel[i].normal;
			m_VerticesTransformed[i].tangent = m_VerticesModel[i].tangent;
			m_VerticesTransformed[i].viewDirection = m_VerticesModel[i].viewDirection;
			m_VerticesTransformed[i].materialIndex = m_VerticesModel[i].materialIndex;
			m_VerticesTransformed[i].color = m_VerticesModel[i].color;
		}

		//struct VertexModel
		//{
		//	Vector3 position{};
		//	Vector2 uv{};
		//	Vector3 normal{};
		//	Vector3 tangent{};
		//	Vector3 viewDirection{};
		//	int materialIndex{ 0 };
		//	ColorRGB color{ colors::White };
		//};

		//struct VertexTransformed
		//{
		//	Vector4 position{};
		//	Vector2 uv{};
		//	Vector3 normal{};
		//	Vector3 tangent{};
		//	Vector3 viewDirection{};
		//	int materialIndex{ 0 };
		//	ColorRGB color{ colors::White };
		//};
	}
}
