#pragma once

#include <cstdint>

#include "Vector3.h"

struct SDL_Window;
struct SDL_Surface;
struct Vector3;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* scenePtr) const;
		bool SaveBufferToImage() const;
		void ToggleShadows();
		void CycleLightMode();

	private:


		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		enum class LightMode
		{
			ObservedArea,
			Radiance,
			RadianceAndObservedArea,
			BRDF,
			Combined,
			COUNT
		};

		LightMode m_CurrentLightMode{ LightMode::Combined };
		bool m_ShadowsEnabled{ false };

	};
}
