#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
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

		void Update(const Timer& timer);
		void Render() const;

		bool SaveBufferToImage() const;

	private:

		void World_to_Screen(const std::vector<Vertex>& verticesIn, std::vector<Vertex>& verticesOut) const;

		SDL_Window* m_WindowPtr{};

		SDL_Surface* m_FrontBufferPtr{ nullptr };
		SDL_Surface* m_BackBufferPtr{ nullptr };
		uint32_t* m_BackBufferPixelsPtr{};
		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_ScreenWidth{};
		int m_ScreenHeight{};
		float m_AspectRatio{};

	};
}
