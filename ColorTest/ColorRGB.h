#pragma once
#include <algorithm>


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


#pragma region ColorRGB (Member) Operators
	const ColorRGB& operator+=(const ColorRGB& c)
	{
		r += c.r;
		g += c.g;
		b += c.b;

		return *this;
	}

	const ColorRGB& operator+(const ColorRGB& c)
	{
		return *this += c;
	}

	ColorRGB operator+(const ColorRGB& c) const
	{
		return { r + c.r, g + c.g, b + c.b };
	}

	const ColorRGB& operator-=(const ColorRGB& c)
	{
		r -= c.r;
		g -= c.g;
		b -= c.b;

		return *this;
	}

	const ColorRGB& operator-(const ColorRGB& c)
	{
		return *this -= c;
	}

	ColorRGB operator-(const ColorRGB& c) const
	{
		return { r - c.r, g - c.g, b - c.b };
	}

	const ColorRGB& operator*=(const ColorRGB& c)
	{
		r *= c.r;
		g *= c.g;
		b *= c.b;

		return *this;
	}

	const ColorRGB& operator*(const ColorRGB& c)
	{
		return *this *= c;
	}

	ColorRGB operator*(const ColorRGB& c) const
	{
		return { r * c.r, g * c.g, b * c.b };
	}

	const ColorRGB& operator/=(const ColorRGB& c)
	{
		r /= c.r;
		g /= c.g;
		b /= c.b;

		return *this;
	}

	const ColorRGB& operator/(const ColorRGB& c)
	{
		return *this /= c;
	}

	const ColorRGB& operator*=(float s)
	{
		r *= s;
		g *= s;
		b *= s;

		return *this;
	}

	const ColorRGB& operator*(float s)
	{
		return *this *= s;
	}

	ColorRGB operator*(float s) const
	{
		return { r * s, g * s,b * s };
	}

	const ColorRGB& operator/=(float s)
	{
		r /= s;
		g /= s;
		b /= s;

		return *this;
	}

	const ColorRGB& operator/(float s)
	{
		return *this /= s;
	}
#pragma endregion
};

//ColorRGB (Global) Operators
inline ColorRGB operator*(float s, const ColorRGB& c)
{
	return c * s;
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
