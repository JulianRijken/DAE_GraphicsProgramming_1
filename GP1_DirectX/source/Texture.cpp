#include "pch.h"

#include "Texture.h"

#include <format>
#include <iostream>
#include <SDL_image.h>

#include "Vector2.h"
#include "GlobalSettings.h"

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
}