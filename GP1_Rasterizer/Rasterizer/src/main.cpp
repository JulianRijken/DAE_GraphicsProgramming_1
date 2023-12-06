//External includes
#include "vld.h"
#include "SDL.h"
#include "SDL_surface.h"
#undef main

//Standard includes
#include <iostream>
#include <algorithm>

//Project includes
#include "Camera.h"
#include "Timer.h"
#include "Renderer.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	constexpr uint32_t width = 640;
	constexpr uint32_t height = 480;

	//constexpr uint32_t width = 1280;
	//constexpr uint32_t height = 720;

	//constexpr uint32_t width = 1920;
	//constexpr uint32_t height = 1080;

	constexpr float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

	SDL_Window* pWindow = SDL_CreateWindow(
		"Rasterizer - Julian Rijken",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;




	//Initialize "framework"
	Timer timer{}; 
	Camera camera{ {0,0,0.0f},90.0f,aspectRatio };
	Renderer renderer{&camera, pWindow};

	//Start loop
	timer.Start();


	float printTimer = 0.f;
	bool isLooping = true;
	bool takeScreenshot = false;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.scancode == SDL_SCANCODE_X)
					takeScreenshot = true;

				if (e.key.keysym.scancode == SDL_SCANCODE_F5)
					renderer.ToggleRotation();
				if (e.key.keysym.scancode == SDL_SCANCODE_F6)
					renderer.ToggleNormalMap();
				if (e.key.keysym.scancode == SDL_SCANCODE_F7)
					renderer.CycleRenderMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_F8)
					renderer.ToggleLinearDepth();

				if (e.key.keysym.scancode == SDL_SCANCODE_1)
					renderer.SetRenderMode(DebugRenderMode::Diffuse);
				if (e.key.keysym.scancode == SDL_SCANCODE_2)
					renderer.SetRenderMode(DebugRenderMode::ObservedArea);
				if (e.key.keysym.scancode == SDL_SCANCODE_3)
					renderer.SetRenderMode(DebugRenderMode::DiffuseOA);
				if (e.key.keysym.scancode == SDL_SCANCODE_4)
					renderer.SetRenderMode(DebugRenderMode::SpecularOA);
				if (e.key.keysym.scancode == SDL_SCANCODE_5)
					renderer.SetRenderMode(DebugRenderMode::Combined);
				if (e.key.keysym.scancode == SDL_SCANCODE_6)
					renderer.SetRenderMode(DebugRenderMode::UVColor);
				if (e.key.keysym.scancode == SDL_SCANCODE_7)
					renderer.SetRenderMode(DebugRenderMode::Weights);
				if (e.key.keysym.scancode == SDL_SCANCODE_8)
					renderer.SetRenderMode(DebugRenderMode::DepthBuffer);
				if (e.key.keysym.scancode == SDL_SCANCODE_9)
					renderer.SetRenderMode(DebugRenderMode::MaterialIndex);
				if (e.key.keysym.scancode == SDL_SCANCODE_0)
					renderer.SetRenderMode(DebugRenderMode::Opacity);
				break;

			case SDL_MOUSEWHEEL:
				if (e.wheel.y > 0)
					camera.ChangeFovAngle(-5.0f);
				if (e.wheel.y < 0)
					camera.ChangeFovAngle(5.0f);
				break;

			}
		}

		//--------- Update ---------
		camera.Update(timer);
		renderer.Update(timer);

		//--------- Render ---------
		renderer.Render();

		//--------- Timer ---------
		timer.Update();
		printTimer += timer.GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << timer.GetdFPS() << std::endl;
		}

		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!renderer.SaveBufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}

	timer.Stop();

	ShutDown(pWindow);
	return 0;
}