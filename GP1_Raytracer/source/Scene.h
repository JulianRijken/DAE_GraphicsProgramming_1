#pragma once
#include <string>
#include <vector>

#include "Math.h"
#include "DataTypes.h"
#include "Camera.h"

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
		std::vector<Triangle> m_Triangles{};
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

	class Scene_Bunny final : public Scene
	{
	public:
		Scene_Bunny() = default;
		~Scene_Bunny() override = default;

		Scene_Bunny(const Scene_Bunny&) = delete;
		Scene_Bunny(Scene_Bunny&&) noexcept = delete;
		Scene_Bunny& operator=(const Scene_Bunny&) = delete;
		Scene_Bunny& operator=(Scene_Bunny&&) noexcept = delete;

		void Initialize() override;
		void Update(dae::Timer* pTimer) override;

		std::vector<TriangleMesh*> m_Meshes;
	};

	class Scene_Car final : public Scene
    {
    public:
        Scene_Car() = default;
        ~Scene_Car() override = default;

        Scene_Car(const Scene_Car&) = delete;
        Scene_Car(Scene_Car&&) noexcept = delete;
        Scene_Car& operator=(const Scene_Car&) = delete;
        Scene_Car& operator=(Scene_Car&&) noexcept = delete;

        void Initialize() override;

		std::vector<TriangleMesh*> m_Meshes;
    };

	class Scene_Raytracer final : public Scene
	{
	public:
		Scene_Raytracer() = default;
		~Scene_Raytracer() override = default;

		Scene_Raytracer(const Scene_Raytracer&) = delete;
		Scene_Raytracer(Scene_Raytracer&&) noexcept = delete;
		Scene_Raytracer& operator=(const Scene_Raytracer&) = delete;
		Scene_Raytracer& operator=(Scene_Raytracer&&) noexcept = delete;

		void Initialize() override;
		void Update(dae::Timer* pTimer) override;

		std::vector<TriangleMesh*> m_Meshes;
	};

	class Scene_Testing final : public Scene
	{
	public:
		Scene_Testing() = default;
		~Scene_Testing() override = default;

		Scene_Testing(const Scene_Testing&) = delete;
		Scene_Testing(Scene_Testing&&) noexcept = delete;
		Scene_Testing& operator=(const Scene_Testing&) = delete;
		Scene_Testing& operator=(Scene_Testing&&) noexcept = delete;

		void Initialize() override;
		void Update(dae::Timer* pTimer) override;

		std::vector<TriangleMesh*> m_Meshes;
	};
}
