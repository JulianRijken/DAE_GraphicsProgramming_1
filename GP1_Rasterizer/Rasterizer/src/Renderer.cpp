//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <algorithm>
#include <execution>
#include <iostream>

#include "Camera.h"
#include "Maths.h"
#include "Mesh.h"
#include "Texture.h"
#include "Utils.h"

#define MULTI_THREAD

using namespace dae;

Renderer::Renderer(Camera* camera, SDL_Window* pWindow) :
m_WindowPtr(pWindow),
m_CameraPtr(camera),
m_RenderMode(DebugRenderMode::Color),
m_ScreenWidth(), // Is set later
m_ScreenHeight() // Is set later
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_ScreenWidth, &m_ScreenHeight);

	//Create Buffers
	m_FrontBufferPtr = SDL_GetWindowSurface(pWindow);
	m_BackBufferPtr = SDL_CreateRGBSurface(0, m_ScreenWidth, m_ScreenHeight, 32, 0, 0, 0, 0);
	m_BackBufferPixelsPtr = static_cast<uint32_t*>(m_BackBufferPtr->pixels);
	m_pDepthBufferPixels = new float[m_ScreenWidth * m_ScreenHeight];

	InitializeMaterials();
	InitializeScene();
}

Renderer::~Renderer()
{
	for (const std::pair<const std::string, Material*>& materialPtrMap : m_MaterialPtrMap)
	{
		delete materialPtrMap.second->color;
		delete materialPtrMap.second->opacity;
		delete materialPtrMap.second->normalMap;
		delete materialPtrMap.second;
	}

	delete[] m_pDepthBufferPixels;
}

void Renderer::InitializeMaterials()
{
	m_MaterialPtrMap.insert({ "uvGrid2",new Material {
		Texture::LoadFromFile("Resources/uv_grid_2.png"),
	}});


	//m_MaterialPtrMap.insert({ "uvGrid3",new Material {
	//	Texture::LoadFromFile("Resources/uv_grid_3.png"),
	//}});



	m_MaterialPtrMap.insert({ "carBody",new Material {
		Texture::LoadFromFile("Resources/Car/Tex_FordGT40_Color_2k_02_Clean.png"),
		Texture::LoadFromFile("Resources/Car/Tex_FordGT40_Opacity_2k_02.png")
	}});

	m_MaterialPtrMap.insert({ "carWheel",new Material {
		Texture::LoadFromFile("Resources/Car/Tex_TireAndRim_Color_1k_02.png"),
	}});


	//m_MaterialPtrMap.insert({ "tukTuk",new Material {
	//	Texture::LoadFromFile("Resources/tuktuk.png"),
	//} });


	m_MaterialPtrMap.insert({"bike",new Material {
		Texture::LoadFromFile("Resources/vehicle_diffuse.png"),
		nullptr
	}});
}

void Renderer::InitializeScene()
{
	Mesh bikeMesh("Resources/vehicle.obj", { m_MaterialPtrMap["bike"] });
	//bikeMesh.SetScale({ 0.2f, 0.2f, 0.2f });
	//bikeMesh.SetYawRotation(-100.0f * TO_RADIANS);
	//bikeMesh.SetPosition({ 50, 0, 0 });
	m_WorldMeshes.push_back(bikeMesh);


	//Mesh carMesh("Resources/Car/car2.obj", { m_MaterialPtrMap["carBody"],m_MaterialPtrMap["carWheel"] });
	//carMesh.SetScale({8,8,8});
	//m_WorldMeshes.push_back(carMesh);
}


void Renderer::Update(const Timer& timer)
{
	m_WorldMeshes[0].AddYawRotation(timer.GetElapsed() * 0.5f);
}

void Renderer::Render()
{
	//Lock BackBuffer
	SDL_LockSurface(m_BackBufferPtr);

	// Clear depth buffer
	std::fill_n(m_pDepthBufferPixels, m_ScreenWidth * m_ScreenHeight, std::numeric_limits<float>::max());
	// Clear screen buffer
	constexpr int color{ 20 };
	SDL_FillRect(m_BackBufferPtr, nullptr, SDL_MapRGB(m_BackBufferPtr->format, color, color, color));


	// Render all meshes
	for (Mesh& mesh : m_WorldMeshes)
		RasterizeMesh(mesh);
	

	//Update SDL Surface
	SDL_UnlockSurface(m_BackBufferPtr);
	SDL_BlitSurface(m_BackBufferPtr, 0, m_FrontBufferPtr, 0);
	SDL_UpdateWindowSurface(m_WindowPtr);
}


void Renderer::CycleDebugMode(bool up)
{
	// Cycle tough modes
	int nextRenderMode = static_cast<int>(m_RenderMode) + (up ? 1 : -1);

	if (nextRenderMode > static_cast<int>(DebugRenderMode::COUNT) - 1)
		nextRenderMode = 0;
	else if (nextRenderMode < 0)
		nextRenderMode = static_cast<int>(DebugRenderMode::COUNT) - 1;

	m_RenderMode = static_cast<DebugRenderMode>(nextRenderMode);

	std::cout << std::endl;
	std::cout << RENDER_MODE_NAMES.at(m_RenderMode).c_str() << std::endl;
	std::cout << std::endl;
}

void Renderer::SetRenderMode(DebugRenderMode mode)
{
	m_RenderMode = mode;

	std::cout << std::endl;
	std::cout << RENDER_MODE_NAMES.at(m_RenderMode).c_str() << std::endl;
	std::cout << std::endl;
}


void Renderer::TransformMesh(Mesh& mesh) const
{
	// SPACES
	// - Model
	// - World
	// - world offset -> Camera -> projection
	// - Perspective divide
	// - NDC

	// For our translation
	// For the postions we go from model space -> NDC
	// For the normal and trangent we go from: model space -> world space (ONLY ROTATION SO MATRIX3)
	// For the view direction the camera is in world and we only need to translate the vertex from model -> world


	// Make sure the transformed vertices are back in the same state as the model vertices
	// This is done because we apply matrix transformations on the transformed vertices
	mesh.ResetTransformedVertices();

	const Matrix worldToViewProjectionMatrix = m_CameraPtr->m_InvViewMatrix * m_CameraPtr->m_ProjectionMatrix;

	for (const std::shared_ptr<VertexTransformed>& vertex : mesh.m_VerticesTransformed)
	{
		// Convert vertex to world
		vertex->pos = mesh.m_WorldMatrix.TransformPoint(vertex->pos);

		// Convert normal to world
		// Note we use transform vector
		vertex->normal = mesh.m_WorldMatrix.TransformPoint(vertex->normal);
		vertex->tangent = mesh.m_WorldMatrix.TransformPoint(vertex->tangent);

		// Calculate view direction based on vertex in world
		vertex->viewDirection = (vertex->pos.GetXYZ() - m_CameraPtr->m_Origin).Normalized(); // Might not need the normalized

		// Transform vertex to view
		vertex->pos = worldToViewProjectionMatrix.TransformPoint(vertex->pos);

		// Apply perspective divide  
		vertex->pos.x /= vertex->pos.w;
		vertex->pos.y /= vertex->pos.w;
		vertex->pos.z /= vertex->pos.w;

		// Convert from NDC to screen
		vertex->pos.x = (vertex->pos.x + 1.0f) / 2.0f * static_cast<float>(m_ScreenWidth);
		vertex->pos.y = (1.0f - vertex->pos.y) / 2.0f * static_cast<float>(m_ScreenHeight);
	}
}


void Renderer::RasterizeMesh(Mesh& mesh) const
{
	TransformMesh(mesh);

#ifdef MULTI_THREAD
	std::for_each(std::execution::par, mesh.m_Triangles.begin(), mesh.m_Triangles.end(), [this,mesh](Triangle& triangle)
		{
			RasterizeTriangle(triangle, mesh.m_MaterialPtrs);
		});
#else
	for (Triangle& triangle : mesh.m_Triangles)
		RasterizeTriangle(triangle, mesh.m_MaterialPtrs);
#endif
}

void Renderer::RasterizeTriangle(const Triangle& triangle, const std::vector<Material*>& materialPtrs) const
{
	// early out culling
	if (triangle.vertex0->pos.z < 0.0f or triangle.vertex0->pos.z > 1.0f and
		triangle.vertex1->pos.z < 0.0f or triangle.vertex1->pos.z > 1.0f and
		triangle.vertex2->pos.z < 0.0f or triangle.vertex2->pos.z > 1.0f) return;

	if (triangle.vertex0->pos.w < 0.0f) return;
	if (triangle.vertex1->pos.w < 0.0f) return;
	if (triangle.vertex2->pos.w < 0.0f) return;


	// Checking normal early for more performance
	const Vector3 normal = Vector3::Cross
	(
		triangle.vertex1->pos - triangle.vertex0->pos,
		triangle.vertex2->pos - triangle.vertex0->pos
	);

	if (normal.z <= 0.0f)
		return;

	// Adding the 1 pixel is done to prevent gaps in the triangles
	constexpr int boundingBoxPadding{1};
	int minX = static_cast<int>(std::min(triangle.vertex0->pos.x, std::min(triangle.vertex1->pos.x, triangle.vertex2->pos.x))) - boundingBoxPadding;
	int maxX = static_cast<int>(std::max(triangle.vertex0->pos.x, std::max(triangle.vertex1->pos.x, triangle.vertex2->pos.x))) + boundingBoxPadding;
	int minY = static_cast<int>(std::min(triangle.vertex0->pos.y, std::min(triangle.vertex1->pos.y, triangle.vertex2->pos.y))) - boundingBoxPadding;
	int maxY = static_cast<int>(std::max(triangle.vertex0->pos.y, std::max(triangle.vertex1->pos.y, triangle.vertex2->pos.y))) + boundingBoxPadding;

	// Clamping is done so that the triangle is not rendered off the screen
	minX = std::ranges::clamp(minX, 0, m_ScreenWidth);
	maxX = std::ranges::clamp(maxX, 0, m_ScreenWidth);
	minY = std::ranges::clamp(minY, 0, m_ScreenHeight);
	maxY = std::ranges::clamp(maxY, 0, m_ScreenHeight);


	// Looping all pixels within the bounding box
	// This is done for optimization
	for (int pixelX{ minX }; pixelX < maxX; pixelX++)
	{
		for (int pixelY{ minY }; pixelY < maxY; pixelY++)
		{
			const Vector2 pixelCenter{ static_cast<float>(pixelX) + 0.5f,static_cast<float>(pixelY) + 0.5f };

			const float signedAreaW0 = Vector2::Cross(pixelCenter - triangle.vertex1->pos.GetXY(),triangle.vertex2->pos.GetXY() - triangle.vertex1->pos.GetXY());
			if (signedAreaW0 >= 0) continue;
			const float signedAreaW1 = Vector2::Cross(pixelCenter - triangle.vertex2->pos.GetXY(),triangle.vertex0->pos.GetXY() - triangle.vertex2->pos.GetXY());
			if (signedAreaW1 >= 0) continue;
			const float signedAreaW2 = Vector2::Cross(pixelCenter - triangle.vertex0->pos.GetXY(),triangle.vertex1->pos.GetXY() - triangle.vertex0->pos.GetXY());
			if (signedAreaW2 >= 0) continue;


			// Get total area before
			const float totalArea = signedAreaW0 + signedAreaW1 + signedAreaW2;
			const float totalAreaInv = 1.0f / totalArea;
			Vector3 weights
			{
				signedAreaW0 * totalAreaInv,
				signedAreaW1 * totalAreaInv,
				signedAreaW2 * totalAreaInv,
			};


			const float nonLinearDepth = 1.0f / (
				1.0f / triangle.vertex0->pos.z * weights.x +
				1.0f / triangle.vertex1->pos.z * weights.y +
				1.0f / triangle.vertex2->pos.z * weights.z);

			// Culling I DON"T KNOW IT DOES NOT SEEM CORRECT THE DEPTH THAT IS
			//if (nonLinearPixelDepth < 0.0f or nonLinearPixelDepth > 1.0f)
			//	continue;

			const int pixelIndex = pixelX + pixelY * m_ScreenWidth;

			// Depth check
			if (nonLinearDepth > m_pDepthBufferPixels[pixelIndex])
				continue;
			m_pDepthBufferPixels[pixelIndex] = nonLinearDepth;

			ShadePixel(triangle, materialPtrs, weights, pixelIndex, nonLinearDepth);
		}
	}
}

void Renderer::ShadePixel(const Triangle& triangle, const std::vector<Material*>& materialPtrs, const Vector3& weights, int pixelIndex, float nonLinearDepth) const
{
	const float linearPixelDepth = 1.0f / (
		1.0f / triangle.vertex0->pos.w * weights.x +
		1.0f / triangle.vertex1->pos.w * weights.y +
		1.0f / triangle.vertex2->pos.w * weights.z);


	const Vector2 uv = linearPixelDepth * (
		triangle.vertex0->uv / triangle.vertex0->pos.w * weights.x +
		triangle.vertex1->uv / triangle.vertex1->pos.w * weights.y +
		triangle.vertex2->uv / triangle.vertex2->pos.w * weights.z);


	ColorRGB finalPixelColor{};
	const Material* material{ materialPtrs[triangle.vertex0->materialIndex] };

	switch (m_RenderMode)
	{
	case DebugRenderMode::FinalColor:
	{
		if (material->color == nullptr)
			break;

		ColorRGB color{};

		// Can be further optimized and more checks
		color = material->color->Sample(uv);

		if (material->opacity != nullptr)
		{
			const uint32_t pixel = m_BackBufferPixelsPtr[pixelIndex];
			// Extract individual color channels (assuming 8 bits per channel)
			const uint8_t red = (pixel & 0xFF0000) >> 16;
			const uint8_t green = (pixel & 0x00FF00) >> 8;
			const uint8_t blue = pixel & 0x0000FF;
			const ColorRGB backColor
			{
				static_cast<float>(red) / 255.0f,
				static_cast<float>(green) / 255.0f,
				static_cast<float>(blue) / 255.0f,
			};

			ColorRGB opacity{ 1.0f,1.0f,1.0f };
			opacity = material->opacity->Sample(uv);

			const float alpha = std::ranges::clamp(opacity.r, 0.0f, 1.0f);
			finalPixelColor = ColorRGB::Lerp(backColor, color, alpha);
		}
		else
		{
			finalPixelColor = color;
		}

	} break;
	case DebugRenderMode::Color:
	{
		if (material->color == nullptr)
			break;

		finalPixelColor = material->color->Sample(uv);

	} break;
	case DebugRenderMode::MaterialIndex:
	{
		switch (triangle.vertex0->materialIndex)
		{
		case 0:
			finalPixelColor = colors::Red;
			break;
		case 1:
			finalPixelColor = colors::Blue;
			break;
		case 2:
			finalPixelColor = colors::Green;
			break;
		case 3:
			finalPixelColor = colors::Cyan;
			break;
		case 4:
			finalPixelColor = colors::Magenta;
			break;
		case 5:
			finalPixelColor = colors::Yellow;
			break;
		default:
			finalPixelColor = colors::White;
		}

	} break;
	case DebugRenderMode::Opacity:
	{
		if (material->opacity == nullptr)
			finalPixelColor = colors::White;
		else
			finalPixelColor = material->opacity->Sample(uv);
	} break;
	case DebugRenderMode::Weights:
	{
		finalPixelColor =
			triangle.vertex0->color * weights.x +
			triangle.vertex1->color * weights.y +
			triangle.vertex2->color * weights.z;
	} break;								
	case DebugRenderMode::DepthBuffer:
	{
		//finalPixelColor = colors::White * ;
		//finalPixelColor = colors::White * std::lerp(0.985f, 1.0f, nonLinearPixelDepth);

		//finalPixelColor = colors::White * std::clamp(triangle.vertex0.positionScreen.z,0.0f,1.0f);


		//finalPixelColor = colors::White * std::clamp(nonLinearPixelDepth,0.0f,1.0f);


		//finalPixelColor = colors::White * Utils::MapValueInRangeClamped(nonLinearPixelDepth,0.9f,1.0f,0.0f,1.0f);
		//finalPixelColor = colors::White * std::ranges::clamp(nonLinearPixelDepth,0.0f,1.0f);

		if (nonLinearDepth < 0.0f)
			finalPixelColor = colors::Red;
		else
			if (nonLinearDepth > 1.0f)
				finalPixelColor = colors::Blue;
			else
				finalPixelColor = colors::Green * nonLinearDepth;


	}break;
	case DebugRenderMode::UVColor:
		finalPixelColor = m_MaterialPtrMap.at("uvGrid2")->color->Sample(uv);
		break;
	}


	//Update Color in Buffer
	//finalPixelColor.MaxToOne();
	m_BackBufferPixelsPtr[pixelIndex] = SDL_MapRGB(m_BackBufferPtr->format,
		static_cast<uint8_t>(finalPixelColor.r * 255),
		static_cast<uint8_t>(finalPixelColor.g * 255),
		static_cast<uint8_t>(finalPixelColor.b * 255));
}


bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_BackBufferPtr, "Rasterizer_ColorBuffer.bmp");
}


