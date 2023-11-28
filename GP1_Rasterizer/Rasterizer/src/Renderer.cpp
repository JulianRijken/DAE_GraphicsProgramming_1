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
m_CameraPtr(camera)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_ScreenWidth, &m_ScreenHeight);

	//Create Buffers
	m_FrontBufferPtr = SDL_GetWindowSurface(pWindow);
	m_BackBufferPtr = SDL_CreateRGBSurface(0, m_ScreenWidth, m_ScreenHeight, 32, 0, 0, 0, 0);
	m_BackBufferPixelsPtr = static_cast<uint32_t*>(m_BackBufferPtr->pixels);
	m_pDepthBufferPixels = new float[m_ScreenWidth * m_ScreenHeight];

	InitializeScene();
}

Renderer::~Renderer()
{
	for (const std::pair<const std::string, Material*> materialPtrMap : m_MaterialPtrMap)
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



	//m_MaterialPtrMap.insert({ "carBody",new Material {
	//	Texture::LoadFromFile("Resources/Car/Tex_FordGT40_Color_2k_02_Clean.png"),
	//	Texture::LoadFromFile("Resources/Car/Tex_FordGT40_Opacity_2k_02.png")
	//}});

	//m_MaterialPtrMap.insert({ "carWheel",new Material {
	//	Texture::LoadFromFile("Resources/Car/Tex_TireAndRim_Color_1k_02.png"),
	//}});


	//m_MaterialPtrMap.insert({ "tukTuk",new Material {
	//	Texture::LoadFromFile("Resources/tuktuk.png"),
	//} });


	m_MaterialPtrMap.insert({ "bike",new Material {
		Texture::LoadFromFile("Resources/vehicle_diffuse.png"),
		nullptr
	}});
}

void Renderer::InitializeScene()
{
	const Mesh bikeMesh("Resources/vehicle.obj", { m_MaterialPtrMap["bike"] });
	//bikeMesh.Scale({ 0.2f, 0.2f, 0.2f });
	//bikeMesh.Rotate(-35 * TO_RADIANS);
	//bikeMesh.Translate({ 0, 0, 20 });
	m_WorldMeshes.push_back(bikeMesh);
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
		RenderMesh(mesh);
	

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
	// For the normal and trangent we go grom: model space -> world space (ONLY ROTATION SO MATRIX3)
	// For the view direction the camera is in world and we only need to translate the vertex from model -> world

	const Matrix worldViewProjectionMatrix =/* mesh.worldMatrix * */m_CameraPtr->m_InvViewMatrix * m_CameraPtr->m_ProjectionMatrix;

	for (VertexModel& vertex : mesh.m_VerticesModel)
	{
		vertex.positionScreen = Vector4(vertex.position.x, vertex.position.y, vertex.position.z, 1);

		vertex.positionScreen = worldViewProjectionMatrix.TransformPoint(vertex.positionScreen);

		// Apply perspective divide  
		vertex.positionScreen.x /= vertex.positionScreen.w;
		vertex.positionScreen.y /= vertex.positionScreen.w;
		vertex.positionScreen.z /= vertex.positionScreen.w;


		// Convert from NDC to screen
		vertex.positionScreen.x = (vertex.positionScreen.x + 1.0f) / 2.0f * static_cast<float>(m_ScreenWidth);
		vertex.positionScreen.y = (1.0f - vertex.positionScreen.y) / 2.0f * static_cast<float>(m_ScreenHeight);
	}
}


void Renderer::RenderMesh(Mesh& mesh) const
{
	// Convert world to screen
	TransformMesh(mesh);

	// Color world vertex
	int vertexIndex{ 0 };
	for (VertexModel& vertex : mesh.m_VerticesModel)
	{
		if (vertexIndex == 0)
			vertex.color = colors::Red;

		if (vertexIndex == 1)
			vertex.color = colors::Green;

		if (vertexIndex == 2)
			vertex.color = colors::Blue;

		vertexIndex++;
		vertexIndex %= 3;
	}

	if (mesh.m_PrimitiveTopology == PrimitiveTopology::TriangleList)
	{
		Triangle triangle{};
		for (int i{}; i < static_cast<int>(mesh.m_Indices.size()); i += 3)
		{
			triangle =
			{
				mesh.m_VerticesModel[mesh.m_Indices[i]],
				mesh.m_VerticesModel[mesh.m_Indices[i + 1]],
				mesh.m_VerticesModel[mesh.m_Indices[i + 2]],
			};

			RenderTriangle(triangle, mesh.m_MaterialPtrs);
		}
	}
	else
	{
		Triangle triangle{};
		for (int i{}; i < static_cast<int>(mesh.m_Indices.size() - 2); i++)
		{
			if (i % 2 == 0)
			{
				triangle =
				{
					mesh.m_VerticesModel[mesh.m_Indices[i]],
					mesh.m_VerticesModel[mesh.m_Indices[i + 1]],
					mesh.m_VerticesModel[mesh.m_Indices[i + 2]]
				};

				RenderTriangle(triangle,mesh.m_MaterialPtrs);
			}
			else
			{
				Triangle triangle =
				{
					mesh.m_VerticesModel[mesh.m_Indices[i]],
					mesh.m_VerticesModel[mesh.m_Indices[i + 2]],
					mesh.m_VerticesModel[mesh.m_Indices[i + 1]]
				};

				RenderTriangle(triangle,mesh.m_MaterialPtrs);
			}
		}
	}
}

void Renderer::RenderTriangle(const Triangle& triangle, const std::vector<Material*>& materialPtrs) const
{
	constexpr bool USE_BACK_FACE_CULLING = true;

	// early out culling
	if (triangle.vertex0.positionScreen.z < 0.0f or triangle.vertex0.positionScreen.z > 1.0f and
		triangle.vertex1.positionScreen.z < 0.0f or triangle.vertex1.positionScreen.z > 1.0f and
		triangle.vertex2.positionScreen.z < 0.0f or triangle.vertex2.positionScreen.z > 1.0f) return;

	if (triangle.vertex0.positionScreen.w < 0.0f) return;
	if (triangle.vertex1.positionScreen.w < 0.0f) return;
	if (triangle.vertex2.positionScreen.w < 0.0f) return;


	// Checking normal early for more performance
	const Vector3 normal = Vector3::Cross
	(
		triangle.vertex1.positionScreen - triangle.vertex0.positionScreen,
		triangle.vertex2.positionScreen - triangle.vertex0.positionScreen
	);


	if (USE_BACK_FACE_CULLING)
	{
		//if (normal.z <= 0.0f)
		//	return;
	}


	// Adding the 1 pixel is done to prevent gaps in the triangles
	constexpr int boundingBoxPadding{1};
	int minX = static_cast<int>(std::min(triangle.vertex0.positionScreen.x, std::min(triangle.vertex1.positionScreen.x, triangle.vertex2.positionScreen.x))) - boundingBoxPadding;
	int maxX = static_cast<int>(std::max(triangle.vertex0.positionScreen.x, std::max(triangle.vertex1.positionScreen.x, triangle.vertex2.positionScreen.x))) + boundingBoxPadding;
	int minY = static_cast<int>(std::min(triangle.vertex0.positionScreen.y, std::min(triangle.vertex1.positionScreen.y, triangle.vertex2.positionScreen.y))) - boundingBoxPadding;
	int maxY = static_cast<int>(std::max(triangle.vertex0.positionScreen.y, std::max(triangle.vertex1.positionScreen.y, triangle.vertex2.positionScreen.y))) + boundingBoxPadding;

	// Clamping is done so that the triangle is not rendered off the screen
	minX = std::ranges::clamp(minX, 0, m_ScreenWidth);
	maxX = std::ranges::clamp(maxX, 0, m_ScreenWidth);
	minY = std::ranges::clamp(minY, 0, m_ScreenHeight);
	maxY = std::ranges::clamp(maxY, 0, m_ScreenHeight);


	float signedAreaW0;
	float signedAreaW1;
	float signedAreaW2;

	// Looping all pixels within the bounding box
	// This is done for optimization
	for (int pixelX{ minX }; pixelX < maxX; pixelX++)
	{
		for (int pixelY{ minY }; pixelY < maxY; pixelY++)
		{
			const Vector2 pixelCenter{ static_cast<float>(pixelX) + 0.5f,static_cast<float>(pixelY) + 0.5f };

			if (USE_BACK_FACE_CULLING)
			{
				signedAreaW0 = Vector2::Cross(pixelCenter - triangle.vertex1.positionScreen.GetXY(), triangle.vertex2.positionScreen.GetXY() - triangle.vertex1.positionScreen.GetXY());
				if (signedAreaW0 >= 0) continue;
				signedAreaW1 = Vector2::Cross(pixelCenter - triangle.vertex2.positionScreen.GetXY(), triangle.vertex0.positionScreen.GetXY() - triangle.vertex2.positionScreen.GetXY());
				if (signedAreaW1 >= 0) continue;
				signedAreaW2 = Vector2::Cross(pixelCenter - triangle.vertex0.positionScreen.GetXY(), triangle.vertex1.positionScreen.GetXY() - triangle.vertex0.positionScreen.GetXY());
				if (signedAreaW2 >= 0) continue;
			}
			else
			{
				if (normal.z > 0.0f)
				{
					signedAreaW0 = Vector2::Cross(pixelCenter - triangle.vertex1.positionScreen.GetXY(), triangle.vertex2.positionScreen.GetXY() - triangle.vertex1.positionScreen.GetXY());
					if (signedAreaW0 >= 0) continue;
					signedAreaW1 = Vector2::Cross(pixelCenter - triangle.vertex2.positionScreen.GetXY(), triangle.vertex0.positionScreen.GetXY() - triangle.vertex2.positionScreen.GetXY());
					if (signedAreaW1 >= 0) continue;
					signedAreaW2 = Vector2::Cross(pixelCenter - triangle.vertex0.positionScreen.GetXY(), triangle.vertex1.positionScreen.GetXY() - triangle.vertex0.positionScreen.GetXY());
					if (signedAreaW2 >= 0) continue;
				}
				else
				{
					signedAreaW0 = Vector2::Cross(triangle.vertex2.positionScreen.GetXY() - triangle.vertex1.positionScreen.GetXY(), pixelCenter - triangle.vertex1.positionScreen.GetXY());
					if (signedAreaW0 >= 0) continue;
					signedAreaW1 = Vector2::Cross(triangle.vertex0.positionScreen.GetXY() - triangle.vertex2.positionScreen.GetXY(), pixelCenter - triangle.vertex2.positionScreen.GetXY());
					if (signedAreaW1 >= 0) continue;
					signedAreaW2 = Vector2::Cross(triangle.vertex1.positionScreen.GetXY() - triangle.vertex0.positionScreen.GetXY(), pixelCenter - triangle.vertex0.positionScreen.GetXY());
					if (signedAreaW2 >= 0) continue;
				}
			}

			// Get total area before
			const float totalArea = signedAreaW0 + signedAreaW1 + signedAreaW2;
			const float totalAreaInv = 1.0f / totalArea;
			const float signedAreaW0Inv = signedAreaW0 * totalAreaInv;
			const float signedAreaW1Inv = signedAreaW1 * totalAreaInv;
			const float signedAreaW2Inv = signedAreaW2 * totalAreaInv;


			const float nonLinearPixelDepth = 1.0f / (
				1.0f / triangle.vertex0.positionScreen.z * signedAreaW0Inv +
				1.0f / triangle.vertex1.positionScreen.z * signedAreaW1Inv +
				1.0f / triangle.vertex2.positionScreen.z * signedAreaW2Inv);


			// Culling I DON"T KNOW IT DOES NOT SEEM CORRECT THE DEPTH THAT IS
			//if (nonLinearPixelDepth < 0.0f or nonLinearPixelDepth > 1.0f)
			//	continue;

			const int pixelIndex{ pixelX + pixelY * m_ScreenWidth };

			// Depth check
			if (nonLinearPixelDepth > m_pDepthBufferPixels[pixelIndex])
				continue;
			m_pDepthBufferPixels[pixelIndex] = nonLinearPixelDepth;



			const float linearPixelDepth = 1.0f / (
				1.0f / triangle.vertex0.positionScreen.w * signedAreaW0Inv +
				1.0f / triangle.vertex1.positionScreen.w * signedAreaW1Inv +
				1.0f / triangle.vertex2.positionScreen.w * signedAreaW2Inv);


			const Vector2 uv = linearPixelDepth * (
				triangle.vertex0.uv / triangle.vertex0.positionScreen.w * signedAreaW0Inv +
				triangle.vertex1.uv / triangle.vertex1.positionScreen.w * signedAreaW1Inv +
				triangle.vertex2.uv / triangle.vertex2.positionScreen.w * signedAreaW2Inv);




			ColorRGB finalPixelColor{};
			const Material* material{ materialPtrs[triangle.vertex0.materialIndex] };
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
						ColorRGB backColor
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
					switch (triangle.vertex0.materialIndex)
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
				case DebugRenderMode::BiometricCoordinate:
				{
					finalPixelColor =
						triangle.vertex0.color * signedAreaW0 * totalAreaInv +
						triangle.vertex1.color * signedAreaW1 * totalAreaInv +
						triangle.vertex2.color * signedAreaW2 * totalAreaInv;
				} break;
				case DebugRenderMode::DepthBuffer:
				{
					//finalPixelColor = colors::White * ;
					//finalPixelColor = colors::White * std::lerp(0.985f, 1.0f, nonLinearPixelDepth);
					
					//finalPixelColor = colors::White * std::clamp(triangle.vertex0.positionScreen.z,0.0f,1.0f);


					//finalPixelColor = colors::White * std::clamp(nonLinearPixelDepth,0.0f,1.0f);


					//finalPixelColor = colors::White * Utils::MapValueInRangeClamped(nonLinearPixelDepth,0.9f,1.0f,0.0f,1.0f);
					//finalPixelColor = colors::White * std::ranges::clamp(nonLinearPixelDepth,0.0f,1.0f);

					if (nonLinearPixelDepth < 0.0f)
						finalPixelColor = colors::Red;
					else
					if (nonLinearPixelDepth > 1.0f)
						finalPixelColor = colors::Blue;
					else
						finalPixelColor = colors::Green * nonLinearPixelDepth;


				}break;
			case DebugRenderMode::UVColor:
					finalPixelColor = m_MaterialPtrMap[0]->color->Sample(uv);
				break;
			}


			//Update Color in Buffer
			//finalPixelColor.MaxToOne();
			m_BackBufferPixelsPtr[pixelIndex] = SDL_MapRGB(m_BackBufferPtr->format,
				static_cast<uint8_t>(finalPixelColor.r * 255),
				static_cast<uint8_t>(finalPixelColor.g * 255),
				static_cast<uint8_t>(finalPixelColor.b * 255));
		}
	}
}



bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_BackBufferPtr, "Rasterizer_ColorBuffer.bmp");
}
