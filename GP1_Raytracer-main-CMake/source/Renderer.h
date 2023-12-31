#pragma once

#include <cstdint>
#include <vector>

struct SDL_Window;
struct SDL_Surface;

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

		void Render(Scene* scenePtr);
		bool SaveBufferToImage() const;
		void ToggleShadows();
		void CycleLightMode();

	private:


		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		const float SHADOW_NORMAL_OFFSET{ 0.001f };

		std::vector<uint16_t> m_YVals;

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
