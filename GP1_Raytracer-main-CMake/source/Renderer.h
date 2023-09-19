#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace dae
{
    class Scene;

    class Renderer final
    {
    public:
        Renderer(SDL_Window* pWindow);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer(Renderer&&) noexcept = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer& operator=(Renderer&&) noexcept = delete;

        void Render(Scene* pScene) const;
        bool SaveBufferToImage() const;

    private:
        SDL_Window* m_pWindow{};
        SDL_Renderer* m_pRenderer{};

        SDL_Texture* m_pTexture{};
        uint32_t* m_pTexturePixels{};

        int m_Width{};
        int m_Height{};
    };
}
