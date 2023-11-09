//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
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
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
}

void Renderer::Update(const Timer& timer)
{
	m_Camera.Update(timer);
}

void Renderer::Render()
{
	//Lock BackBuffer
	SDL_LockSurface(m_BackBufferPtr);

	const std::vector<Vertex> verticesWorld
	{
		// Triangle 0
		{{ 0.0f,  2.0f, 0.0f},{1,0,0}},
		{{ 1.5f, -1.0f, 0.0f},{0,1,0}},
		{{-1.5f, -1.0f, 0.0f},{0,0,1}},

		// Triangle 1
		{{ 0.0f,  4.0f, 2.0f},{1,0,0}},
		{{ 3.0f, -2.0f, 2.0f},{0,1,0}},
		{{-3.0f, -2.0f, 2.0f},{0,0,1}},
	};

	std::vector<Vertex> verticesScreen{};
	World_to_Screen(verticesWorld, verticesScreen);




	//RENDER LOGIC FOR EACH PIXEL
	for (int pixelX{}; pixelX < m_ScreenWidth; ++pixelX)
	{
		for (int pixelY{}; pixelY < m_ScreenHeight; ++pixelY)
		{
			// Fill in the color for each pixel
			ColorRGB finalPixelColor{};

			const Vector2 pixelCenter{ (float)pixelX + 0.5f,(float)pixelY + 0.5f };
			const int pixelIndex{ pixelX + pixelY * m_ScreenWidth };

			// Clear depth buffer 
			m_pDepthBufferPixels[pixelIndex] = std::numeric_limits<float>::max();


			//RENDER LOGIC FOR EACH TRIANGLE
			for (int i{}; i < (int)verticesScreen.size(); i += 3)
			{
				Triangle triangle
				{
					verticesScreen[i],
					verticesScreen[i + 1],
					verticesScreen[i + 2]
				};


				const float SignedArea0{ Vector2::Cross(pixelCenter - triangle.vertex1.position.GetXY(), triangle.vertex2.position.GetXY() - triangle.vertex1.position.GetXY()) };
				if (SignedArea0 > 0.0f)
					continue;

				const float SignedArea1{ Vector2::Cross(pixelCenter - triangle.vertex2.position.GetXY(), triangle.vertex0.position.GetXY() - triangle.vertex2.position.GetXY()) };
				if (SignedArea1 > 0.0f)
					continue;

				const float SignedArea2{ Vector2::Cross(pixelCenter - triangle.vertex0.position.GetXY(), triangle.vertex1.position.GetXY() - triangle.vertex0.position.GetXY()) };
				if (SignedArea2 > 0.0f)
					continue;

				const float totalArea = SignedArea0 + SignedArea1 + SignedArea2;

				const float pixelDepth =
					triangle.vertex0.position.z * (SignedArea0 / totalArea) +
					triangle.vertex1.position.z * (SignedArea1 / totalArea) +
					triangle.vertex2.position.z * (SignedArea2 / totalArea);

				if (pixelDepth > m_pDepthBufferPixels[pixelIndex])
					continue;

				m_pDepthBufferPixels[pixelIndex] = pixelDepth;


				finalPixelColor =
					triangle.vertex0.color * (SignedArea0 / totalArea )+
					triangle.vertex1.color * (SignedArea1 / totalArea )+
					triangle.vertex2.color * (SignedArea2 / totalArea);
			}

			//Update Color in Buffer
			finalPixelColor.MaxToOne();

			m_BackBufferPixelsPtr[pixelIndex] = SDL_MapRGB(m_BackBufferPtr->format,
				static_cast<uint8_t>(finalPixelColor.r * 255),
				static_cast<uint8_t>(finalPixelColor.g * 255),
				static_cast<uint8_t>(finalPixelColor.b * 255));
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


	for (int i{}; i < (int)verticesIn.size(); i++)
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
		newVertex.position.x = (newVertex.position.x + 1.0f) / 2.0f * (float)m_ScreenWidth;
		newVertex.position.y = (1.0f - newVertex.position.y) / 2.0f * (float)m_ScreenHeight;

		verticesOut[i] = newVertex;
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_BackBufferPtr, "Rasterizer_ColorBuffer.bmp");
}
