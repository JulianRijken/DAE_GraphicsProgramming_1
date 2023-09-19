//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <SDL_timer.h>

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
    m_pWindow(pWindow),
    m_pRenderer(SDL_GetRenderer(pWindow))
{
    SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

    m_pTexture = SDL_CreateTexture(m_pRenderer,
            SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
            m_Width, m_Height);

    m_pTexturePixels = new uint32_t[m_Width * m_Height];
}

Renderer::~Renderer()
{
    delete[] m_pTexturePixels;
}

void Renderer::Render(Scene* pScene) const
{
    Camera& camera = pScene->GetCamera();
    auto& materials = pScene->GetMaterials();
    auto& lights = pScene->GetLights();

    for (int px = 0; px < m_Width; ++px)
    {
        for (int py{}; py < m_Height; ++py)
        {
            const float aspectRatio{ float(m_Width) / m_Height };
            float rad{ 50.f };
            float dist{ 100.f } ;

            ColorRGB finalColor{ 0, 0, 0 };

            // TODO: I wish to understand this
            float screenSpaceX{ (2.f * ((px + .5f) / m_Width) - 1.f) * aspectRatio };
            float screenSpaceY{ (1.f - 2.f * ((py + .5f) / m_Height)) };

            Vector3 rayDirection{ screenSpaceX, screenSpaceY, 1 };
            rayDirection.Normalize();

            Ray viewRay{ { 0, 0, 0 }, rayDirection };

            HitRecord closestHit{};

            Sphere testSphere{ { 0.f, 0, dist }, rad, 0 };
            GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);

            if(closestHit.didHit)
            {
                const float scaled_t = (closestHit.t - 50.f) / 40.f;
                finalColor = { scaled_t, scaled_t, scaled_t };
            }

            //Update Color in Buffer
            finalColor.MaxToOne();

            m_pTexturePixels[px + (py * m_Width)] = SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888),
                static_cast<uint8_t>(finalColor.r * 255),
                static_cast<uint8_t>(finalColor.g * 255),
                static_cast<uint8_t>(finalColor.b * 255));
        }
    }

    //@END
    //Update SDL Texture
    SDL_UpdateTexture(m_pTexture, nullptr, m_pTexturePixels, m_Width * sizeof (Uint32));
    SDL_RenderCopy(m_pRenderer, m_pTexture, nullptr, nullptr);
    SDL_RenderPresent(m_pRenderer);
}

bool Renderer::SaveBufferToImage() const
{
    SDL_Surface* pScreenshot = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    SDL_RenderReadPixels(m_pRenderer, nullptr, SDL_PIXELFORMAT_ARGB8888, pScreenshot->pixels, pScreenshot->pitch);
    bool ret = SDL_SaveBMP(pScreenshot, "RayTracing_Buffer.bmp");

    SDL_FreeSurface(pScreenshot);
    return ret;
}
