#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& origin, float fovAngle, float aspectRatio) :
		m_Origin{origin},
		m_TargetOrigin{m_Origin},
		m_AspectRatio{aspectRatio}
		{
			SetFovAngle(fovAngle);
		}


		Vector3 m_Origin;
		Vector3 m_TargetOrigin;

		float m_FovAngle;
		float m_FovValue;

		float m_NearClippingPlane = 3.0f;
		float m_FarClippingPlane = 200.0f;

		Vector3 m_Forward{ Vector3::UnitZ };
		Vector3 m_Up{ Vector3::UnitY };
		Vector3 m_Right{ Vector3::UnitX };

		float m_Pitch{};
		float m_TargetPitch{};
		float m_Yaw{};
		float m_TargetYaw{};

		float m_AspectRatio{};

		Matrix m_InvViewMatrix{};
		Matrix m_ViewMatrix{};
		Matrix m_ProjectionMatrix{};

		inline static constexpr float KEY_MOVE_SPEED{ 50.0f };
		inline static constexpr float MOUSE_MOVE_SPEED{ 0.07f };
		inline static constexpr float ROTATE_SPEED{ 0.001f };

		inline static constexpr float ROTATE_LERP_SPEED{ 20.0f };
		inline static constexpr float MOVE_LERP_SPEED{ 10.0f };


		void Update(const Timer& timer)
		{
			constexpr float minFps{ 30.0f };
			constexpr float maxElapsed{ 1.0f / minFps };
			// using min to create a minimum delay
			const float deltaTime = std::min(timer.GetElapsed(), maxElapsed);

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			const bool isRightMouseDown{ (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0 };
			const bool isLeftMouseDown{ (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0 };

			Vector3 inputVector{};
			Vector3 inputVectorMouse{};


			if (isLeftMouseDown && isRightMouseDown)
			{
				inputVectorMouse.y -= mouseY * MOUSE_MOVE_SPEED;
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
			else if (isLeftMouseDown)
			{
				inputVectorMouse.z -= mouseY * MOUSE_MOVE_SPEED;
				m_TargetYaw -= mouseX * ROTATE_SPEED;
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
			else if (isRightMouseDown)
			{
				m_TargetPitch -= mouseY * ROTATE_SPEED;
				m_TargetYaw -= mouseX * ROTATE_SPEED;
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
			else
			{
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}


			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
				inputVector.x -= 1;

			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
				inputVector.x += 1;

			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
				inputVector.z += 1;

			if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
				inputVector.z -= 1;

			if (pKeyboardState[SDL_SCANCODE_Q] || pKeyboardState[SDL_SCANCODE_LSHIFT])
				inputVector.y -= 1;

			if (pKeyboardState[SDL_SCANCODE_E] || pKeyboardState[SDL_SCANCODE_LCTRL])
				inputVector.y += 1;

			//m_Pitch = m_TargetPitch;
			//m_Yaw = m_TargetYaw;

			m_Pitch = Lerpf(m_Pitch, m_TargetPitch, deltaTime * ROTATE_LERP_SPEED);
			m_Yaw = Lerpf(m_Yaw, m_TargetYaw, deltaTime * ROTATE_LERP_SPEED);


			const Matrix pitchYawRotation
			{
				Vector3{std::cos(m_Yaw), 0, std::sin(m_Yaw)},
				Vector3{std::sin(m_Yaw) * std::sin(m_Pitch), std::cos(m_Pitch), -std::sin(m_Pitch) * std::cos(m_Yaw)},
				Vector3{-std::cos(m_Pitch) * std::sin(m_Yaw), std::sin(m_Pitch),std::cos(m_Pitch) * std::cos(m_Yaw)},
				Vector3::Zero
			};


			m_Forward = Vector3::UnitZ; // Because we transform the point we need to reset it first
			m_Forward = pitchYawRotation.TransformVector(m_Forward);

			m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();
			m_Up = Vector3::Cross(m_Forward, m_Right).Normalized();


			inputVector = pitchYawRotation.TransformVector(inputVector);
			m_TargetOrigin += (inputVector * deltaTime * KEY_MOVE_SPEED) + inputVectorMouse;
			//m_Origin = m_TargetOrigin;
			m_Origin = Lerp(m_Origin, m_TargetOrigin, deltaTime * MOVE_LERP_SPEED);

			//Update Matrices
			//Try to optimize this - should only be called once or when fov/aspectRatio changes
			UpdateViewMatrix();
			CalculateProjectionMatrix();
		}

		void PrintInfo() const
		{
			std::cout << "Origin: {" << m_Origin.x << ", " << m_Origin.y << ", " << m_Origin.z << "}" << std::endl;
			std::cout << "Pitch: " << m_Pitch << std::endl;
			std::cout << "Yaw: " << m_Yaw << std::endl;
		}

		void SetFovAngle(float fovAngle)
		{
			m_FovAngle = fovAngle;
			m_FovValue = tanf((m_FovAngle * TO_RADIANS) / 2.f);
		}

		void SetPosition(Vector3 position, bool teleport = true)
		{
			m_TargetOrigin = position;

			if(teleport)
				m_Origin = position;
		}

		void SetNearClipping(float value)
		{
			m_NearClippingPlane = value;
		}

		void SetFarClipping(float value)
		{
			m_FarClippingPlane = value;
		}

		void SetPitch(float pitch)
		{
			m_Pitch = pitch;
			m_TargetPitch = pitch;
		}

		void SetYaw(float yaw)
		{
			m_Yaw = yaw;
			m_TargetYaw = yaw;
		}



		void ChangeFovAngle(float fovAngleChange)
		{
			m_FovAngle += fovAngleChange;

			if (m_FovAngle < 5.0f)
			{
				m_FovAngle = 5.0f;
				return;
			}

			if (m_FovAngle > 175.0f)
			{
				m_FovAngle = 175.0f;
				return;
			}



			m_FovValue = std::tan((m_FovAngle * TO_RADIANS) / 2.f);


			// Extra to create effect that camera is not moving
			const Matrix pitchYawRotation
			{
				Vector3{std::cos(m_Yaw), 0, std::sin(m_Yaw)},
				Vector3{std::sin(m_Yaw) * std::sin(m_Pitch), std::cos(m_Pitch), -std::sin(m_Pitch) * std::cos(m_Yaw)},
				Vector3{-std::cos(m_Pitch) * std::sin(m_Yaw), std::sin(m_Pitch),std::cos(m_Pitch) * std::cos(m_Yaw)},
				Vector3::Zero
			};

			const float distanceToTarget = (m_Origin - Vector3{0,0,0}).Magnitude(); // Assuming you have a function to get the length of a vector
			const float offsetMagnitude = distanceToTarget * tanf((fovAngleChange * TO_RADIANS) / 2.f);
			const Vector3 offset = pitchYawRotation.TransformVector({ 0,0,offsetMagnitude});

			m_Origin += offset;
			m_TargetOrigin = m_Origin;
		}

		void UpdateViewMatrix()
		{
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
			//m_InvViewMatrix = Matrix::CreateLookAtLH(m_Origin, m_Forward, m_Up);

			m_ViewMatrix =
			{
				m_Right,
				m_Up,
				m_Forward,
				m_Origin
			};

			m_InvViewMatrix = Matrix::Inverse(m_ViewMatrix);
		}

		void CalculateProjectionMatrix()
		{
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
			//m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(m_FovValue,m_AspectRatio)

			m_ProjectionMatrix = 
			{
				Vector4{1.0f / (m_AspectRatio * m_FovValue),0,0,0},
				Vector4{0,1.0f / m_FovValue,0,0},
				Vector4{0,0,m_FarClippingPlane / (m_FarClippingPlane - m_NearClippingPlane),1},
				Vector4{0,0,-(m_FarClippingPlane * m_NearClippingPlane) / (m_FarClippingPlane - m_NearClippingPlane),0},
			};
		}

	};
}
