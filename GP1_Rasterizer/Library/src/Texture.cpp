#include "Texture.h"

#include <algorithm>
#include <format>
#include <iostream>
#include <SDL_image.h>

#include "Vector2.h"
#include "../../Rasterizer/src/GlobalSettings.h"

namespace dae
{

	Texture::Texture(SDL_Surface* pSurface) :
		m_SurfacePtr{ pSurface },
		m_SurfacePixelsPtr{ static_cast<uint32_t*>(pSurface->pixels) }
	{
	}

	Texture::~Texture()
	{
		if (m_SurfacePtr)
		{
			SDL_FreeSurface(m_SurfacePtr);
			m_SurfacePtr = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& fileName)
	{
		const std::string path{ std::format("{}{}",RESOURCES_PATH,fileName) };

		SDL_Surface* loadedSurfacePtr = IMG_Load(path.c_str());

		if (loadedSurfacePtr == nullptr)
		{
			std::cout << "Texture Can't be loaded: " << path.c_str() << std::endl;
			return nullptr;
		}

		//if (loadedSurfacePtr->format->BitsPerPixel != 8)
		//{
		//	std::cout << "Only 8-bit textures supported: " << path.c_str() << std::endl;
		//	return nullptr;
		//}

		// Create and return a new Texture Object (using SDL_Surface)
		return new Texture(loadedSurfacePtr);
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		float u{ uv.x };
		float v{ uv.y };

		// Handle uv wrapping
		if constexpr (true)
		{
			while (u > 1.0f)
				u -= 1.0f;

			while (u < 0.0f)
				u += 1.0f;

			while (v > 1.0f)
				v -= 1.0f;

			while (v < 0.0f)
				v += 1.0f;
		}
		else
		{
			u = std::fmodf(u, 1.0f);
			v = std::fmodf(v, 1.0f);

			// Ensure the result is positive
			u = (u < 0.0f) ? (u + 1.0f) : u;
			v = (v < 0.0f) ? (v + 1.0f) : v;
		}

		constexpr bool useBilinearInterpolation = true;

		if (useBilinearInterpolation)
		{
			// Convert UV coordinates to texel coordinates
			const float texelXf = u * static_cast<float>(m_SurfacePtr->w - 1);
			const float texelYf = v * static_cast<float>(m_SurfacePtr->h - 1);

			// Get the integer part and fractional part of texel coordinates
			const int texelX = static_cast<int>(texelXf);
			const int texelY = static_cast<int>(texelYf);
			const float fracX = texelXf - texelX;
			const float fracY = texelYf - texelY;

			// Sample colors from four neighboring texels
			const ColorRGB c00 = GetTexelColor(texelX, texelY);
			const ColorRGB c10 = GetTexelColor(texelX + 1, texelY);
			const ColorRGB c01 = GetTexelColor(texelX, texelY + 1);
			const ColorRGB c11 = GetTexelColor(texelX + 1, texelY + 1);

			// Bilinear interpolation
			const ColorRGB interpolatedColor =
				Lerp(
					Lerp(c00, c10, fracX),
					Lerp(c01, c11, fracX),
					fracY
				);

			return interpolatedColor;
		}
		else
		{
			// Convert UV coordinates to texel coordinates
			const float texelXf = u * static_cast<float>(m_SurfacePtr->w - 1);
			const float texelYf = v * static_cast<float>(m_SurfacePtr->h - 1);

			// Get the integer part and fractional part of texel coordinates
			const int texelX = static_cast<int>(texelXf);
			const int texelY = static_cast<int>(texelYf);
			return GetTexelColor(texelX, texelY);
		}
	}


	ColorRGB Texture::GetTexelColor(int x, int y) const
	{
		// Get the pixel value at the texel coordinates
		const uint32_t pixel = m_SurfacePixelsPtr[y * m_SurfacePtr->w + x];

		Uint8 red{};
		Uint8 green{};
		Uint8 blue{};

		SDL_GetRGB(pixel, m_SurfacePtr->format, &red, &green, &blue);

		// Return the sampled color
		return
		{
			static_cast<float>(red) / 255.0f,
			static_cast<float>(green) / 255.0f,
			static_cast<float>(blue) / 255.0f,
		};
	}
}