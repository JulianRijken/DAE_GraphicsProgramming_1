#include "pch.h"

#include "Texture.h"

#include <assert.h>
#include <format>
#include <iostream>
#include <SDL_image.h>

#include "GlobalSettings.h"

namespace dae
{
	Texture::Texture(ID3D11Device* devicePtr, const SDL_Surface* sdlSurface)
	{
		// Used for checks
		HRESULT result{};

		// Use SDL surface to create the DX11 Texture
		const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = sdlSurface->w;
		desc.Height = sdlSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = sdlSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(sdlSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(sdlSurface->h * sdlSurface->pitch);

		result = devicePtr->CreateTexture2D(&desc, &initData, &m_ResourcePtr);

		if (FAILED(result))
			assert(true && "Failed to create texture 2D using DX11");



		// Creating shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		result = devicePtr->CreateShaderResourceView(m_ResourcePtr, &SRVDesc, &m_ShaderResourcePtr);

		if (FAILED(result))
			assert(true && "Failed to create shader resource view");


	}

	Texture::~Texture()
	{
		m_ResourcePtr->Release();
		m_ResourcePtr = nullptr;

		m_ShaderResourcePtr->Release();
		m_ShaderResourcePtr = nullptr;
	}

	Texture* Texture::LoadFromFile(ID3D11Device* devicePtr, const std::string& fileName)
	{
		const std::string path{ std::format("{}{}",RESOURCES_PATH,fileName) };

		SDL_Surface* loadedSurfacePtr = IMG_Load(path.c_str());

		if (loadedSurfacePtr == nullptr)
		{
			std::cout << "Texture Can't be loaded: " << path.c_str() << std::endl;
			return nullptr;
		}

		Texture* newTexturePtr = new Texture(devicePtr, loadedSurfacePtr);

		// Clear SDL_Surface as we use DirectX for rendering
		SDL_FreeSurface(loadedSurfacePtr);
		loadedSurfacePtr = nullptr;

		return newTexturePtr;

	}
}