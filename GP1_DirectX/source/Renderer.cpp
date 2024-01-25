#include "pch.h"
#include "Renderer.h"

#include <cassert>
#include <format>

#include "Camera.h"
#include "EffectBase.h"
#include "EffectOpaque.h"
#include "EffectPartialCoverage.h"
#include "GlobalSettings.h"
#include "Light.h"
#include "Mesh.h"
#include "Texture.h"
#include "Utils.h"


namespace dae
{

	Renderer::Renderer(Camera* cameraPtr, SDL_Window* windowPtr) :
		m_pWindow(windowPtr),
		m_CameraPtr(cameraPtr)
	{
		//Initialize Window
		SDL_GetWindowSize(windowPtr, &m_WindowWidth, &m_WindowHeight);

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

		Texture::LoadDefaultTextures(m_DevicePtr);

		m_OpaqueEffectPtr = new EffectOpaque(m_DevicePtr, OPAQUE_SHADER_EFFECT_FILE_PATH);
		m_PartialCoverageEffectPtr = new EffectPartialCoverage(m_DevicePtr, PARTIAL_COVERAGE_EFFECT_FILE_PATH);
		m_TestingEffectPtr = new EffectPartialCoverage(m_DevicePtr, TESTING_EFFECT_FILE_PATH);

		InitializeSceneAssignment();
		//InitializeSceneEnv();
		//InitializeSceneDiorama();
		//InitializeSceneCar();
		//InitializeSceneHallway88();
	}

	Renderer::~Renderer()
	{

		Texture::UnloadDefaultTextures();

		// Delete materials
		for (const std::pair<const std::string, Material*>& pair : m_MaterialPtrMap)
		{
			if (pair.second == nullptr)
				continue;

			delete pair.second->diffuse;
			delete pair.second->opacity;
			delete pair.second->normal;
			delete pair.second->specular;
			delete pair.second->gloss;
			delete pair.second;
		}

		for (const Mesh* worldMesh : m_WorldMeshes)
		{
			delete worldMesh;
		}

		delete m_PartialCoverageEffectPtr;
		delete m_OpaqueEffectPtr;
		delete m_TestingEffectPtr;

		m_RenderTargetViewPtr->Release();
		m_RenderTargetBufferPtr->Release();

		m_DepthStencilViewPtr->Release();
		m_DepthStencilBufferPtr->Release();

		m_SwapChainPtr->Release();

		if (m_DeviceContextPtr)
		{
			m_DeviceContextPtr->ClearState();
			m_DeviceContextPtr->Flush();
			m_DeviceContextPtr->Release();
		}

		m_DevicePtr->Release();
	};


	HRESULT Renderer::InitializeDirectX()
	{
		// Define check and result for readability
#define IS_VALID(result) if (FAILED(result)) return result;
		HRESULT result{};


		// Setup flags for device and context
		constexpr D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;

		// When in debug mode include the device debug flag
#if defined(DEBUG) or defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		std::cout << "Debug code" << std::endl;
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
		IS_VALID(result)



			// Create DXGI Factory
			IDXGIFactory1* dxgiFactoryPtr{ nullptr };
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactoryPtr));
		IS_VALID(result);



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
		IS_VALID(result);


		// RELEASE EARLY!
		dxgiFactoryPtr->Release();
		dxgiFactoryPtr = nullptr;

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
		IS_VALID(result);

		// Create Depth Stencil View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_DevicePtr->CreateDepthStencilView(m_DepthStencilBufferPtr, &depthStencilViewDesc, &m_DepthStencilViewPtr);
		IS_VALID(result);


		// Create Render Target and 
		result = m_SwapChainPtr->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_RenderTargetBufferPtr));
		IS_VALID(result);


		// Create Render Target View
		result = m_DevicePtr->CreateRenderTargetView(m_RenderTargetBufferPtr, nullptr, &m_RenderTargetViewPtr);
		IS_VALID(result);

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



	void Renderer::InitializeSceneAssignment()
	{
		m_CameraPtr->SetFovAngle(45);
		m_CameraPtr->SetPosition(Vector3{ 0,0.0f,-50.0f });
		m_CameraPtr->SetNearClipping(0.1f);
		m_CameraPtr->SetFarClipping(1000.0f);
		m_OrbitCameraDistance = 50.0f;

		//m_AmbientColor = { 0.03f,0.03f,0.03f };
		//m_OpaqueEffectPtr->SetLightDirection({ 0.577f, -0.577f, 0.577f });

		// Lights
		AddDirectionalLight({ 0.577f, -0.577f, 0.577f }, { 1,1,1 }, 1.0f);
		m_OpaqueEffectPtr->SetLights(m_WorldLights);

		m_OpaqueEffectPtr->SetUseNormalMap(true);
		m_OpaqueEffectPtr->SetSampleState(0);

		m_OpaqueEffectPtr->SetSpecularKs(1.0f);
		m_OpaqueEffectPtr->SetSampledPhongExponent(25.0f);
		m_OpaqueEffectPtr->SetDiffuseStrengthKd(7.0f);


		m_MaterialPtrMap.insert({ "bike",new Material {
		   Texture::LoadFromFile(m_DevicePtr,"vehicle_diffuse.png"),
		   nullptr,
		   Texture::LoadFromFile(m_DevicePtr,"vehicle_normal.png"),
		   Texture::LoadFromFile(m_DevicePtr,"vehicle_specular.png"),
		   Texture::LoadFromFile(m_DevicePtr,"vehicle_gloss.png"),
		} });

		AddMesh("vehicle.obj", m_OpaqueEffectPtr, { m_MaterialPtrMap["bike"] });

		m_MaterialPtrMap.insert({ "fire",new Material {
		   Texture::LoadFromFile(m_DevicePtr,"fireFX_diffuse.png"),
		} });

		AddMesh("fireFX.obj", m_PartialCoverageEffectPtr, { m_MaterialPtrMap["fire"] });
	}

	void Renderer::InitializeSceneEnv()
	{

		m_CameraPtr->SetFovAngle(45);
		m_CameraPtr->SetPosition(Vector3{ 0,0.0f,-50.0f });
		m_CameraPtr->SetNearClipping(0.1f);
		m_CameraPtr->SetFarClipping(1000.0f);
		m_OrbitCameraDistance = 50.0f;

		m_OpaqueEffectPtr->SetAmbientColor({ 0.2f,0.2f,0.2f });
		m_OpaqueEffectPtr->SetLightDirection({ 0.577f, -0.577f, 0.577f });
		m_OpaqueEffectPtr->SetSampleState(0);
		m_OpaqueEffectPtr->SetUseNormalMap(true);
		m_OpaqueEffectPtr->SetDiffuseStrengthKd(1.5f);
		m_OpaqueEffectPtr->SetSampledPhongExponent(10.0f);
		m_OpaqueEffectPtr->SetSpecularKs(0.5f);

		// m_MaterialPtrMap.insert({ "uvGrid",new Material {
		// 	Texture::LoadFromFile(m_DevicePtr,"uv_grid_2.png"),
		// } });

		m_MaterialPtrMap.insert({ "env",new Material {
			Texture::LoadFromFile(m_DevicePtr, "Env/PanelTrim_AO.png"),
			nullptr,
			Texture::LoadFromFile(m_DevicePtr, "Env/PanelTrim_Normal.png"),
		} });

		m_MaterialPtrMap.insert({ "envPanel",new Material {
			Texture::LoadFromFile(m_DevicePtr, "Env/M_PanelPillar_D.png"),
			nullptr,
			Texture::LoadFromFile(m_DevicePtr, "Env/M_PanelPillar_N.png"),
		} });

		Mesh* mesh = AddMesh("Env/MainPersistent_SM.obj", m_OpaqueEffectPtr, { m_MaterialPtrMap["envPanel"] });
		mesh->SetScale(Vector3{ 1,1,1 } *0.1f);

	}

	void Renderer::InitializeSceneHallway88()
	{
		m_CameraPtr->SetFovAngle(45);
		m_CameraPtr->SetPosition(Vector3{ 0,0.0f,-50.0f });
		m_CameraPtr->SetNearClipping(0.1f);
		m_CameraPtr->SetFarClipping(1000.0f);
		m_OrbitCameraDistance = 50.0f;

		m_OpaqueEffectPtr->SetAmbientColor({ 0.2f,0.2f,0.2f });
		m_OpaqueEffectPtr->SetSampleState(0);
		m_OpaqueEffectPtr->SetUseNormalMap(true);

		m_OpaqueEffectPtr->SetDiffuseStrengthKd(2.0f);
		m_OpaqueEffectPtr->SetSampledPhongExponent(2.0f);
		m_OpaqueEffectPtr->SetSpecularKs(0.1f);

		const std::vector<Mesh*> meshes = AddMeshParse("Hallway88.obj", "Hallway88.mtl", m_OpaqueEffectPtr, "Hallway88/");

		for (Mesh* mesh : meshes)
			mesh->SetScale(Vector3{ 1,1,1 } *0.1f);

		//AddDirectionalLight({ 0.577f, -0.577f, 0.577f }, { 1,1,1 }, 0.9f);
		AddDirectionalLight({ -0.577f, -0.577f, -0.577f }, { 1,1,1 }, 0.7f);
		m_OpaqueEffectPtr->SetLights(m_WorldLights);
	}

	void Renderer::InitializeSceneCar()
	{
		m_CameraPtr->SetFovAngle(15);

		m_CameraPtr->SetPosition(Vector3{ -53.4506f, 22.7297f, -118.892f });
		m_CameraPtr->SetPitch(-0.104893f);
		m_CameraPtr->SetYaw(-0.415f);
		m_CameraPtr->SetNearClipping(1);
		m_CameraPtr->SetFarClipping(500);

		m_OpaqueEffectPtr->SetLightDirection({ 0.577f, -0.577f, 0.577f });
		m_OpaqueEffectPtr->SetUseNormalMap(true);
		m_OpaqueEffectPtr->SetSampleState(0);

		m_OpaqueEffectPtr->SetDiffuseStrengthKd(1.8f);
		m_OpaqueEffectPtr->SetSampledPhongExponent(10.0f);
		m_OpaqueEffectPtr->SetSpecularKs(0.5f);


		const std::vector<Mesh*> carMesh = AddMeshParse("Car/Car.obj", "Car/Car.mtl", m_OpaqueEffectPtr);
		for (Mesh* mesh : carMesh)
		{
			mesh->SetScale(Vector3{ 1,1,1 } *15);
			mesh->SetYawRotation(60.0f * TO_RADIANS);
		}

		m_MaterialPtrMap.insert({ "uvGrid",new Material {
			Texture::LoadFromFile(m_DevicePtr,"uv_grid_2.png"),
		} });
		Mesh* backdrop = AddMesh("Backdrop.obj", m_OpaqueEffectPtr, { m_MaterialPtrMap["uvGrid"] });
		backdrop->SetScale(Vector3{ 2,1,1 } *11);


		// Lights
		AddDirectionalLight({ 0.577f, -0.577f, 0.577f }, { 1,1,1 }, 1.0f);
		// AddPointLight({ -30.0f, 60, -40.0f }, colors::White, 3000.0f); // Front left light
		// AddPointLight({ 40.0f, -10, -20.0f }, colors::White, 1500.0f); // Right bottom light
		// AddPointLight({ 10, 30, 10.0f }, colors::White, 500.0f); // Back light

		m_OpaqueEffectPtr->SetLights(m_WorldLights);
	}

	void Renderer::InitializeSceneDiorama()
	{
		//m_AmbientColor = { 0.001f,0.001f,0.001f };
		//m_ClearColor = { 10 };

		//AddDirectionalLight({ 0.577f, -0.577f, 0.577f }, { 0.8f,0.8f,1.0f }, 0.5f);
		m_OpaqueEffectPtr->SetLightDirection({ 0.577f, -0.577f, 0.577f });
		m_OpaqueEffectPtr->SetUseNormalMap(true);
		m_OpaqueEffectPtr->SetSampleState(0);

		m_OpaqueEffectPtr->SetDiffuseStrengthKd(3.0f);
		m_OpaqueEffectPtr->SetSampledPhongExponent(20.0f);
		m_OpaqueEffectPtr->SetSpecularKs(0.2f);

		//AddPointLight({ 135.7012f, 267.001f, -158.2f }, { 1.0f,0.9f,0.7f }, 10000.0f); // Fake sun
		//AddPointLight({ -65.8307f, 163.166f, 129.75f }, { 1.0f,0.9f,0.7f }, 1000.0f); // Fake sun Back
		//AddPointLight({ 118.023f, 70.663f, 103.019f }, colors::White, 2000.0f); // Car driving up
		//AddPointLight({ -103.528f, 100.0746f, 10.4314f }, { 1.0f,0.7f,0.3f }, 700.0f); // Garage light

		AddDirectionalLight({ 0.577f, -0.577f, 0.577f }, { 1,1,1 }, 1.0f);
		m_OpaqueEffectPtr->SetLights(m_WorldLights);

		m_CameraPtr->SetFovAngle(42);
		m_CameraPtr->SetPosition({ -167.749f, 158.555f, -359.077f });
		m_CameraPtr->SetPitch(-0.10f);
		m_CameraPtr->SetYaw(-0.40f);
		m_CameraPtr->SetNearClipping(1.0f);
		m_CameraPtr->SetFarClipping(1000.0f);

		const std::vector<Mesh*> diorama = AddMeshParse("Diorama/DioramaGP.obj", "Diorama/DioramaGP.mtl", m_OpaqueEffectPtr);
		for (Mesh* mesh : diorama)
		{
			mesh->SetScale(Vector3{ 1,1,1 } *15.0f);
		}
	}


	Mesh* Renderer::AddMesh(EffectBase* effect, const std::vector<VertexModel>& vertices, const std::vector<uint32_t>& indices, const std::vector<Material*>& materials)
	{
		Mesh* newMesh = new Mesh(m_DevicePtr, effect, vertices, indices, materials);
		m_WorldMeshes.push_back(newMesh);
		return newMesh;
	}

	Mesh* Renderer::AddMesh(const std::string& objName, EffectBase* effect, const std::vector<Material*>& materials)
	{
		Mesh* newMesh = new Mesh(m_DevicePtr, effect, objName, materials);
		m_WorldMeshes.push_back(newMesh);
		return newMesh;
	}

	std::vector<Mesh*> Renderer::AddMeshParse(const std::string& objName, const std::string& mtlName, EffectBase* effect, const std::string& pathPrefix)
	{
		std::map<std::string, Material*> parsedMaterials{};
		Utils::ParseMTL(m_DevicePtr, mtlName, parsedMaterials, pathPrefix);

		std::vector<Mesh*> meshes{};
		Utils::ParseObjAndCreateMeshes(m_DevicePtr, effect, objName, meshes, parsedMaterials, pathPrefix);

		m_WorldMeshes.insert(m_WorldMeshes.end(), meshes.begin(), meshes.end());

		return meshes;
	}

	void Renderer::AddPointLight(const Vector3& origin, const ColorRGB& color, float intensity)
	{
		m_WorldLights.emplace_back(Light(origin, {}, color, intensity, LightType::Point));
	}

	void Renderer::AddDirectionalLight(const Vector3& direction, const ColorRGB& color, float intensity)
	{
		m_WorldLights.emplace_back(Vector3{ 0,0,0 }, direction.Normalized(), color, intensity, LightType::Directional);
	}



	void Renderer::Update(const Timer& timer)
	{
		if (m_OrbitCamera)
		{
			const float angle = -timer.GetTotal() * PI * 2 * ROTATIONS_PER_SECOND;
			const Vector3 position = { std::cos(angle) * m_OrbitCameraDistance,m_OrbitCameraDistance / 5.0f,std::sin(angle) * m_OrbitCameraDistance };
			m_CameraPtr->SetPosition(position);

			m_CameraPtr->SetYaw(angle + PI / 2.0f);
			m_CameraPtr->SetPitch(-0.23f);
		}

		if (m_RotateMesh)
		{
			for (Mesh* mesh : m_WorldMeshes)
				mesh->AddYawRotation(PI * 2 * timer.GetElapsed() * ROTATIONS_PER_SECOND);
		}
	}

	void Renderer::Render() const
	{
		if (not m_IsInitialized)
			return;

		// Clear RTV & DSV
		m_DeviceContextPtr->ClearRenderTargetView(m_RenderTargetViewPtr, SCREEN_CLEAR_COLOR);
		m_DeviceContextPtr->ClearDepthStencilView(m_DepthStencilViewPtr, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_OpaqueEffectPtr->SetCameraOrigin(m_CameraPtr->GetOrigin());

		// Render
		// Oh my I hate this hack to toggle the fire so much!!! But it's quickly needed for the hand in :(
		int mesh{};
		for (const Mesh* worldMesh : m_WorldMeshes)
		{
			if (!(mesh == 1 && !m_ShowFire))
				worldMesh->Render(m_DeviceContextPtr, m_CameraPtr->GetViewProjectionMatrixPtr());

			mesh++;
		}

		// Present Back buffer (Swap)
		m_SwapChainPtr->Present(0, 0);
	}



	void Renderer::SetRenderMode(DebugRenderMode mode)
	{
		m_RenderMode = mode;

		std::cout << std::endl;
		std::cout << RENDER_MODE_NAMES.at(m_RenderMode).c_str() << std::endl;
		std::cout << std::endl;
	}

	void Renderer::CycleRenderMode()
	{
		int current{ static_cast<int>(m_RenderMode) };
		current++;

		if (current >= static_cast<int>(DebugRenderMode::COUNT))
			current = 0;

		SetRenderMode(static_cast<DebugRenderMode>(current));
	}

	void Renderer::ToggleCameraOrbit()
	{
		m_OrbitCamera = !m_OrbitCamera;
	}
	void Renderer::ToggleMeshRotation()
	{
		m_RotateMesh = !m_RotateMesh;
	}

	void Renderer::CycleSampleState()
	{
		m_OpaqueEffectPtr->SetSampleState(++m_SampleState % 3);
	}
	void Renderer::ToggleUseNormalMap()
	{
		m_UseNormalMap = !m_UseNormalMap;

		if (m_UseNormalMap)
			std::cout << "Normal Map Enabled" << std::endl;
		else
			std::cout << "Normal Map Disabled" << std::endl;

		m_OpaqueEffectPtr->SetUseNormalMap(m_UseNormalMap);
	}

	void Renderer::ToggleShowFire()
	{
		m_ShowFire = !m_ShowFire;

		if (m_ShowFire)
			std::cout << "Show Fire Enabled" << std::endl;
		else
			std::cout << "Show FireDisabled" << std::endl;

	}


	bool Renderer::SaveBufferToImage() const
	{
		//const std::string filename = std::format("Screenshot/Rasterizer_{}.bmp", RENDER_MODE_NAMES.at(m_RenderMode));
		//return 	SDL_SaveBMP(m_BackBufferPtr, filename.c_str());

		assert(false && "Not implemented SaveBufferToImage");
		return false;

	}


}
