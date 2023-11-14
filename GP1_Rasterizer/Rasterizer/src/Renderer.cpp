//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <algorithm>
#include <execution>

#include "Maths.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_WindowPtr(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_ScreenWidth, &m_ScreenHeight);
	m_AspectRatio = static_cast<float>(m_ScreenWidth) / static_cast<float>(m_ScreenHeight);

	//Create Buffers
	m_FrontBufferPtr = SDL_GetWindowSurface(pWindow);
	m_BackBufferPtr = SDL_CreateRGBSurface(0, m_ScreenWidth, m_ScreenHeight, 32, 0, 0, 0, 0);
	m_BackBufferPixelsPtr = static_cast<uint32_t*>(m_BackBufferPtr->pixels);
	m_pDepthBufferPixels = new float[m_ScreenWidth * m_ScreenHeight];

	//Initialize Camera
	m_Camera.Initialize(60.f, { 0.0f,0.0f,-10.f });

	Mesh testMeshList
	{
		{
			Vertex{{-3,  3, -2}},
			Vertex{{ 0,  3, -2}},
			Vertex{{ 3,  3, -2}},
			Vertex{{-3,  0, -2}},
			Vertex{{ 0,  0, -2}},
			Vertex{{ 3,  0, -2}},
			Vertex{{-3, -3, -2}},
			Vertex{{ 0, -3, -2}},
			Vertex{{ 3, -3, -2}}
		},
		{
			3, 0, 1,    1, 4, 3,    4, 1, 2,
			2, 5, 4,    6, 3, 4,    4, 7, 6,
			7, 4, 5,    5, 8, 7
		},
		PrimitiveTopology::TriangleList
	};

	Mesh testMeshStrip
	{
		{
			Vertex{{-3,  3, -2}},
			Vertex{{ 0,  3, -2}},
			Vertex{{ 3,  3, -2}},
			Vertex{{-3,  0, -2}},
			Vertex{{ 0,  0, -2}},
			Vertex{{ 3,  0, -2}},
			Vertex{{-3, -3, -2}},
			Vertex{{ 0, -3, -2}},
			Vertex{{ 3, -3, -2}}
		},
		{
			3, 0, 4, 1, 5, 2, 2, 6, 6, 3, 7, 4, 8, 5
		},
		PrimitiveTopology::TriangleStrip
	};

	Mesh testMeshParse{};
	Utils::ParseOBJ("Resources/tuktuk.obj", testMeshParse.vertices, testMeshParse.indices);
	testMeshParse.primitiveTopology = PrimitiveTopology::TriangleList;

	//Utils::ParseOBJ("Resources/simple_quad.obj", testMeshParse.vertices, testMeshParse.indices);



	m_WorldMeshes.clear();
	m_WorldMeshes.resize(1);
	m_WorldMeshes[0] = testMeshStrip;

	std::cout << "Mesh Vertices: " << m_WorldMeshes[0].vertices.size() << std::endl;
	std::cout << "Mesh Indices: " << m_WorldMeshes[0].indices.size() << std::endl;


}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
}

void Renderer::Update(const Timer& timer)
{
	m_Camera.Update(timer);
}

void Renderer::Render() const
{
	//Lock BackBuffer
	SDL_LockSurface(m_BackBufferPtr);



	// Convert world to screen
	std::vector<Vertex> verticesScreen{};
	World_to_Screen(m_WorldMeshes[0].vertices, verticesScreen);


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

	//std::ranges::reverse(verticesScreen);




	// Clear depth buffer
	std::fill_n(m_pDepthBufferPixels, m_ScreenWidth * m_ScreenHeight, std::numeric_limits<float>::max());

	// Clear screen buffer
	SDL_FillRect(m_BackBufferPtr, nullptr, SDL_MapRGB(m_BackBufferPtr->format, 100, 100, 100));

	int triangleIndex{};

	//RENDER LOGIC FOR EACH TRIANGLE
	for (int i{}; i < static_cast<int>(m_WorldMeshes[0].indices.size() - 2); i++)
	{
		triangleIndex++;

		Triangle triangle{};
		if (m_WorldMeshes[0].primitiveTopology == PrimitiveTopology::TriangleList)
		{
			
			triangle = 
			{
				verticesScreen[m_WorldMeshes[0].indices[i]],
				verticesScreen[m_WorldMeshes[0].indices[i + 1]],
				verticesScreen[m_WorldMeshes[0].indices[i + 2]]
			};
		}
		else
		{
			if (i % 2 == 0)
			{

				triangle =
				{
					verticesScreen[m_WorldMeshes[0].indices[i]],
					verticesScreen[m_WorldMeshes[0].indices[i + 1]],
					verticesScreen[m_WorldMeshes[0].indices[i + 2]]
				};
			}
			else
			{
				triangle =
				{
					verticesScreen[m_WorldMeshes[0].indices[i]],
					verticesScreen[m_WorldMeshes[0].indices[i + 2]],
					verticesScreen[m_WorldMeshes[0].indices[i + 1]]
				};
			}

		}

		int minX = static_cast<int>(std::min(triangle.vertex0.position.x,std::min(triangle.vertex1.position.x, triangle.vertex2.position.x))) - 1;
		int maxX = static_cast<int>(std::max(triangle.vertex0.position.x,std::max(triangle.vertex1.position.x, triangle.vertex2.position.x))) + 1;
		int minY = static_cast<int>(std::min(triangle.vertex0.position.y,std::min(triangle.vertex1.position.y, triangle.vertex2.position.y))) - 1;
		int maxY = static_cast<int>(std::max(triangle.vertex0.position.y,std::max(triangle.vertex1.position.y, triangle.vertex2.position.y))) + 1;

		minX = std::ranges::clamp(minX, 0,m_ScreenWidth);
		maxX = std::ranges::clamp(maxX, 0,m_ScreenWidth);
		minY = std::ranges::clamp(minY, 0,m_ScreenHeight);
		maxY = std::ranges::clamp(maxY, 0, m_ScreenHeight);

		for (int pixelX{ minX }; pixelX < maxX; ++pixelX)
		{
			for (int pixelY{ minY }; pixelY < maxY; ++pixelY)
			{
				const Vector2 pixelCenter{ static_cast<float>(pixelX) + 0.5f,static_cast<float>(pixelY) + 0.5f };
				//const Vector2 pixelCenter{ static_cast<float>(pixelX) + 1.0f, static_cast<float>(pixelY) + 1.0f }; // Fixes lines

				const float signedArea0{ Vector2::Cross(pixelCenter - triangle.vertex1.position.GetXY(), triangle.vertex2.position.GetXY() - triangle.vertex1.position.GetXY()) };
				if (signedArea0 > 0)
					continue;

				const float signedArea1{ Vector2::Cross(pixelCenter - triangle.vertex2.position.GetXY(), triangle.vertex0.position.GetXY() - triangle.vertex2.position.GetXY()) };
				if (signedArea1 > 0)
					continue;

				const float signedArea2{ Vector2::Cross(pixelCenter - triangle.vertex0.position.GetXY(), triangle.vertex1.position.GetXY() - triangle.vertex0.position.GetXY()) };
				if (signedArea2 > 0)
					continue;

				const float totalArea = signedArea0 + signedArea1 + signedArea2;
				const float totalAreaInv = 1.0f / totalArea;


				const float pixelDepth =
					triangle.vertex0.position.z * signedArea0 * totalAreaInv +
					triangle.vertex1.position.z * signedArea1 * totalAreaInv +
					triangle.vertex2.position.z * signedArea2 * totalAreaInv;

				const int pixelIndex{ pixelX + pixelY * m_ScreenWidth };

				// Depth check
				if (pixelDepth > m_pDepthBufferPixels[pixelIndex])
					continue;

				m_pDepthBufferPixels[pixelIndex] = pixelDepth;

				ColorRGB finalPixelColor =
					triangle.vertex0.color * signedArea0 * totalAreaInv +
					triangle.vertex1.color * signedArea1 * totalAreaInv +
					triangle.vertex2.color * signedArea2 * totalAreaInv;

				//finalPixelColor = colors::White;

				//finalPixelColor *= 1.0f - std::clamp(m_pDepthBufferPixels[pixelIndex] / 40.0f,0.0f,1.0f);

				//Update Color in Buffer
				//finalPixelColor.MaxToOne();


				//m_BackBufferPixelsPtr[pixelIndex] = SDL_MapRGB(m_BackBufferPtr->format,
				//	static_cast<uint8_t>(std::clamp(m_pDepthBufferPixels[pixelIndex] / 30.0f,0.0f,1.0f) * 255),
				//	static_cast<uint8_t>(std::clamp(m_pDepthBufferPixels[pixelIndex] / 30.0f,0.0f,1.0f) * 255),
				//	static_cast<uint8_t>(std::clamp(m_pDepthBufferPixels[pixelIndex] / 30.0f,0.0f,1.0f) * 255));

				m_BackBufferPixelsPtr[pixelIndex] = SDL_MapRGB(m_BackBufferPtr->format,
					static_cast<uint8_t>(finalPixelColor.r * 255),
					static_cast<uint8_t>(finalPixelColor.g * 255),
					static_cast<uint8_t>(finalPixelColor.b * 255));


				//// WHITE
				//m_BackBufferPixelsPtr[pixelIndex] = SDL_MapRGB(m_BackBufferPtr->format,
				//	static_cast<uint8_t>(255),
				//	static_cast<uint8_t>(255),
				//	static_cast<uint8_t>(255));

			}
		}
	}



	//Update SDL Surface
	SDL_UnlockSurface(m_BackBufferPtr);
	SDL_BlitSurface(m_BackBufferPtr, 0, m_FrontBufferPtr, 0);
	SDL_UpdateWindowSurface(m_WindowPtr);
}

void Renderer::World_to_Screen(const std::vector<Vertex>& verticesIn, std::vector<Vertex>& verticesOut) const
{
	//verticesOut.clear(); // Maybe do this somewhere else

	verticesOut.resize(verticesIn.size());


	for (int i{}; i < static_cast<int>(verticesIn.size()); i++)
	{
		// Vertex in world space
		Vertex newVertex{ verticesIn[i] };

		// Convert from World to View/Camera space
		newVertex.position = m_Camera.m_InvViewMatrix.TransformPoint(newVertex.position);

		// Apply perspective divide
		newVertex.position.x = newVertex.position.x / newVertex.position.z;
		newVertex.position.y = newVertex.position.y / newVertex.position.z;

		// Apply FOV and Aspect
		newVertex.position.x = newVertex.position.x / (m_AspectRatio * m_Camera.m_Fov);
		newVertex.position.y = newVertex.position.y / m_Camera.m_Fov;

		// Convert from NDC to screen
		newVertex.position.x = (newVertex.position.x + 1.0f) / 2.0f * static_cast<float>(m_ScreenWidth);
		newVertex.position.y = (1.0f - newVertex.position.y) / 2.0f * static_cast<float>(m_ScreenHeight);

		verticesOut[i] = newVertex;
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_BackBufferPtr, "Rasterizer_ColorBuffer.bmp");
}
