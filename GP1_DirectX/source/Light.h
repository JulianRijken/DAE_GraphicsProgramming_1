#pragma once

#include "ColorRGB.h"
#include "Vector3.h"

namespace dae
{
	enum class LightType
	{
		Directional = 0,
		Point = 1,
	};

	class Light
	{
	public:
		Light(const Vector3& origin, const Vector3& direction, const ColorRGB& color, const float intensity, const LightType type) :
			// Type(1),
			Intensity(intensity),
			Origin(origin),
			Direction(direction),
			Color({color.r,color.g,color.b,1.0f})
		{}

		float Intensity{};
		Vector3 Origin{};
		Vector3 Direction{};
		Vector4 Color{};
		// UINT32 Type{};
	};
}
