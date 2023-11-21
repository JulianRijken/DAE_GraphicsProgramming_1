#include "Texture.h"

#include <algorithm>
#include <iostream>
#include <SDL_image.h>

#include "Vector2.h"

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

		if (not loadedSurfacePtr->format->BitsPerPixel == 8)
		{
			std::cout << "Only 8-bit textures supported: " << path.c_str() << std::endl;
			return nullptr;
		}

		// Create and return a new Texture Object (using SDL_Surface)
		return new Texture(loadedSurfacePtr);
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		return Sample(uv.x, uv.y);
	}

	ColorRGB Texture::Sample(float u,float v) const
	{
		// Ensure that the UV coordinates are within the valid range [0, 1]
		//const float u{ std::ranges::clamp(uv.x, 0.0f, 1.0f) };
		//const float v{ std::ranges::clamp(uv.y, 0.0f, 1.0f) };

		// Not sure if clamping or returning if out is better
		//if (u > 1.0f or u < 0.0f or v > 1.0f or v < 0.0f)
		//	return colors::Red;


		constexpr bool USE_WHILE{ true };
		
		if(USE_WHILE)
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

		
		// Convert UV coordinates to texel coordinates
		const int texelX = static_cast<int>(u * m_SurfacePtr->w);
		const int texelY = static_cast<int>(v * m_SurfacePtr->h);

		// Get the pixel value at the texel coordinates
		const uint32_t pixel = m_SurfacePixelsPtr[texelY * m_SurfacePtr->w + texelX];

		Uint8 red{};
		Uint8 green{};
		Uint8 blue{};

		SDL_GetRGB(pixel,m_SurfacePtr->format,&red, &green, &blue);

		// Return the sampled color
		return
		{
			static_cast<float>(red) / 255.0f,
			static_cast<float>(green) / 255.0f,
			static_cast<float>(blue) / 255.0f,
		};
	}
}