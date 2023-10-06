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


			ColorRGB finalColor{};

			if (closestHit.didHit)
			{
				for (const Light& light : lights)
				{
					// Setup light ray
					const Vector3 fromPoint{ closestHit.point + closestHit.normal * 0.001f };
					const Vector3 hitToLightDirection{ light.origin - fromPoint };
					const float distance{ hitToLightDirection.Magnitude() };
					Ray hitToLightRay{ fromPoint,hitToLightDirection.Normalized() };
					hitToLightRay.max = distance;

					if (!(scenePtr->DoesHit(hitToLightRay) && m_ShadowsEnabled))
					{
						Vector3 l = (light.origin - closestHit.point).Normalized();
						Vector3 v{ viewRay.direction.Normalized() * -1.0f };

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

//bool Renderer::IsLightOccluded(const Scene* scenePtr, const Vector3& lightOrigin, const Vector3& hitOrigin)
//{
//
//}

