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
	m_AspectRatio = static_cast<float>(m_ScreenWidth) / static_cast<float>(m_ScreenHeight);

	//Create Buffers
	m_FrontBufferPtr = SDL_GetWindowSurface(pWindow);
	m_BackBufferPtr = SDL_CreateRGBSurface(0, m_ScreenWidth, m_ScreenHeight, 32, 0, 0, 0, 0);
	m_BackBufferPixelsPtr = static_cast<uint32_t*>(m_BackBufferPtr->pixels);
	m_pDepthBufferPixels = new float[m_ScreenWidth * m_ScreenHeight];

	InitializeObjects();
}

Renderer::~Renderer()
{
	for (const Material* material: m_Materials)
	{
		delete material->opacity;
		delete material->color;
		delete material;
	}

	delete[] m_pDepthBufferPixels;
}


void Renderer::InitializeObjects()
{
	// Uv debug material
	m_Materials.push_back(new Material
		{
		Texture::LoadFromFile("Resources/uv_grid_2.png"),
		nullptr
		});

	// Car body
	m_Materials.push_back(new Material
		{
		Texture::LoadFromFile("Resources/Car/Tex_FordGT40_Color_2k_02_Clean.png"),
		Texture::LoadFromFile("Resources/Car/Tex_FordGT40_Opacity_2k_02.png")
		});

	// Car wheel
	m_Materials.push_back(new Material
		{
		Texture::LoadFromFile("Resources/Car/Tex_TireAndRim_Color_1k_02.png"),
		nullptr
		});


	// tuktuk
	m_Materials.push_back(new Material
		{
		Texture::LoadFromFile("Resources/tuktuk.png"),
		nullptr
		});

	// Uv debug material
	m_Materials.push_back(new Material
		{
		Texture::LoadFromFile("Resources/uv_grid_3.png"),
		nullptr
		});




	//Mesh testMeshList{};
	//testMeshList.primitiveTopology = PrimitiveTopology::TriangleList;
	//testMeshList.materialPtrs.push_back(m_Materials[0]);
	//testMeshList.vertices =
	//{
	//	Vertex{{-3,  3, -2},{0.0f,0.0f}},
	//	Vertex{{ 0,  3, -2},{0.5f,0.0f}},
	//	Vertex{{ 3,  3, -2},{1.0f,0.0f}},
	//	Vertex{{-3,  0, -2},{0.0f,0.5f}},
	//	Vertex{{ 0,  0, -2},{0.5f,0.5f}},
	//	Vertex{{ 3,  0, -2},{1.0f,0.5f}},
	//	Vertex{{-3, -3, -2},{0.0f,1.0f}},
	//	Vertex{{ 0, -3, -2},{0.5f,1.0f}},
	//	Vertex{{ 3, -3, -2},{1.0f,1.0f}}
	//};
	//testMeshList.indices =
	//{
	//		3, 0, 1,    1, 4, 3,    4, 1, 2,
	//		2, 5, 4,    6, 3, 4,    4, 7, 6,
	//		7, 4, 5,    5, 8, 7
	//};


	Mesh testMeshStrip{};
	testMeshStrip.primitiveTopology = PrimitiveTopology::TriangleStrip;
	testMeshStrip.materialPtrs.push_back(m_Materials[0]);
	testMeshStrip.vertices =
	{
		Vertex{{-3,  3, -2},{0.0f,0.0f}},
		Vertex{{ 0,  3, -2},{0.5f,0.0f}},
		Vertex{{ 3,  3, -2},{1.0f,0.0f}},
		Vertex{{-3,  0, -2},{0.0f,0.5f}},
		Vertex{{ 0,  0, -2},{0.5f,0.5f}},
		Vertex{{ 3,  0, -2},{1.0f,0.5f}},
		Vertex{{-3, -3, -2},{0.0f,1.0f}},
		Vertex{{ 0, -3, -2},{0.5f,1.0f}},
		Vertex{{ 3, -3, -2},{1.0f,1.0f}}
	};
	testMeshStrip.indices =
	{
		3, 0, 4, 1, 5, 2, 2, 6, 6, 3, 7, 4, 8, 5
	};



	Mesh carMesh{};
	Utils::ParseOBJ("Resources/Car/car2.obj", carMesh.vertices, carMesh.indices);
	carMesh.primitiveTopology = PrimitiveTopology::TriangleList;
	carMesh.materialPtrs.push_back(m_Materials[1]);
	carMesh.materialPtrs.push_back(m_Materials[2]);
	//carMesh.Rotate(35 * TO_RADIANS);
	//carMesh.Translate({ -2, 0, 0 });


	Mesh tuktuk{};
	Utils::ParseOBJ("Resources/tuktuk.obj", tuktuk.vertices, tuktuk.indices);
	tuktuk.primitiveTopology = PrimitiveTopology::TriangleList;
	tuktuk.materialPtrs.push_back(m_Materials[3]);
	tuktuk.Scale({ 0.2f, 0.2f, 0.2f });
	tuktuk.Rotate(-35 * TO_RADIANS);
	tuktuk.Translate({ 2, 0, 0 });

	Mesh diorama{};
	Utils::ParseOBJ("Resources/Diorama2.obj", diorama.vertices, diorama.indices);
	diorama.primitiveTopology = PrimitiveTopology::TriangleList;
	diorama.materialPtrs.push_back(m_Materials[4]);
	//diorama.Scale({ 0.2f, 0.2f, 0.2f });
	//diorama.Rotate(-35 * TO_RADIANS);
	diorama.Translate({ 0, 0, 20 });


	

	// Setup meshes
	//m_WorldMeshes.push_back(testMeshList);
	//m_WorldMeshes.push_back(testMeshStrip);
	//m_WorldMeshes.push_back(diorama);
	m_WorldMeshes.push_back(carMesh);
	//m_WorldMeshes.push_back(tuktuk);
}


void Renderer::Render() const
{
	//Lock BackBuffer
	SDL_LockSurface(m_BackBufferPtr);

	// Clear depth buffer
	std::fill_n(m_pDepthBufferPixels, m_ScreenWidth * m_ScreenHeight, std::numeric_limits<float>::max());
	// Clear screen buffer
	constexpr int color{ 20 };
	SDL_FillRect(m_BackBufferPtr, nullptr, SDL_MapRGB(m_BackBufferPtr->format, color, color, color));

	// Render all meshes
	for (const Mesh& mesh : m_WorldMeshes)
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


void Renderer::World_to_Screen(const std::vector<Vertex>& verticesIn, std::vector<Vertex>& verticesOut) const
{
	verticesOut.resize(verticesIn.size());


	Matrix projectionMatrix
	{
		Vector4{1.0f / m_AspectRatio * m_CameraPtr->m_FovValue,0,0,0},
		Vector4{0,1.0f / m_CameraPtr->m_FovValue,0,0},
		Vector4{0,0,m_CameraPtr->m_FarClippingPlane / (m_CameraPtr->m_FarClippingPlane - m_CameraPtr->m_NearClippingPlane),1},
		Vector4{0,0,-(m_CameraPtr->m_FarClippingPlane - m_CameraPtr->m_NearClippingPlane) / (m_CameraPtr->m_FarClippingPlane - m_CameraPtr->m_NearClippingPlane),0},
	};

	Matrix worldViewProjectionMatrix = /*World matrix * */ m_CameraPtr->m_InvViewMatrix * projectionMatrix;


	for (int i{}; i < static_cast<int>(verticesIn.size()); i++)
	{
		constexpr bool useOldMethod = false;
		if (useOldMethod)
		{

			// Vertex in world space
			Vertex newVertex{ verticesIn[i] };

			// Convert from World to View/Camera space
			newVertex.position = m_CameraPtr->m_InvViewMatrix.TransformPoint(newVertex.position);

			// Apply perspective divide
			newVertex.position.x = newVertex.position.x / newVertex.position.z;
			newVertex.position.y = newVertex.position.y / newVertex.position.z;


			// Apply FOV and Aspect
			newVertex.position.x = newVertex.position.x / (m_AspectRatio * m_CameraPtr->m_FovValue);
			newVertex.position.y = newVertex.position.y / m_CameraPtr->m_FovValue;

			// Convert from NDC to screen
			newVertex.position.x = (newVertex.position.x + 1.0f) / 2.0f * static_cast<float>(m_ScreenWidth);
			newVertex.position.y = (1.0f - newVertex.position.y) / 2.0f * static_cast<float>(m_ScreenHeight);

			verticesOut[i] = newVertex;
		}
		else
		{
			Vertex_Out newVertex;

			// USE W AS THE NEW DEPTH
			newVertex.position = worldViewProjectionMatrix.TransformPoint( verticesIn[i].position);



			// USE W AS THE NEW DEPTH
			// Apply perspective divide  
			newVertex.position.x = newVertex.position.x / newVertex.position.w;
			newVertex.position.y = newVertex.position.y / newVertex.position.w;


			// Convert from NDC to screen
			newVertex.position.x = (newVertex.position.x + 1.0f) / 2.0f * static_cast<float>(m_ScreenWidth);
			newVertex.position.y = (1.0f - newVertex.position.y) / 2.0f * static_cast<float>(m_ScreenHeight);


			verticesOut[i] = newVertex;
		}
	}
}


void Renderer::RenderMesh(const Mesh& mesh) const
{
	// Convert world to screen
	std::vector<Vertex> verticesScreen{};
	World_to_Screen(mesh.vertices, verticesScreen);

	// Color world vertex
	int vertexIndex{ 0 };
	for (Vertex& vertex : verticesScreen)
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

	if (mesh.primitiveTopology == PrimitiveTopology::TriangleList)
	{
		Triangle triangle{};
		for (int i{}; i < static_cast<int>(mesh.indices.size()); i += 3)
		{
			triangle =
			{
				verticesScreen[mesh.indices[i]],
				verticesScreen[mesh.indices[i + 1]],
				verticesScreen[mesh.indices[i + 2]],
			};

			RenderTriangle(triangle, mesh.materialPtrs);
		}
	}
	else
	{
		Triangle triangle{};
		for (int i{}; i < static_cast<int>(mesh.indices.size() - 2); i++)
		{
			if (i % 2 == 0)
			{
				triangle =
				{
					verticesScreen[mesh.indices[i]],
					verticesScreen[mesh.indices[i + 1]],
					verticesScreen[mesh.indices[i + 2]]
				};

				RenderTriangle(triangle,mesh.materialPtrs);
			}
			else
			{
				Triangle triangle =
				{
					verticesScreen[mesh.indices[i]],
					verticesScreen[mesh.indices[i + 2]],
					verticesScreen[mesh.indices[i + 1]]
				};

				RenderTriangle(triangle,mesh.materialPtrs);
			}
		}
	}
}

void Renderer::RenderTriangle(const Triangle& triangle, const std::vector<Material*>& materialPtrs) const
{
	constexpr int boundingBoxPadding{1};

	const Vector3 edge1 = triangle.vertex1.position - triangle.vertex0.position;
	const Vector3 edge2 = triangle.vertex2.position - triangle.vertex0.position;
	const Vector3 normal = Vector3::Cross(edge1, edge2);


	//if (normal.z < 0.0f)
	//	return;

	// Create bounding box adding 1 pixel on each side
	// Adding the 1 pixel is done to prevent gaps in the triangles
	int minX = static_cast<int>(std::min(triangle.vertex0.position.x, std::min(triangle.vertex1.position.x, triangle.vertex2.position.x))) - boundingBoxPadding;
	int maxX = static_cast<int>(std::max(triangle.vertex0.position.x, std::max(triangle.vertex1.position.x, triangle.vertex2.position.x))) + boundingBoxPadding;
	int minY = static_cast<int>(std::min(triangle.vertex0.position.y, std::min(triangle.vertex1.position.y, triangle.vertex2.position.y))) - boundingBoxPadding;
	int maxY = static_cast<int>(std::max(triangle.vertex0.position.y, std::max(triangle.vertex1.position.y, triangle.vertex2.position.y))) + boundingBoxPadding;

	// Clamping is done so that the triangle is not rendered off the screen
	minX = std::ranges::clamp(minX, 0, m_ScreenWidth);
	maxX = std::ranges::clamp(maxX, 0, m_ScreenWidth);
	minY = std::ranges::clamp(minY, 0, m_ScreenHeight);
	maxY = std::ranges::clamp(maxY, 0, m_ScreenHeight);

	//// Get total area before
	//const float totalArea = (maxX - minX) * (maxY - minY) / 2.0f;
	//const float totalAreaInv = 1.0f / totalArea;

	// Looping all pixels within the bounding box
	// This is done for optimization
	for (int pixelX{ minX }; pixelX < maxX; pixelX++)
	{
		for (int pixelY{ minY }; pixelY < maxY; pixelY++)
		{
			const Vector2 pixelCenter{ static_cast<float>(pixelX) + 0.5f,static_cast<float>(pixelY) + 0.5f };
			float signedAreaW0;
			float signedAreaW1;
			float signedAreaW2;

			if (normal.z > 0.0f)
			{
				signedAreaW0 = Vector2::Cross(pixelCenter - triangle.vertex1.position.GetXY(), triangle.vertex2.position.GetXY() - triangle.vertex1.position.GetXY());
				if (signedAreaW0 >= 0) continue;
				signedAreaW1 = Vector2::Cross(pixelCenter - triangle.vertex2.position.GetXY(), triangle.vertex0.position.GetXY() - triangle.vertex2.position.GetXY());
				if (signedAreaW1 >= 0) continue;
				signedAreaW2 = Vector2::Cross(pixelCenter - triangle.vertex0.position.GetXY(), triangle.vertex1.position.GetXY() - triangle.vertex0.position.GetXY());
				if (signedAreaW2 >= 0) continue;
			}
			else
			{
				signedAreaW0 = Vector2::Cross(triangle.vertex2.position.GetXY() - triangle.vertex1.position.GetXY(), pixelCenter - triangle.vertex1.position.GetXY());
				if (signedAreaW0 >= 0) continue;
				signedAreaW1 = Vector2::Cross(triangle.vertex0.position.GetXY() - triangle.vertex2.position.GetXY(), pixelCenter - triangle.vertex2.position.GetXY());
				if (signedAreaW1 >= 0) continue;
				signedAreaW2 = Vector2::Cross(triangle.vertex1.position.GetXY() - triangle.vertex0.position.GetXY(), pixelCenter - triangle.vertex0.position.GetXY());
				if (signedAreaW2 >= 0) continue;
			}

			// Get total area before
			const float totalArea = signedAreaW0 + signedAreaW1 + signedAreaW2;
			const float totalAreaInv = 1.0f / totalArea;
			const float signedAreaW0Inv = signedAreaW0 * totalAreaInv;
			const float signedAreaW1Inv = signedAreaW1 * totalAreaInv;
			const float signedAreaW2Inv = signedAreaW2 * totalAreaInv;


			const float pixelDepth = 1.0f / (
				1.0f / triangle.vertex0.position.z * signedAreaW0Inv +
				1.0f / triangle.vertex1.position.z * signedAreaW1Inv +
				1.0f / triangle.vertex2.position.z * signedAreaW2Inv);

			const int pixelIndex{ pixelX + pixelY * m_ScreenWidth };


			// Depth check
			if (pixelDepth > m_pDepthBufferPixels[pixelIndex])
				continue;

			m_pDepthBufferPixels[pixelIndex] = pixelDepth;
			
			ColorRGB finalPixelColor{};
			const Material* material{ materialPtrs[triangle.vertex0.materialIndex] };


			const Vector2 uv =
				(
					triangle.vertex0.uv / triangle.vertex0.position.z * signedAreaW0Inv +
					triangle.vertex1.uv / triangle.vertex1.position.z * signedAreaW1Inv +
					triangle.vertex2.uv / triangle.vertex2.position.z * signedAreaW2Inv
					) * pixelDepth;

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
					finalPixelColor = colors::White * (1.0f - std::clamp(m_pDepthBufferPixels[pixelIndex] / 10.0f, 0.0f, 1.0f));;
				}break;
			case DebugRenderMode::UVColor:
					finalPixelColor = m_Materials[0]->color->Sample(uv);
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
