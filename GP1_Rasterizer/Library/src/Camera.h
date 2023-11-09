#pragma once
#include <cassert>
#include <iostream>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			m_Origin{_origin},
			m_FovAngle{_fovAngle}
		{}


		Vector3 m_Origin{};
		float m_FovAngle{90.f};
		float m_Fov{ tanf((m_FovAngle * TO_RADIANS) / 2.f) };

		Vector3 m_Forward{Vector3::UnitZ};
		Vector3 m_Up{Vector3::UnitY};
		Vector3 m_Right{Vector3::UnitX};

		float m_TotalPitch{};
		float m_TotalYaw{};

		Matrix m_InvViewMatrix{};
		Matrix m_ViewMatrix{};

		inline static constexpr float KEY_MOVE_SPEED{ 20.0f };
		inline static constexpr float MOUSE_MOVE_SPEED{ 1.0f };
		inline static constexpr float ROTATE_SPEED{ 0.001f };

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			m_FovAngle = _fovAngle;
			m_Fov = tanf((m_FovAngle * TO_RADIANS) / 2.f);

			m_Origin = _origin;
		}

		void CalculateViewMatrix()
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
			//TODO W3

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(const Timer& timer)
		{
			const float deltaTime = timer.GetElapsed();


			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			const bool isRightMouseDown{ (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0 };
			const bool isLeftMouseDown{ (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0 };

			Vector3 inputVector{};


			if (isLeftMouseDown && isRightMouseDown)
			{
				inputVector.y -= mouseY * MOUSE_MOVE_SPEED;
			}
			else if (isLeftMouseDown)
			{
				inputVector.z -= mouseY * MOUSE_MOVE_SPEED;
				m_TotalYaw -= mouseX * ROTATE_SPEED;
			}
			else if (isRightMouseDown)
			{
				m_TotalPitch -= mouseY * ROTATE_SPEED;
				m_TotalYaw -= mouseX * ROTATE_SPEED;
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



			const Matrix pitchYawRotation
			{
				Vector3{std::cos(m_TotalYaw), 0, std::sin(m_TotalYaw)},
				Vector3{std::sin(m_TotalYaw) * std::sin(m_TotalPitch), std::cos(m_TotalPitch), -std::sin(m_TotalPitch) * std::cos(m_TotalYaw)},
				Vector3{-std::cos(m_TotalPitch) * std::sin(m_TotalYaw), std::sin(m_TotalPitch),std::cos(m_TotalPitch) * std::cos(m_TotalYaw)},
				Vector3::Zero
			};


			m_Forward = Vector3::UnitZ; // Because we transform the point we need to reset it first
			m_Forward = pitchYawRotation.TransformVector(m_Forward);
			m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();
			m_Up = Vector3::Cross(m_Forward, m_Right).Normalized();


			inputVector = pitchYawRotation.TransformVector(inputVector);
			m_Origin += inputVector * deltaTime * 10.0f;

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}
