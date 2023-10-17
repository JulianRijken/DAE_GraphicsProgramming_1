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
			fovAngle = std::min(180.0f, std::max(0.0f, _fovDegrees));
			fovValue = tanf((fovAngle * TO_RADIANS) / 2.0f);
		}

		void SetPosition(Vector3 postion)
		{
			targetOrigin = postion;
			origin = postion;
		}

		void SetRotation(float pitch, float yaw)
		{
			pitch *= TO_RADIANS;
			yaw *= TO_RADIANS;

			targetCameraPitch = pitch;
			cameraPitch = pitch;
			targetCameraYaw = yaw;
			cameraYaw = yaw;
		}

		void HandleCameraMovement(const float deltaTime)
		{
			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if ((mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0)
			{
				targetCameraPitch -= mouseY * 0.001f;
				targetCameraYaw -= mouseX * 0.001f;
			}

			cameraPitch = Jul::Lerp(cameraPitch, targetCameraPitch, deltaTime / cameraRotateSmoothing);
			cameraYaw = Jul::Lerp(cameraYaw, targetCameraYaw, deltaTime / cameraRotateSmoothing);

			const Matrix pitchYawRotation
			{
				Vector3{cosf(cameraYaw), 0, sinf(cameraYaw)},
				Vector3{sinf(cameraYaw) * sinf(cameraPitch), cosf(cameraPitch), -sinf(cameraPitch) * cosf(cameraYaw)},
				Vector3{-cosf(cameraPitch) * sinf(cameraYaw), sinf(cameraPitch), cosf(cameraPitch) * cosf(cameraYaw)},
				Vector3::Zero
			};

			forward = Vector3::UnitZ;
			forward = pitchYawRotation.TransformVector(forward);


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

		}

		void Update(Timer* pTimer)
		{
			HandleCameraMovement(pTimer->GetElapsed());
		}
	};
}
