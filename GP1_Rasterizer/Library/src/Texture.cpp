#include "Texture.h"

#include <algorithm>
#include <iostream>

#include "Vector2.h"
#include <SDL_image.h>

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

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		SDL_Surface* loadedSurfacePtr = IMG_Load(path.c_str());

		if (loadedSurfacePtr == nullptr)
		{
			std::cout << "Texture Can't be loaded: " << path.c_str() << std::endl;
			return nullptr;
		}

		// Create and return a new Texture Object (using SDL_Surface)
		return new Texture(loadedSurfacePtr);
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		// Ensure that the UV coordinates are within the valid range [0, 1]
		//const float u{ std::ranges::clamp(uv.x, 0.0f, 1.0f) };
		//const float v{ std::ranges::clamp(uv.y, 0.0f, 1.0f) };

		// Not sure if clamping or returning if out is better
		if (uv.x > 1.0f or uv.x < 0.0f or uv.y > 1.0f or uv.y < 0.0f)
			return colors::Black;

		// Convert UV coordinates to texel coordinates
		const int texelX = static_cast<int>(uv.x * m_SurfacePtr->w);
		const int texelY = static_cast<int>(uv.y * m_SurfacePtr->h);

		// Get the pixel value at the texel coordinates
		const uint32_t pixel = m_SurfacePixelsPtr[texelY * m_SurfacePtr->w + texelX];

		// Extract individual color channels (assuming 8 bits per channel)
		const uint8_t red = (pixel & 0xFF0000) >> 16;
		const uint8_t green = (pixel & 0x00FF00) >> 8;
		const uint8_t blue = pixel & 0x0000FF;

		// Return the sampled color
		return
		{
			static_cast<float>(blue) / 255.0f,
			static_cast<float>(green) / 255.0f,
			static_cast<float>(red) / 255.0f,
		};
	}
}