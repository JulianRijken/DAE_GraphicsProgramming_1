#pragma once

class Mesh;
struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

	private:
		SDL_Window* m_pWindow{};

		int m_WindowWidth{};
		int m_WindowHeight{};

		bool m_IsInitialized{ false };

		Mesh* testMeshPtr;

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
