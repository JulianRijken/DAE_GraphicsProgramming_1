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

		//for (const Triangle& triangle : m_Triangles)
		//{
		//	// Apply test to tesHitRecord
		//	GeometryUtils::HitTest_Triangle(triangle, ray, testHitRecord);

		//	if (testHitRecord.t < closestHit.t)
		//		closestHit = testHitRecord;
		//}
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		HitRecord testHitRecord{};

		for (const Plane& plane : m_PlaneGeometries)
		{
			// Apply test to tesHitRecord
			GeometryUtils::HitTest_Plane(plane, ray, testHitRecord);

			if (testHitRecord.didHit)
				return true;
		}

		for (const Sphere& sphere : m_SphereGeometries)
		{
			// Apply test to tesHitRecord
			GeometryUtils::HitTest_Sphere(sphere, ray, testHitRecord);

			if (testHitRecord.didHit)
				return true;
		}

		for (const TriangleMesh& triangleMesh : m_TriangleMeshGeometries)
		{
			// Apply test to tesHitRecord
			GeometryUtils::HitTest_TriangleMesh(triangleMesh, ray, testHitRecord);

			if (testHitRecord.didHit)
				return true;
		}

		//for (const Triangle& triangle : m_Triangles)
		//{
		//	// Apply test to tesHitRecord
		//	GeometryUtils::HitTest_Triangle(triangle, ray, testHitRecord);

		//	if (testHitRecord.didHit)
		//		return true;
		//}

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
		p.normal = normal;
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



#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
				//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		//AddSphere({ -25.f, -10.f, 110.f }, 50.f, matId_Solid_Red);
		//AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);
		//AddSphere({ -15.f, 30.f, 120.f }, 50.f, matId_Solid_Green);


		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
#pragma endregion


#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.targetOrigin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });
		//Plane

		AddPlane({ -5.f,0.0f,0.0f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 5,0.0f,0.0f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.0f,0.0f,0.0f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.0f,10.0f,0.0f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.0f,0.0f,10.0f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f, 1.f,  0.f }, 0.75f, matId_Solid_Red);
		AddSphere({ 0.0f, 1.f,  0.f }, 0.75f, matId_Solid_Blue);
		AddSphere({ 1.75f, 1.f,  0.f }, 0.75f, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f,  0.f }, 0.75f, matId_Solid_Blue);
		AddSphere({ 0, 3.f,  0.f }, 0.75f, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f,  0.f }, 0.75f, matId_Solid_Blue);
		//Light

		AddPointLight({ 0.f,5.f,-5.f }, 70.f, colors::White);
	}
#pragma endregion

#pragma region SCENE W3
	void Scene_W3::Initialize()
	{
		sceneName = "Week 3";
		m_Camera.targetOrigin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

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

		// Spheres
		AddSphere(Vector3{ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);

		AddSphere(Vector3{ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);



		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Backlight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });



		//m_Camera.targetOrigin = { 0.f, 1.f, -5.f };
		//m_Camera.fovAngle = 45.f;

		////default: Material id0 >> SolidColor Material (RED)
		//constexpr unsigned char matId_Solid_Red = 0;
		//const unsigned char matId_Solid_Blue = AddMaterial(new Material_LambertPhong{ colors::Blue,1.0f,1.0f,60.0f });
		//const unsigned char matId_Solid_Yellow = AddMaterial(new Material_Lambert{ colors::White, 1.f });

		////Spheres
		//AddSphere({ -.75f, 1.f, .0f }, 1.f, matId_Solid_Red);
		//AddSphere({ .75f, 1.f, .0f }, 1.f, matId_Solid_Blue);

		////Plane
		//AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matId_Solid_Yellow);

		////Light
		//AddPointLight({ 0.f, 5.f, 5.f }, 25.f, colors::White);


		//AddPointLight({ 0.f, 2.5f, -5.f }, 25.f, colors::White);
	}
#pragma endregion

#pragma region SCENE W4
	void Scene_W4::Initialize()
	{
		//sceneName = "Week 4";
		//m_Camera.targetOrigin = { 0.f, 1.f, -5.f };
		//m_Camera.fovAngle = 45.f;


		////Materials
		//const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
		//const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		////Planes
		//AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		//AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		//AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		//AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		//AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		////Triangle (Temp)
		////===============
		//auto triangle = Triangle{ {-.75f,.5f,.0f},{-.75f,2.f, .0f}, {.75f,.5f,0.f} };
		//triangle.cullMode = TriangleCullMode::NoCulling;
		//triangle.materialIndex = matLambert_White;
		//m_Triangles.emplace_back(triangle);

		//Triangle Mesh
		//=============
		//m_TestMeshPtr = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		//m_TestMeshPtr->positions = 
		//{
		//	{-.75f,-1.f,.0f},  //V0
		//	{-.75f,1.f, .0f},  //V2
		//	{.75f,1.f,1.f},    //V3
		//	{.75f,-1.f,0.f} }; //V4

		//m_TestMeshPtr->indices = 
		//{
		//	0,1,2, //Triangle 1
		//	0,2,3  //Triangle 2
		//};

		//m_TestMeshPtr->CalculateNormals();

		//m_TestMeshPtr->Translate({ 0.f,1.5f,0.f });
		//m_TestMeshPtr->UpdateTransforms();

		////OBJ
		////===
		//m_TestMeshPtr = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		//Utils::ParseOBJ("Resources/simple_cube.obj",
		////Utils::ParseOBJ("Resources/simple_object.obj",
		//	m_TestMeshPtr->positions,
		//	m_TestMeshPtr->normals,
		//	m_TestMeshPtr->indices);

		////No need to Calculate the normals, these are calculated inside the ParseOBJ function
		//m_TestMeshPtr->UpdateTransforms();

		//m_TestMeshPtr->Scale({ .7f,.7f,.7f });
		//m_TestMeshPtr->Translate({ .0f,1.f,0.f });


		////Light
		//AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Backlight
		//AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		//AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });





		sceneName = "Week 4";
		m_Camera.targetOrigin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;



		//const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		//const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		//const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		//const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, 1.f));
		//const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .6f));
		//const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		//AddSphere(Vector3{ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		//AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		//AddSphere(Vector3{ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		//AddSphere(Vector3{ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		//AddSphere(Vector3{ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		//AddSphere(Vector3{ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);





		//CW Winding Order!
		//const Triangle baseTriangle = { Vector3(-.75f, 1.5f, 0.f), Vector3(.75f, 0.f, 0.f), Vector3(-.75f, 0.f, 0.f) };

		//m_Meshes.resize(3);

		//m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		//m_Meshes[0]->AppendTriangle(baseTriangle, true);
		//m_Meshes[0]->Translate({ -1.75f,4.5f,0.f });
		//m_Meshes[0]->UpdateTransforms();

		//m_Meshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		//m_Meshes[1]->AppendTriangle(baseTriangle, true);
		//m_Meshes[1]->Translate({ 0.f,4.5f,0.f });
		//m_Meshes[1]->UpdateTransforms();

		//m_Meshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		//m_Meshes[2]->AppendTriangle(baseTriangle, true);
		//m_Meshes[2]->Translate({ 1.75f,4.5f,0.f });
		//m_Meshes[2]->UpdateTransforms();


		//OBJ
		//===
		TriangleMesh* m_TestMeshPtr = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/lowpoly_bunny.obj",
			//Utils::ParseOBJ("Resources/simple_object.obj",
			m_TestMeshPtr->positions,
			m_TestMeshPtr->normals,
			m_TestMeshPtr->indices);

		//No need to Calculate the normals, these are calculated inside the ParseOBJ function
		m_TestMeshPtr->UpdateTransforms();

		//m_Meshes[0]->Scale({ .7f,.7f,.7f });
		//m_Meshes[0]->Translate({ .0f,1.f,0.f });


		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Backlight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}
#pragma endregion


}
