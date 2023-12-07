#include "pch.h"
#include "Renderer.h"

#include "Mesh.h"


namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_WindowWidth, &m_WindowHeight);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		const std::vector<ModelVertex> vertices
		{
			{{ 0.0f, 0.5f, 0.5f}, {1.0f,0.0f,0.0f}},
			{{ 0.5f,-0.5f, 0.5f}, {0.0f,0.0f,1.0f}},
			{{-0.5f,-0.5f, 0.5f}, {0.0f,1.0f,0.0f}},
		};

		const std::vector<uint32_t> indices{ 0,1,2 };

		testMeshPtr = new Mesh(m_DevicePtr, vertices, indices);
	}

	Renderer::~Renderer()
	{
		m_RenderTargetViewPtr->Release();
		m_RenderTargetBufferPtr->Release();

		m_DepthStencilViewPtr->Release();
		m_DepthStencilBufferPtr->Release();

		m_SwapChainPtr->Release();

		if(m_DeviceContextPtr)
		{
			m_DeviceContextPtr->ClearState();
			m_DeviceContextPtr->Flush();
			m_DeviceContextPtr->Release();
		}

		m_DevicePtr->Release();

		delete testMeshPtr;
	};

	void Renderer::Update(const Timer* pTimer)
	{

	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		// Clear RTV & DSV
		m_DeviceContextPtr->ClearRenderTargetView(m_RenderTargetViewPtr, SCREEN_CLEAR_COLOR);
		m_DeviceContextPtr->ClearDepthStencilView(m_DepthStencilViewPtr, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,0.0f);

		// Render



		// Present Back buffer (Swap)
		m_SwapChainPtr->Present(0, 0);
	}

	HRESULT Renderer::InitializeDirectX()
	{
		// Define check and result for readability
		#define CHECK(result) if (FAILED(result)) return result;
		HRESULT result{};



		// Setup flags for device and context
		const D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;

		// When in debug mode include the device debug flag
		#if defined(DEBUG) or defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;std::cout << "Release code" << std::endl;
		#endif

		// Create device and context
		result = D3D11CreateDevice
		(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			&featureLevel,
			1,
			D3D11_SDK_VERSION,
			&m_DevicePtr,
			nullptr,
			&m_DeviceContextPtr
		);
		CHECK(result)



		// Create DXGI Factory
		IDXGIFactory1* dxgiFactoryPtr{ nullptr };
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactoryPtr));
		CHECK(result)



		// Create swap chain
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_WindowWidth;
		swapChainDesc.BufferDesc.Height = m_WindowHeight;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// Get the handle form the sdl back buffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_GetVersion(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		result = dxgiFactoryPtr->CreateSwapChain(m_DevicePtr, &swapChainDesc, &m_SwapChainPtr);
		CHECK(result)

		// Todo Not sure if this one can be released here ask teacher or make sure
		dxgiFactoryPtr->Release();

		// Create Depth Stencil
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_WindowWidth;
		depthStencilDesc.Height = m_WindowHeight;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		result = m_DevicePtr->CreateTexture2D(&depthStencilDesc, nullptr, &m_DepthStencilBufferPtr);
		CHECK(result)

		// Create Depth Stencil View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_DevicePtr->CreateDepthStencilView(m_DepthStencilBufferPtr, &depthStencilViewDesc, &m_DepthStencilViewPtr);
		CHECK(result)


		// Create Render Target and 
		result = m_SwapChainPtr->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_RenderTargetBufferPtr));
		CHECK(result)


		// Create Render Target View
		result = m_DevicePtr->CreateRenderTargetView(m_RenderTargetBufferPtr, nullptr,&m_RenderTargetViewPtr); 
		CHECK(result)

		// Bind RTV (Render Target View) and DSV (Depth Stencil View)
		m_DeviceContextPtr->OMSetRenderTargets(1, &m_RenderTargetViewPtr, m_DepthStencilViewPtr);

		// Setup viewport
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_WindowWidth);
		viewport.Height = static_cast<float>(m_WindowHeight);
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_DeviceContextPtr->RSSetViewports(1, &viewport);

		return S_OK;
	}
}
