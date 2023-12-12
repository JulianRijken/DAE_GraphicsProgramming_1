#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "MathHelpers.h"

namespace dae
{
	class Camera
	{

	public:
		Camera(const Vector3& origin, float fovAngle, float aspectRatio);

		void Update(const Timer& timer);

		void PrintInfo() const;


		[[nodiscard]] const Matrix& GetProjectionMatrixPtr() const { return m_ProjectionMatrix; }
		[[nodiscard]] const Matrix& GetViewMatrixPtr() const { return m_ViewMatrix; }
		[[nodiscard]] const Matrix& GetViewProjectionMatrixPtr() const { return m_ViewProjectionMatrix; }

		void SetFovAngle(float fovAngle);

		void SetPosition(Vector3 position, bool teleport = true);

		void SetNearClipping(float value);

		void SetFarClipping(float value);

		void SetPitch(float pitch);

		void SetYaw(float yaw);


		void ChangeFovAngle(float fovAngleChange);

		void UpdateViewMatrix();
		void UpdateProjectionMatrix();
		void CombineViewProjectionMatrix();

	private:

		Vector3 m_Origin;
		Vector3 m_TargetOrigin;

		float m_FovAngle;
		float m_FovValue;

		float m_NearClippingPlane = 0.001f;
		float m_FarClippingPlane = 100000.0f;

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

		Matrix m_ViewProjectionMatrix{}; // Combined

		inline static constexpr float KEY_MOVE_SPEED{ 10.0f };
		inline static constexpr float MOUSE_MOVE_SPEED{ 0.07f };
		inline static constexpr float ROTATE_SPEED{ 0.001f };

		inline static constexpr float ROTATE_LERP_SPEED{ 20.0f };
		inline static constexpr float MOVE_LERP_SPEED{ 10.0f };

	};
}
