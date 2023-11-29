#pragma once
#include <memory>

#include "Maths.h"
#include "Texture.h"
#include "vector"

namespace dae
{
	struct VertexModel
	{
		Vector3 pos{};
		Vector2 uv{};
		Vector3 normal{}; 
		Vector3 tangent{};
		Vector3 viewDirection{};
		int materialIndex{0}; 
		ColorRGB color{colors::White};
	};

	struct VertexTransformed
	{
		Vector4 pos{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
		int materialIndex{ 0 };
		ColorRGB color{ colors::White };
	};

	struct Triangle
	{
		std::shared_ptr<VertexTransformed> vertex0;
		std::shared_ptr<VertexTransformed> vertex1;
	 	std::shared_ptr<VertexTransformed> vertex2;
	};


	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	struct Material
	{
		Texture* diffuse = nullptr;
		Texture* opacity = nullptr;
		Texture* normal = nullptr;
		Texture* specular = nullptr;
		Texture* gloss = nullptr;
	};

}
