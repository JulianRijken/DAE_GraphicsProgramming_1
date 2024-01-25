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


		inline static constexpr float KEY_MOVE_SPEED{20.0f};
		inline static constexpr float MOUSE_MOVE_SPEED{0.1f};
		inline static constexpr float ROTATE_SPEED{0.001f};

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
			Vector3 localInputVector{};
			Vector3 worldInputVector{};

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			const bool isRightMouseDown{ (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0 };
			const bool isLeftMouseDown{ (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0 };

			if(isLeftMouseDown && isRightMouseDown)
			{
				localInputVector.y -= mouseY * MOUSE_MOVE_SPEED;
			}
			else if(isLeftMouseDown)
			{
				localInputVector.z -= mouseY  * MOUSE_MOVE_SPEED;
				targetCameraYaw -= mouseX * ROTATE_SPEED;
			}
			else if (isRightMouseDown)
			{
				targetCameraPitch -= mouseY * ROTATE_SPEED;
				targetCameraYaw -= mouseX  * ROTATE_SPEED;
			}

			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
				localInputVector.x -= 1;

			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
				localInputVector.x += 1;

			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
				localInputVector.z += 1;

			if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
				localInputVector.z -= 1;

			if (pKeyboardState[SDL_SCANCODE_Q] || pKeyboardState[SDL_SCANCODE_LSHIFT])
				worldInputVector.y -= 1;

			if (pKeyboardState[SDL_SCANCODE_E] || pKeyboardState[SDL_SCANCODE_LCTRL])
				worldInputVector.y += 1;


			// Apply input vector and rotation

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

			localInputVector = pitchYawRotation.TransformVector(localInputVector);
			targetOrigin += (localInputVector + worldInputVector) * deltaTime * KEY_MOVE_SPEED;
			origin = Jul::Lerp(origin, targetOrigin, deltaTime / cameraMoveSmoothing);

			//std::cout << "x: " << origin.x << "y: " << origin.y << "z: " << origin.z << std::endl;
			//std::cout << "Pitch: " << cameraPitch << "Yaw: " << cameraYaw << std::endl;
			//std::cout << "Fov: " << fovAngle << std::endl;
		}

		void Update(Timer* pTimer)
		{
			HandleCameraMovement(pTimer->GetElapsed());
		}
	};
}
