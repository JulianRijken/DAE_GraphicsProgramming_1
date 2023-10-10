//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <algorithm>
#include <execution>
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

	m_XVals.reserve(m_Width);
	for (uint16_t x{}; x < m_Width; ++x)
		m_XVals.push_back(x);
	
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

	// We run a for_each for each of the x pixels, this will be distributed over all cpu threads
	std::for_each(std::execution::par, m_XVals.begin(), m_XVals.end(), [this, camera,multiplierXValue,multiplierYValue,fieldOfViewTimesAspect, cameraToWorld,scenePtr,lights,materials](const uint16_t pixelX)
		{
			Ray viewRay{ camera.origin };

			for (int pixelY{}; pixelY < m_Height; ++pixelY)
			{
				Vector3 rayDirection
				{
					((static_cast<float>(pixelX) + 0.5f) * multiplierXValue - 1.0f) * fieldOfViewTimesAspect,
					(1.0f - (static_cast<float>(pixelY) + 0.5f) * multiplierYValue) * camera.fovValue,
					1
				};

				// Get view direction
				viewRay.direction = cameraToWorld.TransformVector(rayDirection.Normalized());
				const Vector3 v{ viewRay.direction * -1.0f };

				HitRecord closestHit{};
				scenePtr->GetClosestHit(viewRay, closestHit);


				ColorRGB finalColor{};
				const Vector3 fromPoint{ closestHit.point + closestHit.normal * SHADOW_NORMAL_OFFSET };


				if (closestHit.didHit)
				{
					for (const Light& light : lights)
					{
						// Setup light ray
						const Vector3 hitToLightDirection{ light.origin - fromPoint };
						const float distance{ hitToLightDirection.Magnitude() };
						Ray hitToLightRay{ fromPoint,hitToLightDirection.Normalized() };
						hitToLightRay.max = distance;

						if (!m_ShadowsEnabled || !scenePtr->DoesHit(hitToLightRay))
						{
							const Vector3 l = (light.origin - closestHit.point).Normalized();
							const float cosineLaw = std::max(0.0f, Vector3::Dot(closestHit.normal, l));

							switch (m_CurrentLightMode)
							{
								case LightMode::ObservedArea:
									finalColor += ColorRGB(1, 1, 1) * cosineLaw;

									break;
								case LightMode::Radiance:
									finalColor += LightUtils::GetRadiance(light, closestHit.point);
									break;

								case LightMode::RadianceAndObservedArea:
									finalColor += LightUtils::GetRadiance(light, closestHit.point) * cosineLaw;

									break;
								case LightMode::BRDF:
									finalColor += materials[closestHit.materialIndex]->Shade(closestHit, l, v);

									break;
								case LightMode::Combined:
									finalColor +=
										LightUtils::GetRadiance(light, closestHit.point) *
										materials[closestHit.materialIndex]->Shade(closestHit, l, v) *
										cosineLaw;
									break;
							}

						}
					}

				}

				finalColor.MaxToOne();

				m_pBufferPixels[pixelX + pixelY * m_Width] = SDL_MapRGB(m_pBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		});

	
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
