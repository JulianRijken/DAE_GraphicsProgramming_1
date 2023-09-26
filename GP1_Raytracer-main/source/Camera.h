#pragma once
#include <cassert>
#include <iostream>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngleDegrees):
			origin{_origin},
			fovAngle{_fovAngleDegrees}
		{}


		Vector3 origin{};
		float fovAngle{45.0f};
		float fovValue{ tanf((fovAngle * static_cast<float>(M_PI) / 180.0f) / 2.0f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float a{0.f};
		float y{0.f};




		Matrix CalculateCameraToWorld()
		{
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right);

			return
			{
				right,
				up,
				forward,
				origin
			};
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			Vector3 inputVector{};

			if (pKeyboardState[SDL_SCANCODE_A])
				inputVector.x -= 1;

			if (pKeyboardState[SDL_SCANCODE_D])
				inputVector.x += 1;

			if (pKeyboardState[SDL_SCANCODE_W])
				inputVector.z += 1;

			if (pKeyboardState[SDL_SCANCODE_S])
				inputVector.z -= 1;


			if (pKeyboardState[SDL_SCANCODE_Q])
				inputVector.y -= 1;

			if (pKeyboardState[SDL_SCANCODE_E])
				inputVector.y += 1;

			origin += inputVector * deltaTime * 20.0f; 


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			a -= mouseX * deltaTime * 0.4f;
			y -= mouseY * deltaTime * 0.4f;



			forward = Vector3::UnitZ;
			Matrix rotation
			{
				Vector3{cosf(a), 0, sinf(a)},
				Vector3{sinf(a) * sinf(y), cosf(y), -sinf(y) * cosf(a)},
				Vector3{-cosf(y) * sinf(a), sinf(y), cosf(y) * cosf(a)},
				Vector3::Zero
			};

			forward = rotation.TransformVector(forward);

		}
	};
}
