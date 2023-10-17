//External includes
#include "vld.h"
#include "SDL.h"
#include "SDL_surface.h"
#undef main

//Standard includes
#include <iostream>

//Project includes
#include "Timer.h"
#include "Renderer.h"
#include "Scene.h"

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

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"RayTracer - Julian Rijken",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	SDL_SetRelativeMouseMode(SDL_TRUE);


	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	const auto pScene = new Scene_W3();
	pScene->Initialize();

	//Start loop
	pTimer->Start();

	// Start Benchmark
	//pTimer->StartBenchmark();

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
				if(e.key.keysym.scancode == SDL_SCANCODE_X)
					takeScreenshot = true;

				if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
					SDL_SetRelativeMouseMode(SDL_FALSE);

				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
					pRenderer->ToggleShadows();

				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
					pRenderer->CycleLightMode();

				break;
			case SDL_MOUSEWHEEL:
				if (e.wheel.y > 0)
					pScene->GetCamera().AdjustFOV(-5.0f);
				else if (e.wheel.y < 0)
					pScene->GetCamera().AdjustFOV(5.0f);
				break;

			}
		}

		//--------- Update ---------
		pScene->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render(pScene);

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}

		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!pRenderer->SaveBufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pScene;
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}