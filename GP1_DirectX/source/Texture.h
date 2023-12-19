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

	private:
		Texture(ID3D11Device* devicePtr, const SDL_Surface* sdlSurface);

		ID3D11Texture2D* m_ResourcePtr;
		ID3D11ShaderResourceView* m_ShaderResourcePtr;
	};
}