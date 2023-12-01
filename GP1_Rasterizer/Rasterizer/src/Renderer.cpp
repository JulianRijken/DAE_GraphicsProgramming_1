//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <algorithm>
#include <cassert>
#include <execution>
#include <iostream>

#include "Camera.h"
#include "Maths.h"
#include "Mesh.h"
#include "Texture.h"
#include "Utils.h"

//#define MULTI_THREAD_TRANSFORM
//#define MULTI_THREAD_TRIANGLE

using namespace dae;

Renderer::Renderer(Camera* camera, SDL_Window* pWindow) :
m_WindowPtr(pWindow),
m_CameraPtr(camera),
m_RenderMode(DebugRenderMode::Combined),
m_ScreenWidth(), // Is set later
m_ScreenHeight(), // Is set later
m_HasToRotate(true),
m_UseNormalMap(true)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_ScreenWidth, &m_ScreenHeight);

	//Create Buffers
	m_FrontBufferPtr = SDL_GetWindowSurface(pWindow);
	m_BackBufferPtr = SDL_CreateRGBSurface(0, m_ScreenWidth, m_ScreenHeight, 32, 0, 0, 0, 0);
	m_BackBufferPixelsPtr = static_cast<uint32_t*>(m_BackBufferPtr->pixels);
	m_pDepthBufferPixels = new float[m_ScreenWidth * m_ScreenHeight];

	InitializeMaterials();
	InitializeScene();
}

Renderer::~Renderer()
{
	for (const std::pair<const std::string, Material*>& materialPtrMap : m_MaterialPtrMap)
	{
		delete materialPtrMap.second->diffuse;
		delete materialPtrMap.second->opacity;
		delete materialPtrMap.second->normal;
		delete materialPtrMap.second->specular;
		delete materialPtrMap.second->gloss;
		delete materialPtrMap.second;
	}

	delete[] m_pDepthBufferPixels;
}

void Renderer::InitializeMaterials()
{
	m_MaterialPtrMap.insert({ "uvGrid2",new Material {
		Texture::LoadFromFile("Resources/uv_grid_2.png"),
	}});


	m_MaterialPtrMap.insert({ "uvGrid3",new Material {
		Texture::LoadFromFile("Resources/uv_grid_3.png"),
	}});



	m_MaterialPtrMap.insert({ "carBody",new Material {
		Texture::LoadFromFile("Resources/Car/Tex_FordGT40_Color_2k_02_Clean.png"),
		Texture::LoadFromFile("Resources/Car/Tex_FordGT40_Opacity_2k_02.png")
	}});

	m_MaterialPtrMap.insert({ "carWheel",new Material {
		Texture::LoadFromFile("Resources/Car/Tex_TireAndRim_Color_1k_02.png"),
	}});


	//m_MaterialPtrMap.insert({ "tukTuk",new Material {
	//	Texture::LoadFromFile("Resources/tuktuk.png"),
	//} });


	m_MaterialPtrMap.insert({"bike",new Material {
		Texture::LoadFromFile("Resources/vehicle_diffuse.png"),
		nullptr,
		Texture::LoadFromFile("Resources/vehicle_normal.png"),
		Texture::LoadFromFile("Resources/vehicle_specular.png"),
		Texture::LoadFromFile("Resources/vehicle_gloss.png"),
	}});





	m_MaterialPtrMap.insert({"branches",new Material {
		Texture::LoadFromFile("Resources/Diorama/T_Leaves_Color.png"),
} });





}

void Renderer::InitializeScene()
{

	Mesh bikeMesh("Resources/vehicle.obj", { m_MaterialPtrMap["bike"] });
	bikeMesh.SetPosition({ 0, 0, 50 });
	m_WorldMeshes.push_back(bikeMesh);


	Mesh carMesh("Resources/Car/car2.obj", { m_MaterialPtrMap["carBody"],m_MaterialPtrMap["carWheel"] });
	carMesh.SetPosition({ 45, -7, 50 });
	carMesh.SetScale(Vector3{1,1,1} * 15.0f);
	m_WorldMeshes.push_back(carMesh);

	Mesh diroama("Resources/Diorama2.obj", { m_MaterialPtrMap["branches"] });
	diroama.SetPosition({ 100, 0, 400 });
	diroama.SetScale(Vector3{ 1,1,1 } * 15.0f);
	m_WorldMeshes.push_back(diroama);
}


void Renderer::Update(const Timer& timer)
{
	if(m_HasToRotate)
		m_WorldMeshes[0].AddYawRotation(timer.GetElapsed() * 0.5f);
}

void Renderer::Render()
{
	//Lock BackBuffer
	SDL_LockSurface(m_BackBufferPtr);

	// Clear depth buffer
	std::fill_n(m_pDepthBufferPixels, m_ScreenWidth * m_ScreenHeight, std::numeric_limits<float>::max());
	// Clear screen buffer
	constexpr int color{ 20 };
	SDL_FillRect(m_BackBufferPtr, nullptr, SDL_MapRGB(m_BackBufferPtr->format, color, color, color));


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
	// For the postions we go from model space -> NDC
	// For the normal and trangent we go from: model space -> world space (ONLY ROTATION SO MATRIX3)
	// For the view direction the camera is in world and we only need to translate the vertex from model -> world


	// Make sure the transformed vertices are back in the same state as the model vertices
	// This is done because we apply matrix transformations on the transformed vertices
	mesh.ResetTransformedVertices();

	const Matrix worldToViewProjectionMatrix = m_CameraPtr->m_InvViewMatrix * m_CameraPtr->m_ProjectionMatrix;

#ifdef MULTI_THREAD_TRANSFORM
	std::for_each(std::execution::par, mesh.m_VerticesTransformed.begin(), mesh.m_VerticesTransformed.end(), [this, mesh, worldToViewProjectionMatrix](const std::shared_ptr<VertexTransformed>& vertex)
		{
#else
	for (const std::shared_ptr<VertexTransformed>& vertex : mesh.m_VerticesTransformed)
	{
#endif

		// Convert vertex to world
		vertex->pos = mesh.m_WorldMatrix.TransformPoint(vertex->pos);

		// Convert normal to world
		// Note we use transform vector
		vertex->normal  = mesh.m_WorldMatrix.TransformVector(vertex->normal ).Normalized();
		vertex->tangent = mesh.m_WorldMatrix.TransformVector(vertex->tangent).Normalized();

		// Calculate view direction based on vertex in world
		vertex->viewDirection = (vertex->pos.GetXYZ() - m_CameraPtr->m_Origin).Normalized();

		// Transform vertex to view
		vertex->pos = worldToViewProjectionMatrix.TransformPoint(vertex->pos);

		// Apply perspective divide  
		vertex->pos.x /= vertex->pos.w;
		vertex->pos.y /= vertex->pos.w;
		vertex->pos.z /= vertex->pos.w;

		// Convert from NDC to screen
		vertex->pos.x = (vertex->pos.x + 1.0f) / 2.0f * static_cast<float>(m_ScreenWidth);
		vertex->pos.y = (1.0f - vertex->pos.y) / 2.0f * static_cast<float>(m_ScreenHeight);

#ifdef MULTI_THREAD_TRANSFORM
		});
#else
		}
#endif

}


void Renderer::RasterizeMesh(Mesh& mesh) const
{
	TransformMesh(mesh);

#ifdef MULTI_THREAD_TRIANGLE
	std::for_each(std::execution::par, mesh.m_Triangles.begin(), mesh.m_Triangles.end(), [this,mesh](Triangle& triangle)
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
	// early out culling
	if (triangle.vertex0->pos.z < 0.0f or triangle.vertex0->pos.z > 1.0f and
		triangle.vertex1->pos.z < 0.0f or triangle.vertex1->pos.z > 1.0f and
		triangle.vertex2->pos.z < 0.0f or triangle.vertex2->pos.z > 1.0f) return;

	if (triangle.vertex0->pos.w < 0.0f) return;
	if (triangle.vertex1->pos.w < 0.0f) return;
	if (triangle.vertex2->pos.w < 0.0f) return;


	// Checking normal early for more performance
	const Vector3 normal = Vector3::Cross
	(
		triangle.vertex1->pos - triangle.vertex0->pos,
		triangle.vertex2->pos - triangle.vertex0->pos
	);

	if (normal.z <= 0.0f)
		return;

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


	// Looping all pixels within the bounding box
	// This is done for optimization
	for (int pixelX{ minX }; pixelX < maxX; pixelX++)
	{
		for (int pixelY{ minY }; pixelY < maxY; pixelY++)
		{
			const Vector2 pixelCenter{ static_cast<float>(pixelX) + 0.5f,static_cast<float>(pixelY) + 0.5f };

			const float signedAreaW0 = Vector2::Cross(pixelCenter - triangle.vertex1->pos.GetXY(),triangle.vertex2->pos.GetXY() - triangle.vertex1->pos.GetXY());
			if (signedAreaW0 >= 0) continue;
			const float signedAreaW1 = Vector2::Cross(pixelCenter - triangle.vertex2->pos.GetXY(),triangle.vertex0->pos.GetXY() - triangle.vertex2->pos.GetXY());
			if (signedAreaW1 >= 0) continue;
			const float signedAreaW2 = Vector2::Cross(pixelCenter - triangle.vertex0->pos.GetXY(),triangle.vertex1->pos.GetXY() - triangle.vertex0->pos.GetXY());
			if (signedAreaW2 >= 0) continue;


			// Get total area before
			const float totalArea = signedAreaW0 + signedAreaW1 + signedAreaW2;
			Vector3 weights
			{
				signedAreaW0 / totalArea,
				signedAreaW1 / totalArea,
				signedAreaW2 / totalArea,
			};


			const float nonLinearDepth = 1.0f / (
				weights.x / triangle.vertex0->pos.z +
				weights.y / triangle.vertex1->pos.z +
				weights.z / triangle.vertex2->pos.z);

			// Culling I DON"T KNOW IT DOES NOT SEEM CORRECT THE DEPTH THAT IS
			//if (nonLinearPixelDepth < 0.0f or nonLinearPixelDepth > 1.0f)
			//	continue;

			const int pixelIndex = pixelX + pixelY * m_ScreenWidth;

			// Depth check
			if (nonLinearDepth > m_pDepthBufferPixels[pixelIndex])
				continue;
			m_pDepthBufferPixels[pixelIndex] = nonLinearDepth;

			ShadePixel(triangle, materialPtrs, weights, pixelIndex, nonLinearDepth);
		}
	}
}

void Renderer::ShadePixel(const Triangle& triangle, const std::vector<Material*>& materialPtrs, const Vector3& weights, int pixelIndex, float nonLinearDepth) const
{
	ColorRGB finalPixelColor{};
	const Material* material{ materialPtrs[triangle.vertex0->materialIndex] };

	const float linearPixelDepth = 1.0f / (
		weights.x / triangle.vertex0->pos.w +
		weights.y / triangle.vertex1->pos.w +
		weights.z / triangle.vertex2->pos.w);


	const Vector2 uv = linearPixelDepth * (
		triangle.vertex0->uv / triangle.vertex0->pos.w * weights.x +
		triangle.vertex1->uv / triangle.vertex1->pos.w * weights.y +
		triangle.vertex2->uv / triangle.vertex2->pos.w * weights.z);


	Vector3 normal = (
		triangle.vertex0->normal * weights.x +
		triangle.vertex1->normal * weights.y +
		triangle.vertex2->normal * weights.z);


	const Vector3 tangent = (
		triangle.vertex0->tangent * weights.x +
		triangle.vertex1->tangent * weights.y +
		triangle.vertex2->tangent * weights.z);


	Vector3 binormal = Vector3::Cross(normal, tangent);

	Matrix tangentSpaceAxis =
	{
		tangent,
		binormal,
		normal,
		Vector3::Zero
	};

	if (material->normal && m_UseNormalMap)
	{
		ColorRGB normalColor = material->normal->Sample(uv);
		Vector3 sampledNormal
		{
			2.0f * normalColor.r - 1.0f,
			2.0f * normalColor.g - 1.0f,
			2.0f * normalColor.b - 1.0f
		};

		normal = sampledNormal;
		normal = tangentSpaceAxis.TransformPoint(normal);
	}

	float diffuseStrength{ 5.0f };//m_DiffuseReflectance
	float specular{ 0.5f }; //m_SpecularReflectance
	float phongExponent{ 10.0f }; // Shininess

	if (material->specular)
		specular *= material->specular->Sample(uv).r;

	if (material->gloss)
		phongExponent *= material->gloss->Sample(uv).r;


	ColorRGB diffuseColor = material->diffuse->Sample(uv);

	//Diffuse Color * Diffuse Reflection Coefficient / pi
	ColorRGB lambertDeffuse = diffuseColor * diffuseStrength / PI;

	// Cosine Law
	const float observedArea = std::max(0.0f, Vector3::Dot(normal, -LIGHT_DIRECTION));


	const Vector3 viewDirection = (
		triangle.vertex0->viewDirection * weights.x +
		triangle.vertex1->viewDirection * weights.y +
		triangle.vertex2->viewDirection * weights.z);


	const Vector3 reflectedRay = Vector3::Reflect(LIGHT_DIRECTION, normal);
	const float cosAlpha{ std::max(Vector3::Dot(reflectedRay,-viewDirection),0.0f) };
	const float specularIntensity{ specular * std::powf(cosAlpha,phongExponent) };


	switch (m_RenderMode)
	{
		case DebugRenderMode::Diffuse:
		{
			finalPixelColor = lambertDeffuse;
		} break;
		case DebugRenderMode::ObservedArea:
		{
			finalPixelColor = colors::White * observedArea;
		}break;
		case DebugRenderMode::DiffuseOA:
		{
			finalPixelColor = lambertDeffuse * observedArea;
		}break;
		case DebugRenderMode::SpecularOA:
		{
			finalPixelColor = specularIntensity * colors::White * observedArea;
		}break;
		case DebugRenderMode::Combined:
		{
			finalPixelColor = (specularIntensity * colors::White + lambertDeffuse) * observedArea + AMBIENT_COLOR;
		} break;
		case DebugRenderMode::UVColor:
		{
			finalPixelColor = m_MaterialPtrMap.at("uvGrid2")->diffuse->Sample(uv);
		} break;
		case DebugRenderMode::Weights:
		{
			finalPixelColor =
				triangle.vertex0->color * weights.x +
				triangle.vertex1->color * weights.y +
				triangle.vertex2->color * weights.z;
		} break;
		case DebugRenderMode::DepthBuffer:
		{
			//finalPixelColor = colors::White * ;
			//finalPixelColor = colors::White * std::lerp(0.985f, 1.0f, nonLinearPixelDepth);
			//finalPixelColor = colors::White * std::clamp(triangle.vertex0.positionScreen.z,0.0f,1.0f);
			//finalPixelColor = colors::White * std::clamp(nonLinearPixelDepth,0.0f,1.0f);
			//finalPixelColor = colors::White * Utils::MapValueInRangeClamped(nonLinearPixelDepth,0.9f,1.0f,0.0f,1.0f);
			//finalPixelColor = colors::White * std::ranges::clamp(nonLinearPixelDepth,0.0f,1.0f);

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
			switch (triangle.vertex0->materialIndex)
			{
			case 0:
				finalPixelColor = colors::Red;
				break;
			case 1:
				finalPixelColor = colors::Blue;
				break;
			case 2:
				finalPixelColor = colors::Green;
				break;
			case 3:
				finalPixelColor = colors::Cyan;
				break;
			case 4:
				finalPixelColor = colors::Magenta;
				break;
			case 5:
				finalPixelColor = colors::Yellow;
				break;
			default:
				finalPixelColor = colors::White;
			}

		} break;
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
	return SDL_SaveBMP(m_BackBufferPtr, "Rasterizer_ColorBuffer.bmp");
}


