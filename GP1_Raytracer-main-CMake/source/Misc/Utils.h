#pragma once
#include <cassert>
#include <fstream>

#include "Math/Math.h"
#include "Misc/DataTypes.h"
#include "Math/ColorRGB.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const Vector3 rayOriginToSphere = ray.origin - sphere.origin;

			// Calculate coefficients of the quadratic equation for ray-sphere intersection.
			const float b = Vector3::Dot(rayOriginToSphere, ray.direction);
			const float c = Vector3::Dot(rayOriginToSphere, rayOriginToSphere) - (sphere.radius * sphere.radius);
			const float discriminant = b * b - c;

			// Check if ray intersects
			if (discriminant < 0)
				return false;

			const float distance = (-b - sqrt(discriminant));

			if (distance < ray.min || distance > ray.max)
				return false;

			if (not ignoreHitRecord)
			{
				hitRecord.t = distance;
				hitRecord.point = ray.origin + ray.direction * hitRecord.t;
				hitRecord.normal = (hitRecord.point - sphere.origin) / sphere.radius;
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;
			}

			return true;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const float normalDot = Vector3::Dot(ray.direction, plane.normal);

			// Don't render back of plane
			if (normalDot > 0)
				return false;

			const float distance = Vector3::Dot(plane.origin - ray.origin, plane.normal) / normalDot;

			// Check if the intersection point is behind the ray's origin
			if (distance < ray.min || distance > ray.max)
				return false;

			if (not ignoreHitRecord)
			{
				hitRecord.t = distance;
				hitRecord.point = ray.origin + ray.direction * distance;
				hitRecord.normal = plane.normal;
				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
			}

			return true;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			//assert(false && "No Implemented Yet!");

			return { light.origin - origin};
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			if(light.type == LightType::Point)
			{
				const float radiusSquared{ Square((light.origin - target).Magnitude()) };
				const float irradiance{ light.intensity / radiusSquared };

				return{ light.color * irradiance };
			}

			if (light.type == LightType::Directional)
			{
				return{ light.color * light.intensity };
			}

			return {};
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}
