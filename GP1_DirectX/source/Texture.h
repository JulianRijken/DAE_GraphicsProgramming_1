#pragma once

#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();

		static Texture* LoadFromFile(const std::string& fileName);

	private:
		Texture(SDL_Surface* pSurface);

		SDL_Surface* m_SurfacePtr{ nullptr };
		uint32_t* m_SurfacePixelsPtr{ nullptr };
	};
}