//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <iostream>

#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* scenePtr) const
{
	Camera& camera = scenePtr->GetCamera();
	auto& materials = scenePtr->GetMaterials();
	auto& lights = scenePtr->GetLights();

	const float widthFloat{ static_cast<float>(m_Width) };
	const float heightFloat{ static_cast<float>(m_Height) };
	const float multiplierXValue{ 2.0f / widthFloat };
	const float multiplierYValue{ 2.0f / heightFloat };
	const float aspectRatio = widthFloat / heightFloat;
	const float fieldOfViewTimesAspect{ aspectRatio * camera.fovValue };

	const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

	Vector3 rayDirection{0,0,1};
	Ray viewRay{camera.origin};
	ColorRGB finalColor{};

	for (float pixelX{ 0.5f }; pixelX < widthFloat; ++pixelX)
	{
		rayDirection.x = (pixelX * multiplierXValue - 1.0f) * fieldOfViewTimesAspect;

		for (float pixelY{ 0.5f }; pixelY < heightFloat; ++pixelY)
		{
			rayDirection.y = (1.0f - pixelY * multiplierYValue) * camera.fovValue;

			// Get view direction
			viewRay.direction = cameraToWorld.TransformVector(rayDirection.Normalized());

			HitRecord closestHit{};
			scenePtr->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				float lightStrength{1.0f};

				if (m_ShadowsEnabled)
				{
					for (const Light& light : lights)
					{
						Vector3 fromPoint{ closestHit.point + closestHit.normal * 0.001f };
						Vector3 toPoint{ light.origin };

						Vector3 directionVector{ toPoint - fromPoint };
						const float distance{ directionVector.Magnitude()};

						Ray lightRay{fromPoint,directionVector.Normalized()};
						lightRay.max = distance;

						HitRecord lightHit{};
						scenePtr->GetClosestHit(lightRay, lightHit);

				
						if (lightHit.didHit)
							lightStrength = 0.5f;
						else
							lightStrength = 1.0f;
					}
				}

				finalColor = materials[closestHit.materialIndex]->Shade() * lightStrength;
			}
			else
			{
				finalColor = ColorRGB(0,0,0);
			}

			finalColor.MaxToOne();

			m_pBufferPixels[static_cast<int>(pixelX) + (static_cast<int>(pixelY) * m_Width)] = SDL_MapRGB(m_pBuffer->format,
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

void Renderer::ToggleShadows()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}

void Renderer::CycleLightMode()
{
	int current{ static_cast<int>(m_CurrentLightMode) };
	current++;

	if (current >= static_cast<int>(LightMode::COUNT))
		current = 0;

	m_CurrentLightMode = static_cast<LightMode>(current);
}
