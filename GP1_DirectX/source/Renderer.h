#pragma once
#include <map>

class Mesh;
struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Camera;

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
		LightRadiance,
		COUNT
	};

	const std::map<DebugRenderMode, std::string> RENDER_MODE_NAMES
	{
		{DebugRenderMode::Diffuse,"Diffuse Color"},

		{DebugRenderMode::ObservedArea,"Observed_Area"},
		{DebugRenderMode::DiffuseOA,"Diffuse_OA"},
		{DebugRenderMode::SpecularOA,"Specular_OA"},
		{DebugRenderMode::Combined,"Combined"},

		{DebugRenderMode::UVColor,"UV_Color"},
		{DebugRenderMode::Weights,"Weights"},
		{DebugRenderMode::DepthBuffer,"Depth_Buffer"},
		{DebugRenderMode::MaterialIndex,"Material_Index"},
		{DebugRenderMode::Opacity,"Opacity"},
		{DebugRenderMode::LightRadiance,"Light_Radiance"},
	};

	class Renderer final
	{
	public:
		Renderer(Camera* cameraPtr, SDL_Window* windowPtr);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer& timer);
		void Render() const;

		void SetRenderMode(DebugRenderMode mode);
		void CycleRenderMode();

		bool SaveBufferToImage() const;

	private:
		SDL_Window* m_pWindow{};

		int m_WindowWidth{};
		int m_WindowHeight{};

		bool m_IsInitialized{ false };

		Mesh* testMeshPtr;
		Camera* m_CameraPtr;
		DebugRenderMode m_RenderMode;

		inline static constexpr float SCREEN_CLEAR_COLOR[4]{ 0.0f,0.0f,0.3f,1.0f };

		//DIRECTX
		HRESULT InitializeDirectX();

		ID3D11Device*			m_DevicePtr{};
		ID3D11DeviceContext*	m_DeviceContextPtr{};
		IDXGISwapChain*			m_SwapChainPtr{};
		ID3D11Texture2D*		m_DepthStencilBufferPtr{};
		ID3D11DepthStencilView* m_DepthStencilViewPtr{};
		ID3D11Resource*			m_RenderTargetBufferPtr{};
		ID3D11RenderTargetView* m_RenderTargetViewPtr{};

	};
}
