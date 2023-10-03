#pragma once
#include <cassert>
#include <iostream>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include "Jul.h"

namespace dae
{
	struct Camera
	{
		Camera()
		{
			SetFOV(fovAngle);
		}

		//Camera(const Vector3& _origin, float _fovAngleDegrees):
		//	origin{_origin},
		//	fovAngle{_fovAngleDegrees}
		//{}

		Vector3 targetOrigin{};
		Vector3 origin{};


		float fovAngle{45.0f};
		float fovValue{};

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float cameraRotateSmoothing{ 0.06f };
		float cameraMoveSmoothing{ 0.1f };

		float targetCameraPitch{ 0.f };
		float targetCameraYaw{ 0.f };

		float cameraPitch{0.f};
		float cameraYaw{0.f};


		Matrix CalculateCameraToWorld()
		{
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			return
			{
				right,
				up,
				forward,
				origin
			};
		}

		void AdjustFOV(float _fovDelta)
		{
			SetFOV(fovAngle + _fovDelta);
		}

		void SetFOV(float _fovDegrees)
		{
			fovAngle =  std::min(180.0f, std::max(0.0f, _fovDegrees));
			fovValue = tanf((fovAngle * static_cast<float>(M_PI) / 180.0f) / 2.0f);
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			const Matrix pitchYawRotation
			{
				Vector3{cosf(cameraPitch), 0, sinf(cameraPitch)},
				Vector3{sinf(cameraPitch) * sinf(cameraYaw), cosf(cameraYaw), -sinf(cameraYaw) * cosf(cameraPitch)},
				Vector3{-cosf(cameraYaw) * sinf(cameraPitch), sinf(cameraYaw), cosf(cameraYaw) * cosf(cameraPitch)},
				Vector3::Zero
			};


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



			inputVector = pitchYawRotation.TransformVector(inputVector);
			targetOrigin += inputVector * deltaTime * 20.0f; 

			origin = Jul::Lerp(origin, targetOrigin, deltaTime / cameraMoveSmoothing);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			targetCameraPitch -= mouseX * deltaTime * 0.1f;
			targetCameraYaw -= mouseY * deltaTime * 0.1f;

			cameraPitch = Jul::Lerp(cameraPitch, targetCameraPitch, deltaTime / cameraRotateSmoothing);
			cameraYaw = Jul::Lerp(cameraYaw, targetCameraYaw, deltaTime / cameraRotateSmoothing);


			forward = Vector3::UnitZ;
			forward = pitchYawRotation.TransformVector(forward);

		}
	};
}
