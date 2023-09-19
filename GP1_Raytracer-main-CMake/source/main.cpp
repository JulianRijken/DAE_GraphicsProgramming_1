//External includes
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

int main(int argc, char* argv[])
{
    //Unreferenced parameters
    (void)argc;
    (void)argv;

    //Create window + surfaces
    SDL_Init(SDL_INIT_VIDEO);

    const uint32_t width = 640;
    const uint32_t height = 480;

    SDL_Window* pWindow = SDL_CreateWindow(
        "RayTracer - not Matias Devred",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(pWindow, -1, 0);

    if (!pWindow)
        return 1;

    //Initialize "framework"
    const auto pTimer = new Timer();
    const auto pRenderer = new Renderer(pWindow);

    const auto pScene = new Scene_W1();
    pScene->Initialize();

    //Start loop
    pTimer->Start();

    // Start Benchmark
    // pTimer->StartBenchmark();

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
            case SDL_KEYUP:
                if(e.key.keysym.scancode == SDL_SCANCODE_X)
                    takeScreenshot = true;
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
