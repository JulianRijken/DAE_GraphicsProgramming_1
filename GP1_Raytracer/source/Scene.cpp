#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene():
		m_Materials({ new Material_SolidColor({1,0,0})})
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for(auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		HitRecord testHitRecord{};

		for (const Plane& plane : m_PlaneGeometries)
		{
			// Apply test to tesHitRecord
			GeometryUtils::HitTest_Plane(plane, ray, testHitRecord);

			if (testHitRecord.t < closestHit.t)
				closestHit = testHitRecord;
		}

		for (const Sphere& sphere : m_SphereGeometries)
		{
			// Apply test to tesHitRecord
			GeometryUtils::HitTest_Sphere(sphere, ray, testHitRecord);

			if (testHitRecord.t < closestHit.t)
				closestHit = testHitRecord;
		}

		for (const TriangleMesh& triangleMesh : m_TriangleMeshGeometries)
		{
			// Apply test to tesHitRecord
			GeometryUtils::HitTest_TriangleMesh(triangleMesh, ray, testHitRecord);

			if (testHitRecord.t < closestHit.t)
				closestHit = testHitRecord;
		}
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		HitRecord testHitRecord{};

		for (const Plane& plane : m_PlaneGeometries)
		{
			if(GeometryUtils::HitTest_Plane(plane, ray, testHitRecord,true))
				return true;
		}

		for (const Sphere& sphere : m_SphereGeometries)
		{
			if (GeometryUtils::HitTest_Sphere(sphere, ray, testHitRecord,true))
				return true;
		}

		for (const TriangleMesh& triangleMesh : m_TriangleMeshGeometries)
		{
			if (GeometryUtils::HitTest_TriangleMesh(triangleMesh, ray, testHitRecord,true))
				return true;
		}

		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal.Normalized();
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion


	void Scene_Bunny::Initialize()
	{
		sceneName = "Bunny";
		m_Camera.SetPosition({ 0,3,-9 });
		m_Camera.SetFOV(45.f);
		//m_Camera.SetRotation(-10.0f, 0.0f);

		// Materials
		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.0f));
		//const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.0f, 0.5f));

		GetMaterials()[matLambert_GrayBlue]->m_globalRoughness = 0.75f;
		GetMaterials()[matLambert_White]->m_globalRoughness = 1.0f;
		//GetMaterials()[matCT_GraySmoothMetal]->m_globalRoughness = 0.5f;

		// Walls
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		//AddPlane(Vector3{ 0.f, 0.f, -10.f }, Vector3{ 0.f, 0.f, 1.f }, matLambert_GrayBlue); //FRONT
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		m_Meshes.resize(1);

		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/lowpoly_bunny2.obj",
			m_Meshes[0]->positions,
			m_Meshes[0]->indices);

		m_Meshes[0]->Scale({ 2, 2, 2 });

		m_Meshes[0]->UpdateAABB();
		m_Meshes[0]->UpdateTransforms();


		//AddSphere(Vector3{ 2.9f, 1.0f, 0.5f }, 1.0f, matCT_GraySmoothMetal);


		// Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Backlight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}
	void Scene_Bunny::Update(dae::Timer* pTimer)
	{
		Scene::Update(pTimer);

		const float yawAngle{ (std::cos(pTimer->GetTotal()) + 1.0f) / 2.0f * PI_2 };
		for (TriangleMesh* triangleMesh : m_Meshes)
		{
			triangleMesh->RotateY(yawAngle);
			triangleMesh->UpdateTransforms();
		}
	}

	void Scene_Car::Initialize()
	{
		sceneName = "Car";
		//m_Camera.SetPosition({ 0.f, 1.9f, -7.f });
		//m_Camera.SetFOV(25.f);
		//m_Camera.SetRotation(-10.0f, 0.0f);

		//m_Camera.SetPosition({ -2.0f, 2.5f, -16.f });
		//m_Camera.SetFOV(12.f);
		//m_Camera.SetRotation(-5.0f, -7.0f);

		m_Camera.SetPosition({ -6.0f, 2.26f, -16.91f });
		m_Camera.SetFOV(12.0f);
		m_Camera.SetRotation(-0.0902665f * TO_DEGREES, -0.321173f * TO_DEGREES);

		// Materials
		const auto planesMaterial = AddMaterial(new Material_Lambert({ 0.7f, 0.8f, 0.7f }, 1.f));
		GetMaterials()[planesMaterial]->m_globalRoughness = 0.70f;

		const auto groundMaterial = AddMaterial(new Material_Lambert({ 1.0f, 1.0f, 1.0f }, 0.3f));
		GetMaterials()[groundMaterial]->m_globalRoughness = 0.3f;


		const auto modelMaterial = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 0.7f));
		GetMaterials()[modelMaterial]->m_globalRoughness = 0.8f;


		// Walls
		//AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, groundMaterial); //BACK
		//AddPlane(Vector3{ 0.f, 0.f, -10.f }, Vector3{ 0.f, 0.f, 1.f }, groundMaterial); //FRONT
		//AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, groundMaterial); //TOP
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, groundMaterial); //BOTTOM
		AddPlane(Vector3{ 7.f, 0.f, 7.f }, Vector3{ -0.6f, 0.f, -1.0f }, planesMaterial); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, planesMaterial); //LEFT

		m_Meshes.resize(1);

		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::NoCulling, modelMaterial);
		//Utils::ParseOBJ("Resources/lowpoly_bunny2.obj",
		//	m_Meshes[0]->positions,
		//	m_Meshes[0]->indices);

		Utils::ParseOBJ("Resources/car1.obj",
			m_Meshes[0]->positions,
			m_Meshes[0]->indices);

		m_Meshes[0]->Translate({ -1.48f,0,-3.5f });
		m_Meshes[0]->RotateY(160 * TO_RADIANS);
		m_Meshes[0]->Scale({0.97f,0.97f,0.97f});

		m_Meshes[0]->UpdateAABB();
		m_Meshes[0]->UpdateTransforms();

		// Lights
		AddPointLight(Vector3{ 0.f, 2.f, 5.f }, 20.0f,	 ColorRGB{ 0.8f, 0.6f, 0.45f }); //Backlight
		AddPointLight(Vector3{ -5.0f, 4.f, -7.f }, 100.f, ColorRGB{ 1.0f, 1.0f, 0.58f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 60.f, ColorRGB{ 0.24f, 0.57f, 0.78f });
	}

	void Scene_Raytracer::Initialize()
	{
		sceneName = "Week 3";
		m_Camera.SetPosition({ 0.f, 3.f, -9.f });
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
		GetMaterials()[matLambert_GrayBlue]->m_globalRoughness = 0.9f;
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));
		GetMaterials()[matLambert_White]->m_globalRoughness = 0.8f;

		AddPlane(Vector3{ 0.f, 0.f, -10.f }, Vector3{ 0.f, 0.f, 1.f }, matLambert_GrayBlue); //FRONT
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		//const auto matLambertPhong1 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 3.0f));
		//const auto matLambertPhong2 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 15.0f));
		//const auto matLambertPhong3 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f,50.0f));

		//AddSphere(Vector3{ -1.75f, 1.f, 0.f }, .75f, matLambertPhong1);
		//AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, matLambertPhong2);
		//AddSphere(Vector3{ 1.75f, 1.f, 0.f }, .75f, matLambertPhong3);

		//AddSphere(Vector3{ -1.75f, 5.f, 0.f }, .75f, matCT_GrayRoughMetal);
		//AddSphere(Vector3{ 0.f, 5.f, 0.f }, .75f, matCT_GrayMediumMetal);
		//AddSphere(Vector3{ 1.75f, 5.f, 0.f }, .75f, matCT_GraySmoothMetal);


		m_Meshes.resize(3);

		const Triangle baseTriangle = { Vector3(-.75f, 1.5f, 0.f), Vector3(.75f, 0.f, 0.f), Vector3(-.75f, 0.f, 0.f) };

		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		m_Meshes[0]->AppendTriangle(baseTriangle, true);
		m_Meshes[0]->Translate({ -1.75f,4.5f,0.f });
		m_Meshes[0]->UpdateAABB();
		m_Meshes[0]->UpdateTransforms();

		m_Meshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m_Meshes[1]->AppendTriangle(baseTriangle, true);
		m_Meshes[1]->Translate({ 0.f,4.5f,0.f });
		m_Meshes[1]->UpdateAABB();
		m_Meshes[1]->UpdateTransforms();

		m_Meshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m_Meshes[2]->AppendTriangle(baseTriangle, true);
		m_Meshes[2]->Translate({ 1.75f,4.5f,0.f });
		m_Meshes[2]->UpdateAABB();
		m_Meshes[2]->UpdateTransforms();





		// Spheres
		AddSphere(Vector3{ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);

		AddSphere(Vector3{ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//AddSphere(Vector3{ 0,0,0 }, 0.5f, matCT_GraySmoothPlastic);

		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Backlight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}
	void Scene_Raytracer::Update(dae::Timer* pTimer)
	{
		Scene::Update(pTimer);

		const float yawAngle{ (std::cos(pTimer->GetTotal()) + 1.0f) / 2.0f * PI_2 };
		for (TriangleMesh* triangleMesh : m_Meshes)
		{
			triangleMesh->RotateY(yawAngle);
			triangleMesh->UpdateTransforms();
		}

		//m_SphereGeometries[6].origin = m_Camera.origin;
	}


	void Scene_Testing::Initialize()
	{
		sceneName = "Bunny";
		m_Camera.SetPosition({ 0,3,-9 });
		m_Camera.SetFOV(85.f);
		//m_Camera.SetRotation(-10.0f, 0.0f);

		// Materials
		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
		GetMaterials()[matLambert_GrayBlue]->m_globalRoughness = 0.50f;

		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.0f));
		//const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.0f, 0.5f));

		GetMaterials()[matLambert_White]->m_globalRoughness = 1.0f;
		//GetMaterials()[matCT_GraySmoothMetal]->m_globalRoughness = 0.5f;

		// Walls
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, -10.f }, Vector3{ 0.f, 0.f, 1.f }, matLambert_GrayBlue); //FRONT
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		m_Meshes.resize(1);

		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/lowpoly_bunny2.obj",
			m_Meshes[0]->positions,
			m_Meshes[0]->indices);

		m_Meshes[0]->Scale({ 2, 2, 2 });

		m_Meshes[0]->UpdateAABB();
		m_Meshes[0]->UpdateTransforms();


		//AddSphere(Vector3{ 2.9f, 1.0f, 0.5f }, 1.0f, matCT_GraySmoothMetal);

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));

		AddSphere(Vector3{ -1.75f, 5.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.f, 5.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75f, 5.f, 0.f }, .75f, matCT_GraySmoothMetal);


		// Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Backlight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}
	void Scene_Testing::Update(dae::Timer* pTimer)
	{
		Scene::Update(pTimer);

		const float yawAngle{ (std::cos(pTimer->GetTotal()) + 1.0f) / 2.0f * PI_2 };
		for (TriangleMesh* triangleMesh : m_Meshes)
		{
			triangleMesh->RotateY(yawAngle);
			triangleMesh->UpdateTransforms();
		}

		//m_SphereGeometries[6].origin = m_Camera.origin;
	}
}
