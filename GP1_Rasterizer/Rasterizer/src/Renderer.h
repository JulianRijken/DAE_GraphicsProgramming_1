#pragma once
#include <map>
#include <string>
#include <vector>
#include <DataTypes.h>


namespace dae
{
	struct Camera;
	class Timer;
	class Texture;
}

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	enum class DebugRenderMode
	{
		FinalColor,
		Color,
		Opacity,
		UVColor,
		BiometricCoordinate,
		DepthBuffer,
		MaterialIndex,
		COUNT
	};

	const std::map<DebugRenderMode, std::string> RENDER_MODE_NAMES
	{
		{DebugRenderMode::FinalColor,"Final Color"},
		{DebugRenderMode::Color,"Color"},
		{DebugRenderMode::Opacity,"Opacity"},
		{DebugRenderMode::UVColor,"UV Color"},
		{DebugRenderMode::BiometricCoordinate,"Biometric Coordinates"},
		{DebugRenderMode::DepthBuffer,"Depth Buffer"},
		{DebugRenderMode::MaterialIndex,"Material Index"},
	};


	class Renderer final
	{
	public:
		Renderer(Camera* camera, SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render() const;

		void CycleDebugMode(bool up);
		void SetRenderMode(DebugRenderMode mode);

		bool SaveBufferToImage() const;

	private:

		void World_to_Screen(const std::vector<Vertex>& verticesIn, std::vector<Vertex>& verticesOut) const;

		inline void RenderMesh(const Mesh& mesh) const;
		inline void RenderTriangle(const Triangle& triangle, const std::vector<Material*>& materialPtrs) const;

		void InitializeObjects();

		SDL_Window* m_WindowPtr{};

		SDL_Surface* m_FrontBufferPtr{ nullptr };
		SDL_Surface* m_BackBufferPtr{ nullptr };
		uint32_t* m_BackBufferPixelsPtr{};
		float* m_pDepthBufferPixels{};

		Camera* m_CameraPtr;
		DebugRenderMode m_RenderMode{DebugRenderMode::FinalColor};

		std::vector<Mesh> m_WorldMeshes{};
		std::vector<Material*> m_Materials{};

		int m_ScreenWidth{};
		int m_ScreenHeight{};
		float m_AspectRatio{};
	};
}
