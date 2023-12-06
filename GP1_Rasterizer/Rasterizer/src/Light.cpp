#include "Light.h"

namespace dae
{
	Light::Light(const Vector3& origin, const Vector3& direction, const ColorRGB& color, const float intensity, const LightType type) :
		m_Type(type),
		m_Origin(origin),
		m_Direction(direction),
		m_Color(color),
		m_Intensity(intensity)
	{}


	ColorRGB Light::GetRadiance(const Vector3& target) const
	{
		if (m_Type == LightType::Point)
			return{ m_Color * (m_Intensity / Square((m_Origin - target).Magnitude())) };

		if (m_Type == LightType::Directional)
			return{ m_Color * m_Intensity };

		return {0,0,0};
	}

}
