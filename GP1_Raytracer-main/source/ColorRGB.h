#pragma once
#include "MathHelpers.h"

namespace dae
{
	struct ColorRGB
	{
		float r{};
		float g{};
		float b{};

		void MaxToOne()
		{
			const float maxValue = std::max(r, std::max(g, b));
			if (maxValue > 1.f)
				*this /= maxValue;
		}

		static ColorRGB Lerp(const ColorRGB& c1, const ColorRGB& c2, float factor)
		{
			return { Lerpf(c1.r, c2.r, factor), Lerpf(c1.g, c2.g, factor), Lerpf(c1.b, c2.b, factor) };
		}

		ColorRGB& operator+=(const ColorRGB& c)
		{
			r += c.r;
			g += c.g;
			b += c.b;

			return *this;
		}
		ColorRGB& operator-=(const ColorRGB& c)
		{
			r -= c.r;
			g -= c.g;
			b -= c.b;

			return *this;
		}
		ColorRGB& operator*=(const ColorRGB& c)
		{
			r *= c.r;
			g *= c.g;
			b *= c.b;

			return *this;
		}
		ColorRGB& operator*=(float s)
		{
			r *= s;
			g *= s;
			b *= s;

			return *this;
		}
		ColorRGB& operator/=(const ColorRGB& c)
		{
			r /= c.r;
			g /= c.g;
			b /= c.b;

			return *this;
		}
		ColorRGB& operator/=(float s)
		{
			r /= s;
			g /= s;
			b /= s;

			return *this;
		}
	};

	inline ColorRGB operator*(float lhs, const ColorRGB& rhs)
	{
		return { lhs * rhs.r, lhs * rhs.g, lhs * rhs.b };
	}

	inline ColorRGB operator*(const ColorRGB& lhs, float rhs)
	{
		return { lhs.r * rhs, lhs.g * rhs, lhs.b * rhs };
	}

	inline ColorRGB operator*(const ColorRGB& lhs, const ColorRGB& rhs)
	{
		return { lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b };
	}

	inline ColorRGB operator/(const ColorRGB& lhs, float rhs)
	{
		return { lhs.r / rhs, lhs.g / rhs, lhs.b / rhs };
	}

	inline ColorRGB operator/(float lhs, const ColorRGB& rhs)
	{
		return { lhs / rhs.r, lhs / rhs.g, lhs / rhs.b };
	}

	inline ColorRGB operator/(const ColorRGB& lhs, const ColorRGB& rhs)
	{
		return { lhs.r / rhs.r, lhs.g / rhs.g, lhs.b / rhs.b };
	}

	inline ColorRGB operator+(const ColorRGB& lhs, const ColorRGB& rhs)
	{
		return { lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b };
	}

	inline ColorRGB operator-(const ColorRGB& lhs, const ColorRGB& rhs)
	{
		return { lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b };
	}

	inline ColorRGB operator-(const float lhs, const ColorRGB& rhs)
	{
		return { lhs - rhs.r, lhs - rhs.g, lhs - rhs.b };
	}


	namespace colors
	{
		static ColorRGB Red{ 1,0,0 };
		static ColorRGB Blue{ 0,0,1 };
		static ColorRGB Green{ 0,1,0 };
		static ColorRGB Yellow{ 1,1,0 };
		static ColorRGB Cyan{ 0,1,1 };
		static ColorRGB Magenta{ 1,0,1 };
		static ColorRGB White{ 1,1,1 };
		static ColorRGB Black{ 0,0,0 };
		static ColorRGB Gray{ 0.5f,0.5f,0.5f };
	}
}