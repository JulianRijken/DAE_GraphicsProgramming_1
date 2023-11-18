#pragma once
#include "Maths.h"
#include "Texture.h"
#include "vector"

namespace dae
{
	struct Vertex
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
		Vertex vertex0;
		Vertex vertex1;
		Vertex vertex2;
	};

	struct Vertex_Out
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
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

	struct Mesh
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

		std::vector<Vertex_Out> vertices_out{};
		std::vector<Material*> materialPtrs{};
		//Matrix worldMatrix{};

		void Translate(Vector3 translate)
		{
			const Matrix translateMatrix{

				{1.0f,0.0f,0.0f},
				{0.0f,1.0f,0.0f},
				{0.0f,0.0f,1.0f},
				{translate}
			};

			for (Vertex& vertex : vertices)
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

			for (Vertex& vertex : vertices)
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

			for (Vertex& vertex : vertices)
				vertex.position = rotateMatrix.TransformPoint(vertex.position);
		}
	};


}
