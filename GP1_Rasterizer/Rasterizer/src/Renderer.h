#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

namespace dae
{
	class Texture;
}

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	enum class DebugRenderMode
	{
		FinalColor,
		BiometricCoordinate,
		DepthBuffer
	};


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

		void CycleDebugMode();

		bool SaveBufferToImage() const;

	private:

		void World_to_Screen(const std::vector<Vertex>& verticesIn, std::vector<Vertex>& verticesOut) const;

		inline void RenderMesh(const Mesh& mesh) const;
		inline void RenderTriangle(const Triangle& triangle) const;

		void MakeMeshes();

		SDL_Window* m_WindowPtr{};

		SDL_Surface* m_FrontBufferPtr{ nullptr };
		SDL_Surface* m_BackBufferPtr{ nullptr };
		uint32_t* m_BackBufferPixelsPtr{};
		float* m_pDepthBufferPixels{};

		Camera m_Camera{};
		DebugRenderMode m_RenderMode{};

		Texture* m_texture1{};
		Texture* m_texture1Opacity{};
		Texture* m_texture2{};

		std::vector<Mesh> m_WorldMeshes{};

		int m_ScreenWidth{};
		int m_ScreenHeight{};
		float m_AspectRatio{};

	};
}
