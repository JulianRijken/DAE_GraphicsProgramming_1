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
		ColorRGB Sample(const Vector2& uv) const;

	private:
		Texture(SDL_Surface* pSurface);

		ColorRGB GetTexelColor(int x, int y) const;

		SDL_Surface* m_SurfacePtr{ nullptr };
		uint32_t* m_SurfacePixelsPtr{ nullptr };
	};
}