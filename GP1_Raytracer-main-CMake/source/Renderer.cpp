//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math/Math.h"
#include "Math/Matrix.h"
#include "Misc/Material.h"
#include "Misc/Scene.h"
#include "Misc/Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

    float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };

    Vector3 origin{ 0, 0, 0 };
    Sphere testSphere{ {0.f, 0.f, 100.f}, 50.f, 0 };

    #pragma omp parallel for schedule(guided) default(none) shared(origin, testSphere, aspectRatio, pScene, materials)
    for (int px{}; px < m_Width; ++px)
	{
        HitRecord closestHit{};

        for (int py{}; py < m_Height; ++py)
		{
            float x = ((2.f * (static_cast<float>(px) + 0.5f)) / static_cast<float>(m_Width) - 1.f) * aspectRatio;
            float y = 1.f - 2.f * (static_cast<float>(py) + 0.5f) / static_cast<float>(m_Height);

            Vector3 rayDirection{
                x,
                y,
                1.f
            };

            rayDirection.Normalize();

			Ray hitRay{origin, rayDirection};

            pScene->GetClosestHit(hitRay, closestHit);

			ColorRGB finalColor{ 0.f, 0.f, 0.f };

            if (closestHit.didHit) {
                const float scaled_t = (closestHit.t - 50.f) / 40.f;
                finalColor = scaled_t * materials[closestHit.materialIndex]->Shade();
//                finalColor = {scaled_t, scaled_t, scaled_t};
//                Vector3 normalColor{
//                    abs(closestHit.normal.x),
//                    abs(closestHit.normal.y),
//                    abs(closestHit.normal.z)
//                };

//                finalColor = {normalColor.x, normalColor.y, normalColor.z};
            }

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
