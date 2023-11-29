#pragma once
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
		VertexTransformed vertex0;
		VertexTransformed vertex1;
		VertexTransformed vertex2;
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
		Texture* normalMap = nullptr;
	};

}
