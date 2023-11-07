//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <algorithm>
#include <execution>
#include <format>

#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#define MULTI
#define SWITCH
//#define REFLECT

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);

	m_YVals.reserve(m_Height);
	for (uint16_t y{}; y < m_Height; ++y)
		m_YVals.push_back(y);
}




void Renderer::Render(Scene* scenePtr)
{
	Camera& camera = scenePtr->GetCamera();
	const auto& materials = scenePtr->GetMaterials();
	const auto& lights = scenePtr->GetLights();

	const float widthFloat{ static_cast<float>(m_Width) };
	const float heightFloat{ static_cast<float>(m_Height) };
	const float multiplierXValue{ 2.0f / widthFloat };
	const float multiplierYValue{ 2.0f / heightFloat };
	const float aspectRatio = widthFloat / heightFloat;
	const float fieldOfViewTimesAspect{ aspectRatio * camera.fovValue };

	const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

#ifdef MULTI
	// We run a for_each for each of the y pixels, this will be distributed over all cpu threads
	std::for_each(std::execution::par, m_YVals.begin(), m_YVals.end(), [this, camera, multiplierXValue, multiplierYValue, fieldOfViewTimesAspect, cameraToWorld, scenePtr, lights, materials](const uint16_t pixelY)
		{
#else
	for (int pixelY{}; pixelY < m_Height; ++pixelY)
	{
#endif
		Vector3 rayDirection{ 0,0,1 };

		rayDirection.y = (1.0f - (static_cast<float>(pixelY) + 0.5f) * multiplierYValue) * camera.fovValue;

		for (int pixelX{}; pixelX < m_Width; ++pixelX)
		{
			rayDirection.x = ((static_cast<float>(pixelX) + 0.5f) * multiplierXValue - 1.0f) * fieldOfViewTimesAspect;

			//=====================FOR EVERY PIXEL===============================

			ColorRGB finalColor{};

			// Setup view ray
			Ray viewRay
			{
				camera.origin,
				cameraToWorld.TransformVector(rayDirection.Normalized())
			};

#ifdef REFLECT
			float colorLeft{ 1.0f };

			for (int bounceIndex = 0; bounceIndex <= maxBounces; ++bounceIndex)
			{
				//=====================FOR EVERY BOUNCE===============================
				if(colorLeft < EPSILON)
					break;
#endif
				HitRecord closestHit{};
				scenePtr->GetClosestHit(viewRay, closestHit);

				Material* hitMaterial{ materials[closestHit.materialIndex] };

#ifdef REFLECT
				// Get the current amount of color picked up 
				const float currentColor = colorLeft * hitMaterial->m_globalRoughness;

				// Remove the current color from color left
				colorLeft -= currentColor;
#endif
				const Vector3 v{ -viewRay.direction };
				const Vector3 hitPointWithOffset{ closestHit.point + closestHit.normal * SHADOW_NORMAL_OFFSET };

				if (closestHit.didHit)
				{
					for (const Light& light : lights)
					{
						const Vector3 lightToHitDirection{ hitPointWithOffset - light.origin };
						const float lightToHitDistance{ lightToHitDirection.Magnitude() };
						const Vector3 l = lightToHitDirection / lightToHitDistance;

						const Ray hitToLightRay{ light.origin, l,0.0f,lightToHitDistance };

						if (m_ShadowsEnabled && scenePtr->DoesHit(hitToLightRay))
							continue;

						const float cosineLaw = std::max(0.0f, Vector3::Dot(closestHit.normal, -l));


						switch (m_CurrentLightMode)
						{
						case LightMode::Combined:

							finalColor += LightUtils::GetRadiance(light, closestHit.point) *
								hitMaterial->Shade(closestHit, -l, v) *
								cosineLaw
#ifdef REFLECT
								* currentColor;
#else
								;
#endif
							break;
						case LightMode::ObservedArea:
							finalColor += ColorRGB(1, 1, 1) * cosineLaw;

							break;
						case LightMode::Radiance:
							finalColor += LightUtils::GetRadiance(light, closestHit.point);
							break;
						case LightMode::BRDF:
							finalColor += hitMaterial->Shade(closestHit, -l, v);

							break;

						}

					}
				}
#ifdef REFLECT

				// Bounce ray 
				viewRay.direction = Vector3::Reflect(viewRay.direction, closestHit.normal);
				viewRay.origin = closestHit.point;

				//=====================FOR EVERY BOUNCE===============================
			}
#endif


			finalColor.MaxToOne();

			m_pBufferPixels[pixelX + pixelY * m_Width] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));


			//=====================FOR EVERY PIXEL===============================
		}

#ifdef MULTI
	});
#else
			}
#endif

	
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

	std::cout << std::endl;
	std::cout << std::format("Current light mode {}", LIGHT_MODE_NAMES.at((int)m_CurrentLightMode)) << std::endl;
	std::cout << std::endl;
}
	