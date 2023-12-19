//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <algorithm>
#include <cassert>
#include <execution>
#include <iostream>
#include <format>

#include "Camera.h"
#include "Maths.h"
#include "Mesh.h"
#include "Light.h"
#include "Texture.h"
#include "Utils.h"

//#define MULTI_THREAD_PIXELS
#define MULTI_THREAD_TRIANGLE
//#define DOUBLE_SIDED
//#define SORT_TRIANGLES
#define RENDER_OPACITY_CUTOUT

using namespace dae;

Renderer::Renderer(Camera* camera, SDL_Window* pWindow) :
m_WindowPtr(pWindow),
m_CameraPtr(camera),
m_RenderMode(DebugRenderMode::Combined),
m_ScreenWidth(), // Is set later
m_ScreenHeight(), // Is set later
m_HasToRotate(true),
m_UseNormalMap(true),
m_UseLinearDepth(true)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_ScreenWidth, &m_ScreenHeight);

	//Create Buffers
	m_FrontBufferPtr = SDL_GetWindowSurface(pWindow);
	m_BackBufferPtr = SDL_CreateRGBSurface(0, m_ScreenWidth, m_ScreenHeight, 32, 0, 0, 0, 0);
	m_BackBufferPixelsPtr = static_cast<uint32_t*>(m_BackBufferPtr->pixels);
	m_pDepthBufferPixels = new float[m_ScreenWidth * m_ScreenHeight];


#ifdef MULTI_THREAD_PIXELS
	const int maxPixelLength{ std::max(m_ScreenWidth, m_ScreenHeight) };
	m_Integers.resize(maxPixelLength);
	// fill with incrementing values
	std::iota(m_Integers.begin(), m_Integers.end(), 1);
#endif


	m_MaterialPtrMap.insert({ "default",new Material {
	}});
	//m_MaterialPtrMap["default"]->diffuseColor = ColorRGB{ 0.2f,0.2f,0.2f};
	m_MaterialPtrMap["default"]->diffuseColor = ColorRGB{ 1.0f,1.0f,1.0f};
	defaultMaterial = m_MaterialPtrMap["default"];

	m_MaterialPtrMap.insert({ "uvGrid",new Material {
	Texture::LoadFromFile("uv_grid_2.png"),
	}});



	//InitializeSceneAssignment();
	//InitializeSceneCar();
	InitializeSceneDioramaDay();
}

Renderer::~Renderer()
{
	for (const std::pair<const std::string, Material*>& pair : m_MaterialPtrMap)
	{
		if(pair.second == nullptr)
			continue;

		delete pair.second->diffuse;
		delete pair.second->opacity;
		delete pair.second->normal;
		delete pair.second->specular;
		delete pair.second->gloss;
		delete pair.second;
	}

	delete[] m_pDepthBufferPixels;
}


void Renderer::InitializeSceneAssignment()
{

	m_CameraPtr->SetFovAngle(45);
	m_CameraPtr->SetPosition(Vector3{ 0,5.0f,-64.0f });
	m_CameraPtr->SetNearClipping(0.1f);
	m_CameraPtr->SetFarClipping(100.0f);

	m_AmbientColor = { 0.03f,0.03f,0.03f };

	m_DiffuseStrengthKd = 7.0f;
	m_PhongExponentExp = 25.0f;
	m_SpecularKs = 1.0f;

	m_MaterialPtrMap.insert({ "bike",new Material {
		Texture::LoadFromFile("vehicle_diffuse.png"),
		nullptr,
		Texture::LoadFromFile("vehicle_normal.png"),
		Texture::LoadFromFile("vehicle_specular.png"),
		Texture::LoadFromFile("vehicle_gloss.png"),
	} });


	const Mesh bikeMesh("vehicle.obj", { m_MaterialPtrMap["bike"] });
	//bikeMesh.SetPosition({ 0, 0, 50 });
	m_WorldMeshes.push_back(bikeMesh);

	AddDirectionalLight({ 0.577f, -0.577f, 0.577f }, { 1,1,1 }, 1.0f);

}

void Renderer::InitializeSceneCar()
{
	m_CameraPtr->SetFovAngle(15);

	m_HasToRotate = false;

	m_CameraPtr->SetPosition(Vector3{ -53.4506f, 22.7297f, -118.892f });
	m_CameraPtr->SetPitch(-0.104893f);
	m_CameraPtr->SetYaw(-0.415f);


	m_CameraPtr->SetNearClipping(1);
	m_CameraPtr->SetFarClipping(500);

	m_AmbientColor = { 0,0,0};

	m_DiffuseStrengthKd = 1.8f;
	m_PhongExponentExp = 10.0f;
	m_SpecularKs = 0.5f;


	Mesh carMesh("Car/Car.obj", "Car/Car.mtl", m_MaterialPtrMap);
	carMesh.SetScale(Vector3{ 1,1,1 } * 15);
	carMesh.SetYawRotation(60.0f * TO_RADIANS);
	AddMesh(carMesh);


	m_MaterialPtrMap.insert({ "backdrop",new Material {
} });
	//m_MaterialPtrMap["default"]->diffuseColor = ColorRGB{ 0.2f,0.2f,0.2f};
	m_MaterialPtrMap["backdrop"]->diffuseColor = colors::White * 0.2f;
	defaultMaterial = m_MaterialPtrMap["backdrop"];

	Mesh backdrop("Backdrop.obj", {m_MaterialPtrMap["backdrop"]});
	backdrop.SetScale(Vector3{ 2,1,1 } * 11);
	AddMesh(backdrop);


	// Lights
	AddDirectionalLight({ 0.577f, -0.577f, 0.577f }, { 1,1,1},0.3f);
	AddPointLight({ -30.0f, 60, -40.0f }, colors::White, 3000.0f); // Front left light
	AddPointLight({ 40.0f, -10, -20.0f }, colors::White, 1500.0f); // Right bottom light
	AddPointLight({ 10, 30, 10.0f }, colors::White, 500.0f); // Back light


	//AddPointLight({ -10.1473f, 70.2765f, -61.4862f }, ColorRGB::Lerp(colors::White, { 1.0f, 1.0f, 0.58f }, 0.4f), 1000.0f); //Front Light Left
	//AddPointLight({  62.7225f, 47.0482f, -7.69772f }, ColorRGB::Lerp(colors::White, { 1.0f, 1.0f, 0.58f }, 0.4f), 1000.0f); //Front Light right
	//AddPointLight({ -11.7184f, 40.9686f,  57.3831f }, ColorRGB::Lerp(colors::White,{ 1.0f, 0.651f, 0.678f},0.4f), 800.0f); //Backlight
}

void Renderer::InitializeSceneDioramaDay()
{
	m_AmbientColor = { 0.001f,0.001f,0.001f };
	m_ClearColor = { 10 };
	AddDirectionalLight({ 0.577f, -0.577f, 0.577f }, { 0.8f,0.8f,1.0f }, 0.5f);



	AddPointLight({ 135.7012f, 267.001f, -158.2f }, { 1.0f,0.9f,0.7f }, 10000.0f); // Fake sun

	AddPointLight({-65.8307f, 163.166f, 129.75f}, { 1.0f,0.9f,0.7f }, 1000.0f); // Fake sun Back
	


	AddPointLight({ 118.023f, 70.663f, 103.019f }, colors::White, 2000.0f); // Car driving up

	//AddPointLight({ -40.9558f, 73.9197f, -104.5f }, colors::White, 40.0f); // Car driving up front
	//AddPointLight({ -40.2404f, 74.3321f, -88.2044f }, colors::White, 40.0f); // Car driving up front

	//AddPointLight({ 30.9558f, 73.9197f, -104.5f }, colors::Red, 10.0f); // Car driving up front
	//AddPointLight({ 30.2404f, 74.3321f, -88.2044f }, colors::Red, 10.0f); // Car driving up front

	AddPointLight({ -103.528f, 100.0746f, 10.4314f }, { 1.0f,0.7f,0.3f }, 700.0f); // Garage light


	m_DiffuseStrengthKd = 3.0f;
	m_PhongExponentExp = 20.0f;
	m_SpecularKs = 0.2f;

	m_CameraPtr->SetFovAngle(42);
	m_CameraPtr->SetPosition({ -167.749f, 158.555f, -359.077f });
	m_CameraPtr->SetPitch(-0.10f);
	m_CameraPtr->SetYaw(-0.40f);
	m_CameraPtr->SetNearClipping(5);
	m_CameraPtr->SetFarClipping(1000);

	m_SpinSpeed = 0.0f;

	Mesh diroama("Diorama/DioramaGP.obj", "Diorama/DioramaGP.mtl", m_MaterialPtrMap);
	diroama.SetScale(Vector3{ 1,1,1 } *15.0f);
	m_WorldMeshes.push_back(diroama);
}


void Renderer::AddMesh(const Mesh& mesh)
{
	m_WorldMeshes.emplace_back(mesh);
}

void Renderer::AddPointLight(const Vector3& origin, const ColorRGB& color, float intensity)
{
	m_WorldLights.emplace_back(Light(origin,{},color,intensity,LightType::Point));
}

void Renderer::AddDirectionalLight(const Vector3& direction, const ColorRGB& color, float intensity)
{
	m_WorldLights.emplace_back(Light({}, direction.Normalized(), color, intensity, LightType::Directional));
}



void Renderer::Update(const Timer& timer)
{
	if(m_HasToRotate && !m_WorldMeshes.empty())
		m_WorldMeshes[0].AddYawRotation(timer.GetElapsed() * m_SpinSpeed);
}

void Renderer::Render()
{
	//Lock BackBuffer
	SDL_LockSurface(m_BackBufferPtr);
	
	// Clear depth buffer
	std::fill_n(m_pDepthBufferPixels, m_ScreenWidth * m_ScreenHeight, std::numeric_limits<float>::max());
	// Clear screen buffer
	SDL_FillRect(m_BackBufferPtr, nullptr, SDL_MapRGB(m_BackBufferPtr->format, m_ClearColor, m_ClearColor, m_ClearColor));


	// Render all meshes
	for (Mesh& mesh : m_WorldMeshes)
		RasterizeMesh(mesh);
	

	//Update SDL Surface
	SDL_UnlockSurface(m_BackBufferPtr);
	SDL_BlitSurface(m_BackBufferPtr, 0, m_FrontBufferPtr, 0);
	SDL_UpdateWindowSurface(m_WindowPtr);
}


void Renderer::ToggleRotation()
{
	m_HasToRotate = !m_HasToRotate;
	
	std::cout << std::boolalpha << "Rotation Enabled -> " << m_HasToRotate << std::endl;
}

void Renderer::ToggleNormalMap()
{
	m_UseNormalMap = !m_UseNormalMap;
	std::cout << std::boolalpha << "Normal Map -> " << m_UseNormalMap << std::endl;
}

void Renderer::ToggleLinearDepth()
{
	m_UseLinearDepth = !m_UseLinearDepth;
	std::cout << std::boolalpha << "Linear Depth-> " << m_UseLinearDepth << std::endl;
}

void Renderer::SetRenderMode(DebugRenderMode mode)
{
	m_RenderMode = mode;

	std::cout << std::endl;
	std::cout << RENDER_MODE_NAMES.at(m_RenderMode).c_str() << std::endl;
	std::cout << std::endl;
}

void Renderer::CycleRenderMode()
{
	int current{ static_cast<int>(m_RenderMode) };
	current++;

	if (current >= static_cast<int>(DebugRenderMode::COUNT))
		current = 0;

	SetRenderMode(static_cast<DebugRenderMode>(current));
}


void Renderer::TransformMesh(Mesh& mesh) const
{
	// SPACES
	// - Model
	// - World
	// - world offset -> Camera -> projection
	// - Perspective divide
	// - NDC

	// For our translation
	// For the positions we go from model space -> NDC
	// For the normal and tangent we go from: model space -> world space (ONLY ROTATION SO MATRIX3)
	// For the view direction the camera is in world and we only need to translate the vertex from model -> world


	// Make sure the transformed vertices are back in the same state as the model vertices
	// This is done because we apply matrix transformations on the transformed vertices
	mesh.ResetTransformedVertices();

	const Matrix worldToViewProjectionMatrix = m_CameraPtr->m_InvViewMatrix * m_CameraPtr->m_ProjectionMatrix;

	for (const std::shared_ptr<VertexTransformed>& vertex : mesh.m_VerticesTransformed)
	{
		// Convert vertex to world
		vertex->pos = mesh.m_WorldMatrix.TransformPoint(vertex->pos);

		// Store world pos, this is used later on for lighting
		vertex->worldPos = vertex->pos.GetXYZ(); 

		// Convert normal to world
		// Note we use transform vector
		vertex->normal  = mesh.m_WorldMatrix.TransformVector(vertex->normal ).Normalized();
		vertex->tangent = mesh.m_WorldMatrix.TransformVector(vertex->tangent).Normalized();

		// Calculate view direction based on vertex in world
		vertex->viewDirection = (vertex->worldPos - m_CameraPtr->m_Origin).Normalized();

		// Transform vertex to view
		vertex->pos = worldToViewProjectionMatrix.TransformPoint(vertex->pos);

		// Apply perspective divide  
		vertex->pos.x /= vertex->pos.w;
		vertex->pos.y /= vertex->pos.w;
		vertex->pos.z /= vertex->pos.w;

		vertex->pos.z = 1.0f / vertex->pos.z;

		// Convert from NDC to screen
		vertex->pos.x = (vertex->pos.x + 1.0f) / 2.0f * static_cast<float>(m_ScreenWidth);
		vertex->pos.y = (1.0f - vertex->pos.y) / 2.0f * static_cast<float>(m_ScreenHeight);
	}
}


void Renderer::RasterizeMesh(Mesh& mesh) const
{
	TransformMesh(mesh);

#ifdef SORT_TRIANGLES
	auto compareTriangles = [](const Triangle& triangle1, const Triangle& triangle2) {
		// Assuming Vector4 has a member pos.z

		const float triangle1Z
		{
				triangle1.vertex0->pos.z +
				triangle1.vertex1->pos.z +
				triangle1.vertex2->pos.z 
		};

		const float triangle2Z
		{
				triangle2.vertex0->pos.z +
				triangle2.vertex1->pos.z +
				triangle2.vertex2->pos.z
		};

		return triangle1Z >= triangle2Z;

		};

	std::ranges::sort(mesh.m_Triangles, compareTriangles);
#endif

#ifdef MULTI_THREAD_TRIANGLE
	std::for_each(std::execution::par, mesh.m_Triangles.begin(), mesh.m_Triangles.end(), [this,mesh](const Triangle& triangle)
		{
			RasterizeTriangle(triangle, mesh.m_MaterialPtrs);
		});
#else
	for (Triangle& triangle : mesh.m_Triangles)
		RasterizeTriangle(triangle, mesh.m_MaterialPtrs);
#endif
}

void Renderer::RasterizeTriangle(const Triangle& triangle, const std::vector<Material*>& materialPtrs) const
{
	// Checking normal early for more performance
	const Vector3 normal = Vector3::Cross
	(
		triangle.vertex1->pos - triangle.vertex0->pos,
		triangle.vertex2->pos - triangle.vertex0->pos
	);

#ifndef DOUBLE_SIDED
	if (normal.z <= 0.0f)
		return;
#endif


	if (triangle.vertex0->pos.w < 0.0f) return;
	if (triangle.vertex1->pos.w < 0.0f) return;
	if (triangle.vertex2->pos.w < 0.0f) return;


	// Adding the 1 pixel is done to prevent gaps in the triangles
	constexpr int boundingBoxPadding{1};
	int minX = static_cast<int>(std::min(triangle.vertex0->pos.x, std::min(triangle.vertex1->pos.x, triangle.vertex2->pos.x))) - boundingBoxPadding;
	int maxX = static_cast<int>(std::max(triangle.vertex0->pos.x, std::max(triangle.vertex1->pos.x, triangle.vertex2->pos.x))) + boundingBoxPadding;
	int minY = static_cast<int>(std::min(triangle.vertex0->pos.y, std::min(triangle.vertex1->pos.y, triangle.vertex2->pos.y))) - boundingBoxPadding;
	int maxY = static_cast<int>(std::max(triangle.vertex0->pos.y, std::max(triangle.vertex1->pos.y, triangle.vertex2->pos.y))) + boundingBoxPadding;

	// Clamping is done so that the triangle is not rendered off the screen
	minX = std::ranges::clamp(minX, 0, m_ScreenWidth);
	maxX = std::ranges::clamp(maxX, 0, m_ScreenWidth);
	minY = std::ranges::clamp(minY, 0, m_ScreenHeight);
	maxY = std::ranges::clamp(maxY, 0, m_ScreenHeight);


	float signedAreaW0;
	float signedAreaW1;
	float signedAreaW2;

	// Looping all pixels within the bounding box
	// This is done for optimization
#ifdef MULTI_THREAD_PIXELS

	std::for_each(std::execution::par, m_Integers.begin() + minX, m_Integers.begin() + maxX, [&](uint32_t pixelX)
		{
#else
	for (int pixelX{ minX }; pixelX < maxX; pixelX++)
	{
#endif
		for (int pixelY{ minY }; pixelY < maxY; pixelY++)
		{

			const Vector2 pixelCenter{ static_cast<float>(pixelX) + 0.5f,static_cast<float>(pixelY) + 0.5f };
			const int pixelIndex = pixelX + pixelY * m_ScreenWidth;


#ifdef DOUBLE_SIDED
			if (normal.z > 0.0f)
			{
				signedAreaW0 = Vector2::Cross(pixelCenter - triangle.vertex1->pos.GetXY(), triangle.vertex2->pos.GetXY() - triangle.vertex1->pos.GetXY());
				if (signedAreaW0 >= 0) continue;
				signedAreaW1 = Vector2::Cross(pixelCenter - triangle.vertex2->pos.GetXY(), triangle.vertex0->pos.GetXY() - triangle.vertex2->pos.GetXY());
				if (signedAreaW1 >= 0) continue;
				signedAreaW2 = Vector2::Cross(pixelCenter - triangle.vertex0->pos.GetXY(), triangle.vertex1->pos.GetXY() - triangle.vertex0->pos.GetXY());
				if (signedAreaW2 >= 0) continue;
			}
			else
			{
				signedAreaW0 = Vector2::Cross(triangle.vertex2->pos.GetXY() - triangle.vertex1->pos.GetXY(), pixelCenter - triangle.vertex1->pos.GetXY());
				if (signedAreaW0 >= 0) continue;
				signedAreaW1 = Vector2::Cross(triangle.vertex0->pos.GetXY() - triangle.vertex2->pos.GetXY(), pixelCenter - triangle.vertex2->pos.GetXY());
				if (signedAreaW1 >= 0) continue;
				signedAreaW2 = Vector2::Cross(triangle.vertex1->pos.GetXY() - triangle.vertex0->pos.GetXY(), pixelCenter - triangle.vertex0->pos.GetXY());
				if (signedAreaW2 >= 0) continue;
			}
#else
			signedAreaW0 = Vector2::Cross(pixelCenter - triangle.vertex1->pos.GetXY(), triangle.vertex2->pos.GetXY() - triangle.vertex1->pos.GetXY());
			if (signedAreaW0 >= 0) continue;
			signedAreaW1 = Vector2::Cross(pixelCenter - triangle.vertex2->pos.GetXY(), triangle.vertex0->pos.GetXY() - triangle.vertex2->pos.GetXY());
			if (signedAreaW1 >= 0) continue;
			signedAreaW2 = Vector2::Cross(pixelCenter - triangle.vertex0->pos.GetXY(), triangle.vertex1->pos.GetXY() - triangle.vertex0->pos.GetXY());
			if (signedAreaW2 >= 0) continue;
#endif

			// Get total area before
			const float totalArea = signedAreaW0 + signedAreaW1 + signedAreaW2;
			const Vector3 weights
			{
				signedAreaW0 / totalArea,
				signedAreaW1 / totalArea,
				signedAreaW2 / totalArea,
			};

			const float nonLinearDepth = (
				weights.x / triangle.vertex0->pos.z +
				weights.y / triangle.vertex1->pos.z +
				weights.z / triangle.vertex2->pos.z);


			// Don't cull when showing depth buffer
			if (m_RenderMode != DebugRenderMode::DepthBuffer)
				if (nonLinearDepth < 0.0f or nonLinearDepth > 1.0f)
					continue;


			const Material* material = defaultMaterial;

			if (!materialPtrs.empty())
			{
				if(const Material* materialAtIndex = materialPtrs[triangle.vertex0->materialIndex])
					material = materialAtIndex;
			}


#ifdef RENDER_OPACITY_CUTOUT

			if (material->opacity)
			{
				const float linearPixelDepth = 1.0f / (
					weights.x / triangle.vertex0->pos.w +
					weights.y / triangle.vertex1->pos.w +
					weights.z / triangle.vertex2->pos.w);


				const Vector2 uv = linearPixelDepth * (
					triangle.vertex0->uv / triangle.vertex0->pos.w * weights.x +
					triangle.vertex1->uv / triangle.vertex1->pos.w * weights.y +
					triangle.vertex2->uv / triangle.vertex2->pos.w * weights.z);


				ColorRGB opacityMask = material->opacity->Sample(uv);
				const float alpha = std::ranges::clamp(opacityMask.r, 0.0f, 1.0f);

				if (alpha < 0.75f)
					continue;
			}
#endif


			// Depth check
			if (nonLinearDepth > m_pDepthBufferPixels[pixelIndex]) continue;
			m_pDepthBufferPixels[pixelIndex] = nonLinearDepth;


			const float linearPixelDepth = 1.0f / (
				weights.x / triangle.vertex0->pos.w +
				weights.y / triangle.vertex1->pos.w +
				weights.z / triangle.vertex2->pos.w);

			const Vector2 interpUV = linearPixelDepth * (
				triangle.vertex0->uv / triangle.vertex0->pos.w * weights.x +
				triangle.vertex1->uv / triangle.vertex1->pos.w * weights.y +
				triangle.vertex2->uv / triangle.vertex2->pos.w * weights.z);

			const Vector3 interpNormal = linearPixelDepth * (
				triangle.vertex0->normal / triangle.vertex0->pos.w * weights.x +
				triangle.vertex1->normal / triangle.vertex1->pos.w * weights.y +
				triangle.vertex2->normal / triangle.vertex2->pos.w * weights.z);

			const Vector3 interpTangent = linearPixelDepth * (
				triangle.vertex0->tangent / triangle.vertex0->pos.w * weights.x +
				triangle.vertex1->tangent / triangle.vertex1->pos.w * weights.y +
				triangle.vertex2->tangent / triangle.vertex2->pos.w * weights.z);

			const Vector3 interpViewDirection = linearPixelDepth * (
				triangle.vertex0->viewDirection / triangle.vertex0->pos.w * weights.x +
				triangle.vertex1->viewDirection / triangle.vertex1->pos.w * weights.y +
				triangle.vertex2->viewDirection / triangle.vertex2->pos.w * weights.z);

			const Vector3 interpPixelPosition = linearPixelDepth * (
				triangle.vertex0->worldPos / triangle.vertex0->pos.w * weights.x +
				triangle.vertex1->worldPos / triangle.vertex1->pos.w * weights.y +
				triangle.vertex2->worldPos / triangle.vertex2->pos.w * weights.z);

			const ColorRGB interpVertexColor = 
				triangle.vertex0->color* weights.x +
				triangle.vertex1->color * weights.y +
				triangle.vertex2->color * weights.z;

			// Will be called inline to avoid passing all parameters
			ShadePixel
			(
				material,
				triangle.vertex0->materialIndex,
				pixelIndex,
				interpVertexColor,
				interpUV,
				interpNormal,
				interpTangent,
				interpViewDirection,
				interpPixelPosition,
				nonLinearDepth
			);
		}
#ifdef MULTI_THREAD_PIXELS
		});
#else
	}
#endif

}

void Renderer::ShadePixel(const Material* material, int materialIndex, int pixelIndex, ColorRGB vertexColor, Vector2 uv,
                          Vector3 normal, Vector3 tangent, Vector3 viewDirection, Vector3 pixelPosition, float nonLinearDepth) const
{

	// Create locals for sampling
	Vector3 sampledNormal{ normal };
	float sampledSpecular{ m_SpecularKs }; 
	float sampledPhongExponent{ m_PhongExponentExp };
	ColorRGB sampledDiffuseColor{material->diffuseColor}; // cd
	ColorRGB sampledOpacity{0,0,0};


	if (material->opacity)
		sampledOpacity = material->opacity->Sample(uv);

	if (material->specular)
		sampledSpecular *= material->specular->Sample(uv).r; // NOTE Only using R

	if (material->gloss)
		sampledPhongExponent *= material->gloss->Sample(uv).r; // NOTE Only using R

	if (material->diffuse)
		sampledDiffuseColor = material->diffuse->Sample(uv);

	if (material->normal && m_UseNormalMap)
	{
		const Matrix tangentSpaceAxis =
		{
			tangent,
			Vector3::Cross(normal, tangent),
			normal,
			Vector3::Zero
		};

		const ColorRGB sampledNormalColor = material->normal->Sample(uv);
		const Vector3 sampledNormalMapped
		{
			2.0f * sampledNormalColor.r - 1.0f,
			2.0f * sampledNormalColor.g - 1.0f,
			2.0f * sampledNormalColor.b - 1.0f
		};

		sampledNormal = tangentSpaceAxis.TransformPoint(sampledNormalMapped);
	}

	ColorRGB finalPixelColor{};
	for (const Light& light : m_WorldLights)
	{
		Vector3 lightDirection{};

		if (light.GetType() == LightType::Point)
			lightDirection = (pixelPosition - light.GetOrigin()).Normalized();
		else if (light.GetType() == LightType::Directional)
			lightDirection = light.GetDirection();
		


		// Get lambert diffuse
		ColorRGB lambertDiffuse = sampledDiffuseColor * m_DiffuseStrengthKd / PI;

		// Get Cosine Law
		const float observedArea = std::max(0.0f, Vector3::Dot(sampledNormal, -lightDirection));

		// Get Specular Intensity
		const Vector3 reflectedRay = Vector3::Reflect(lightDirection, sampledNormal);
		const float cosAlpha{ std::max(Vector3::Dot(reflectedRay,-viewDirection),0.0f) };
		const float specularIntensity{ sampledSpecular * std::powf(cosAlpha,sampledPhongExponent) };


		switch (m_RenderMode)
		{
		case DebugRenderMode::Diffuse:
		{
			finalPixelColor = lambertDiffuse;

		} break;
		case DebugRenderMode::ObservedArea:
		{
			finalPixelColor += colors::White * observedArea;
		}break;
		case DebugRenderMode::DiffuseOA:
		{
			finalPixelColor += lambertDiffuse * observedArea;
		}break;
		case DebugRenderMode::SpecularOA:
		{
			finalPixelColor += specularIntensity * colors::White * observedArea;
		}break;
		case DebugRenderMode::Combined:
		{
			finalPixelColor += light.GetRadiance(pixelPosition) * ((specularIntensity * colors::White + lambertDiffuse) * observedArea) + m_AmbientColor;
		} break;
		case DebugRenderMode::UVColor:
		{
			finalPixelColor = m_MaterialPtrMap.at("uvGrid")->diffuse->Sample(uv);
		} break;
		case DebugRenderMode::Weights:
		{
			finalPixelColor = vertexColor;

		} break;
		case DebugRenderMode::DepthBuffer:
		{
			if (nonLinearDepth < 0.0f)
			{
				finalPixelColor = colors::Red;
			}
			else
			{
				if (nonLinearDepth > 1.0f)
					finalPixelColor = colors::Blue;
				else
					finalPixelColor = colors::Green * nonLinearDepth;
			}
		}break;
		case DebugRenderMode::MaterialIndex:
		{
			srand(materialIndex);

			ColorRGB color
			{
				(std::rand() * (materialIndex + 1) % 255) / 255.0f,
				(std::rand() * (materialIndex + 1) % 255) / 255.0f,
				(std::rand() * (materialIndex + 1) % 255) / 255.0f,
			};

			finalPixelColor = color;
		} break;
		case DebugRenderMode::Opacity:
		{
			finalPixelColor = sampledOpacity;
		} break;
		case DebugRenderMode::LightRadiance:
		{
			finalPixelColor += light.GetRadiance(pixelPosition);
		} break;
		}
	}

	//Update Color in Buffer
	finalPixelColor.MaxToOne();
	m_BackBufferPixelsPtr[pixelIndex] = SDL_MapRGB(m_BackBufferPtr->format,
		static_cast<uint8_t>(finalPixelColor.r * 255),
		static_cast<uint8_t>(finalPixelColor.g * 255),
		static_cast<uint8_t>(finalPixelColor.b * 255));
}


bool Renderer::SaveBufferToImage() const
{
	const std::string filename = std::format("Screenshot/Rasterizer_{}.bmp", RENDER_MODE_NAMES.at(m_RenderMode));
	return 	SDL_SaveBMP(m_BackBufferPtr, filename.c_str());
	
}


