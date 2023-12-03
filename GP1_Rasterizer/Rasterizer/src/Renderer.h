#pragma once
#include <map>
#include <string>
#include <vector>
#include <DataTypes.h>

#include "Mesh.h"

namespace dae
{
	struct Camera;
	class Timer;
	class Texture;
	class Mesh;
}

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	enum class DebugRenderMode
	{
		Diffuse,
		ObservedArea,
		DiffuseOA,
		SpecularOA,
		Combined,
		UVColor,
		Weights,
		DepthBuffer,
		MaterialIndex,
		Opacity,
		COUNT
	};

	const std::map<DebugRenderMode, std::string> RENDER_MODE_NAMES
	{
		{DebugRenderMode::Diffuse,"Diffuse Color"},
		{DebugRenderMode::ObservedArea,"Observed Area"},
		{DebugRenderMode::DiffuseOA,"Diffuse + OA"},
		{DebugRenderMode::SpecularOA,"Specular + OA"},
		{DebugRenderMode::Combined,"Combined"},
		{DebugRenderMode::UVColor,"UV Color"},
		{DebugRenderMode::Weights,"Weights"},
		{DebugRenderMode::DepthBuffer,"Depth Buffer"},
		{DebugRenderMode::MaterialIndex,"Material Index"},
		{DebugRenderMode::Opacity,"Opacity"},
	};


	const Vector3 LIGHT_DIRECTION{ Vector3{0.577f,-0.577f,0.577f}.Normalized() };



	class Renderer final
	{
	public:
		Renderer(Camera* camera, SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer& timer);
		void Render();

		void ToggleRotation();
		void ToggleNormalMap();
		void ToggleLinearDepth();
		void SetRenderMode(DebugRenderMode mode);
		void CycleRenderMode();

		bool SaveBufferToImage() const;

	private:

		void TransformMesh(Mesh& mesh) const;

		inline void RasterizeMesh(Mesh& mesh) const;
		inline void RasterizeTriangle(const Triangle& triangle, const std::vector<Material*>& materialPtrs) const;
		inline void ShadePixel       (const Triangle& triangle, const std::vector<Material*>& materialPtrs, const Vector3& weights, int pixelIndex, float nonLinearDepth) const;

		void InitializeSceneAssignment();
		void InitializeSceneCar();
		void InitializeSceneDiorama();

		SDL_Window* m_WindowPtr{};
		SDL_Surface* m_FrontBufferPtr{ nullptr };
		SDL_Surface* m_BackBufferPtr{ nullptr };
		uint32_t* m_BackBufferPixelsPtr{};
		float* m_pDepthBufferPixels{ nullptr };

		Camera* m_CameraPtr;
		DebugRenderMode m_RenderMode;

		std::vector<Mesh> m_WorldMeshes;
		std::map <std::string, Material* > m_MaterialPtrMap;
		Material* defaultMaterial;

		ColorRGB m_AmbientColor{ 0.025f,0.025f ,0.025f };

		int m_ScreenWidth;
		int m_ScreenHeight;

		bool m_HasToRotate;
		bool m_UseNormalMap;
		bool m_UseLinearDepth;

		float spinSpeed{ 0.5f };
	};
}
