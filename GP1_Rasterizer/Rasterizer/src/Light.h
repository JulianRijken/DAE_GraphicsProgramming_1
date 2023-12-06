#pragma once

#include "ColorRGB.h"
#include "Vector3.h"

namespace dae
{
	enum class LightType
	{
		Point,
		Directional
	};

	class Light
	{
	public:
		Light(const Vector3& origin,const Vector3& direction,const ColorRGB& color,float intensity, LightType type);

		ColorRGB GetRadiance(const Vector3& target) const;
		const Vector3& GetOrigin() const { return m_Origin; }
		const Vector3& GetDirection() const { return m_Direction; }

		const LightType& GetType() const { return m_Type; }

	private:

		LightType m_Type{};

		Vector3 m_Origin{};
		Vector3 m_Direction{};
		ColorRGB m_Color{};
		float m_Intensity{};
	};
}
