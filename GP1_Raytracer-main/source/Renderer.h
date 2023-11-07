#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct SDL_Window;
struct SDL_Surface;

constexpr int maxBounces{ 5 };

namespace dae
{
	class Scene;
	struct ColorRGB;

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
			Combined,
			ObservedArea,
			Radiance,
			BRDF,
			COUNT
		};

		inline static const std::map<int,std::string> LIGHT_MODE_NAMES
		{
			{static_cast<int>(LightMode::Combined),"Combined"},
			{static_cast<int>(LightMode::ObservedArea),"ObservedArea"},
			{static_cast<int>(LightMode::Radiance),"Radiance"},
			{static_cast<int>(LightMode::BRDF),"BRDF"},
		};

		LightMode m_CurrentLightMode{ LightMode::Combined };
		bool m_ShadowsEnabled{ true };
		int interlaceState{};
		int interlaceSpace{2};

	};
}
