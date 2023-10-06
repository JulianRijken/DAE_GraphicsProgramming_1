#pragma once
#include <string>
#include <vector>

#include "SDL_keyboard.h"
#include "Math/Vector3.h"
#include "Misc/Camera.h"
#include "Misc/DataTypes.h"
#include <math/ColorRGB.h>


namespace dae
{
	//Forward Declarations
	class Timer;
	class Material;
	struct Plane;
	struct Sphere;
	struct Light;

	//Scene Base Class
	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void Update(dae::Timer* pTimer)
		{
			//float rad1{ std::sin(SDL_GetTicks64() / 1000.f) * 40.f };
			//m_SphereGeometries[0].radius = rad1;
			//
			//float rad2{ std::sin(SDL_GetTicks64() / 1600.f) * 60.f };
			//m_SphereGeometries[1].radius = rad2;
			//
			//float rad3{ std::sin(SDL_GetTicks64() / 1900.f) * 30.f };
			//m_SphereGeometries[2].radius = rad3;

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			Vector3 inputVector{};

			if (pKeyboardState[SDL_SCANCODE_J])
				inputVector.x -= 1;

			if (pKeyboardState[SDL_SCANCODE_L])
				inputVector.x += 1;

			if (pKeyboardState[SDL_SCANCODE_I])
				inputVector.z += 1;

			if (pKeyboardState[SDL_SCANCODE_K])
				inputVector.z -= 1;

			if (pKeyboardState[SDL_SCANCODE_U])
				inputVector.y -= 1;

			if (pKeyboardState[SDL_SCANCODE_O])
				inputVector.y += 1;


			m_Lights[0].origin += inputVector * pTimer->GetElapsed() * 10.0f;


			m_Camera.Update(pTimer);
		}

		Camera& GetCamera() { return m_Camera; }
		void GetClosestHit(const Ray& ray, HitRecord& closestHit) const;
		bool DoesHit(const Ray& ray) const;

		const std::vector<Plane>& GetPlaneGeometries() const { return m_PlaneGeometries; }
		const std::vector<Sphere>& GetSphereGeometries() const { return m_SphereGeometries; }
		const std::vector<Light>& GetLights() const { return m_Lights; }
		const std::vector<Material*> GetMaterials() const { return m_Materials; }

	protected:
		std::string	sceneName;

		std::vector<Plane> m_PlaneGeometries{};
		std::vector<Sphere> m_SphereGeometries{};
		std::vector<TriangleMesh> m_TriangleMeshGeometries{};
		std::vector<Light> m_Lights{};
		std::vector<Material*> m_Materials{};

		Camera m_Camera{};

		Sphere* AddSphere(const Vector3& origin, float radius, unsigned char materialIndex = 0);
		Plane* AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex = 0);
		TriangleMesh* AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex = 0);

		Light* AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color);
		Light* AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color);
		unsigned char AddMaterial(Material* pMaterial);
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 1 Test Scene
	class Scene_W1 final : public Scene
	{
	public:
		Scene_W1() = default;
		~Scene_W1() override = default;

		Scene_W1(const Scene_W1&) = delete;
		Scene_W1(Scene_W1&&) noexcept = delete;
		Scene_W1& operator=(const Scene_W1&) = delete;
		Scene_W1& operator=(Scene_W1&&) noexcept = delete;

		void Initialize() override;
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 2 Test Scene 2
	class Scene_W2 final : public Scene
    {
    public:
        Scene_W2() = default;
        ~Scene_W2() override = default;

        Scene_W2(const Scene_W2&) = delete;
        Scene_W2(Scene_W2&&) noexcept = delete;
        Scene_W2& operator=(const Scene_W2&) = delete;
        Scene_W2& operator=(Scene_W2&&) noexcept = delete;

        void Initialize() override;
    };

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 3 Test Scene 3
	class Scene_W3 final : public Scene
	{
	public:
		Scene_W3() = default;
		~Scene_W3() override = default;

		Scene_W3(const Scene_W3&) = delete;
		Scene_W3(Scene_W3&&) noexcept = delete;
		Scene_W3& operator=(const Scene_W3&) = delete;
		Scene_W3& operator=(Scene_W3&&) noexcept = delete;

		void Initialize() override;
	};
}
