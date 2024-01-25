#pragma once

#include <SDL_surface.h>
#include <string>

namespace dae
{
	struct Vector2;

	class Texture final
	{
	public:
		~Texture();

		// Returns nullptr if texture can't be loaded
		static Texture* LoadFromFile(ID3D11Device* devicePtr, const std::string& fileName);

		[[nodiscard]] ID3D11ShaderResourceView* GetShaderResource() const
		{
			return m_ShaderResourcePtr;
		}

		static void LoadDefaultTextures(ID3D11Device* devicePtr)
		{
			delete g_DefaultBlack;
			delete g_DefaultNormal;
			delete g_DefaultWhite;
			
			g_DefaultBlack = LoadFromFile(devicePtr,"defaultBlack.png");
			g_DefaultNormal = LoadFromFile(devicePtr,"defaultNormal.png");
			g_DefaultWhite = LoadFromFile(devicePtr,"defaultWhite.png");
		}

		static void UnloadDefaultTextures()
		{
			delete g_DefaultBlack;
			delete g_DefaultNormal;
			delete g_DefaultWhite;
			
			g_DefaultBlack = nullptr;
			g_DefaultNormal = nullptr;
			g_DefaultWhite = nullptr;
		}

		static Texture* Black() { return g_DefaultBlack; }
		static Texture* Normal() { return g_DefaultNormal; }
		static Texture* White() { return g_DefaultWhite; }
	
	private:
		
		inline static Texture* g_DefaultBlack{nullptr};
		inline static Texture* g_DefaultNormal{nullptr};
		inline static Texture* g_DefaultWhite{nullptr};

		
		Texture(ID3D11Device* devicePtr, const SDL_Surface* sdlSurface);

		ID3D11Texture2D* m_ResourcePtr;
		ID3D11ShaderResourceView* m_ShaderResourcePtr;
	};
}