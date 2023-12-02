#pragma once
#include <vector>

#include "DataTypes.h"

namespace dae
{
	class Renderer;

	class Mesh
	{
	public:

		friend Renderer;

		Mesh(std::vector<VertexModel> vertices, std::vector<uint32_t> indices, std::vector<Material*> materials, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList);
		Mesh(const std::string& name, std::vector<Material*> materials, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList);

		void SetPosition(Vector3 translate);
		void SetScale(Vector3 scale);
		void SetYawRotation(float yaw);

		void AddYawRotation(float yawDelta);

	private:

		std::vector<VertexModel> m_VerticesModel;
		std::vector<VertexTransformed> m_VerticesTransformed;
		std::vector<Triangle> m_Triangles;

		std::vector<uint32_t> m_Indices;

		std::vector<Material*> m_MaterialPtrs;
		PrimitiveTopology m_PrimitiveTopology;

		Matrix	m_WorldMatrix
		{
			{1.0f,0.0f,0.0f,0.0f},
			{0.0f,1.0f,0.0f,0.0f},
			{0.0f,0.0f,1.0f,0.0f},
			{0.0f,0.0f,0.0f,1.0f}
		};

		float m_YawRotation;
		Vector3 m_Scale;
		Vector3 m_Position;

		void InitializeVertexColors();
		void InitializeTriangles();
		void UpdateWorldMatrix();
		void ResetTransformedVertices();
	};
}
