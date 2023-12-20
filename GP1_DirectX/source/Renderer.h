#pragma once
#include <map>

class Effect;
struct VertexModel;
struct Material;
class Mesh;
struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
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


	struct DefaultTextures
	{
		Texture* defaultWhiteTexture;
		Texture* defaultBlackTexture;
		Texture* defaultNormalTexture;
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

		void Update(const Timer& timer) const;
		void Render() const;

		void SetRenderMode(DebugRenderMode mode);
		void CycleRenderMode();
		void ToggleCameraOrbit();
		void ToggleMeshRotation();
		void CycleSampleState();

		bool SaveBufferToImage() const;


	private:

		HRESULT InitializeDirectX();

		void InitializeSceneTriangle();
		void InitializeSceneAssignment();
		void InitializeSceneCar();
		void InitializeSceneDiorama();

		Mesh* AddMesh(const std::vector<VertexModel>& vertices, const std::vector<uint32_t>& indices, const std::vector<Material*>& materials);
		Mesh* AddMesh(const std::string& objName, const std::vector<Material*>& materials);
		Mesh* AddMesh(const std::string& objName, const std::string& mtlName);


		SDL_Window* m_pWindow{};

		int m_WindowWidth{};
		int m_WindowHeight{};

		int m_SampleState{};

		bool m_IsInitialized{ false };
		bool m_OrbitCamera{ false };
		bool m_RotateMesh{ false };
		float m_OrbitCameraDistance{ 400.0f };

		Camera* m_CameraPtr;
		DebugRenderMode m_RenderMode;

		Effect* m_DefaultEffectPtr;

		std::vector<Mesh*> m_WorldMeshes;
		std::map <std::string, Material* > m_MaterialPtrMap;

		DefaultTextures m_DefaultTextures;
		
		ID3D11Device*			m_DevicePtr{};
		ID3D11DeviceContext*	m_DeviceContextPtr{};
		IDXGISwapChain*			m_SwapChainPtr{};
		ID3D11Texture2D*		m_DepthStencilBufferPtr{};
		ID3D11DepthStencilView* m_DepthStencilViewPtr{};
		ID3D11Resource*			m_RenderTargetBufferPtr{};
		ID3D11RenderTargetView* m_RenderTargetViewPtr{};
	};

}
