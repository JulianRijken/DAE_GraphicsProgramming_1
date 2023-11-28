#pragma once
#include "Maths.h"
#include "Texture.h"
#include "vector"

namespace dae
{
	struct VertexModel
	{
		Vector3 position{};
		Vector2 uv{};
		Vector3 normal{}; 
		Vector3 tangent{};
		Vector3 viewDirection{};
		int materialIndex{0}; 
		ColorRGB color{colors::White};
	};

	struct Triangle
	{
		VertexModel vertex0;
		VertexModel vertex1;
		VertexModel vertex2;
	};

	struct VertexTransformed
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
		int materialIndex{ 0 };
		ColorRGB color{ colors::White };
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	struct Material
	{
		Texture* color = nullptr;
		Texture* opacity = nullptr;
	};

	class Mesh
	{
	public:
		//Mesh(std::vector<VertexModel> vertexModels)


	private:

		std::vector<VertexModel> m_VerticesModel{};
		std::vector<VertexTransformed> m_VerticesTransformed{};

		int vertexCount{};

		std::vector<uint32_t> indices{};
		std::vector<Material*> materialPtrs{};

		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

		Matrix worldMatrix
		{
			{1.0f,0.0f,0.0f},
			{0.0f,1.0f,0.0f},
			{0.0f,0.0f,1.0f},
			{0.0f,0.0f,0.0f}
		};

		void Translate(Vector3 translate)
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

		void Scale(Vector3 scale)
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

		void Rotate(float yaw)
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
	};


}
