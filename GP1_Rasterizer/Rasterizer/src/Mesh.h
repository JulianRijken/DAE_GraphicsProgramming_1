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

		Mesh(const std::vector<VertexModel>& vertices,const std::vector<uint32_t>& indices, const std::vector<Material*>& materials, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList);
		Mesh(const std::string& name, const std::vector<Material*>& materials, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList);

		void Translate(Vector3 translate);
		void Scale(Vector3 scale);
		void Rotate(float yaw);

	private:

		std::vector<VertexModel> m_VerticesModel;
		std::vector<VertexTransformed> m_VerticesTransformed;

		std::vector<uint32_t> m_Indices;

		std::vector<Material*> m_MaterialPtrs;
		PrimitiveTopology m_PrimitiveTopology;

		Matrix m_WorldMatrix
		{
			{1.0f,0.0f,0.0f},
			{0.0f,1.0f,0.0f},
			{0.0f,0.0f,1.0f},
			{0.0f,0.0f,0.0f}
		};

		void ResetTransformedVertices();
	};
}
