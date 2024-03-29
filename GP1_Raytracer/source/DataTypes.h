#pragma once
#include <cassert>

#include "Math.h"
#include "vector"

namespace dae
{
#pragma region GEOMETRY
	struct Sphere
	{
		Vector3 origin{};
		float radius{};

		unsigned char materialIndex{ 0 };
	};

	struct Plane
	{
		Vector3 origin{};
		Vector3 normal{};

		unsigned char materialIndex{ 0 };
	};

	enum class TriangleCullMode
	{
		FrontFaceCulling,
		BackFaceCulling,
		NoCulling
	};

	struct Triangle
	{
		Triangle() = default;
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal):
			v0{_v0}, v1{_v1}, v2{_v2}, normal{_normal.Normalized()}{}

		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			const Vector3 edgeV0V1 = v1 - v0;
			const Vector3 edgeV0V2 = v2 - v0;
			normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
		}

		Vector3 v0{};
		Vector3 v1{};
		Vector3 v2{};

		Vector3 normal{};

		TriangleCullMode cullMode{};
		unsigned char materialIndex{ 0 };
	};

	struct TriangleMesh
	{
		TriangleMesh() = default;
		TriangleMesh(std::vector<Vector3> _positions, std::vector<int> _indices, TriangleCullMode _cullMode):
		positions(std::move(_positions)), indices(std::move(_indices)), cullMode(_cullMode)
		{
			////Calculate Normals
			//CalculateNormals();

			//Update Transforms
			UpdateTransforms();
		}

		TriangleMesh(std::vector<Vector3> _positions, std::vector<int> _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode);

		std::vector<Vector3> positions{};
		//std::vector<Vector3> normals{};
		std::vector<int> indices{};
		unsigned char materialIndex{ 0 };

		TriangleCullMode cullMode{TriangleCullMode::BackFaceCulling};

		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		Vector3 minAABB;
		Vector3 maxAABB;

		Vector3 transformedMinAABB;
		Vector3 transformedMaxAABB;

		std::vector<Vector3> transformedPositions{};
		//std::vector<Vector3> transformedNormals{};

		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void RotateY(float yaw)
		{
			rotationTransform = Matrix::CreateRotationY(yaw);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			int startIndex = static_cast<int>(positions.size());

			positions.push_back(triangle.v0);
			positions.push_back(triangle.v1);
			positions.push_back(triangle.v2);

			indices.push_back(startIndex);
			indices.push_back(++startIndex);
			indices.push_back(++startIndex);

			//normals.push_back(triangle.normal);

			//Not ideal, but making sure all vertices are updated
			if(!ignoreTransformUpdate)
				UpdateTransforms();
		}

		//void CalculateNormals()
		//{
		//	normals.clear(); // Clear any existing normals

		//	// Loop through the triangles and calculate normals
		//	for (size_t i = 0; i < indices.size(); i += 3)
		//	{
		//		const Vector3& v0 = positions[indices[i]];
		//		const Vector3& v1 = positions[indices[i + 1]];
		//		const Vector3& v2 = positions[indices[i + 2]];

		//		// Calculate the normal for this triangle
		//		Vector3 edge1 = v1 - v0;
		//		Vector3 edge2 = v2 - v0;
		//		Vector3 normal = Vector3::Cross(edge1, edge2).Normalized();

		//		// Assign the same normal to all vertices of the triangle
		//		normals.push_back(normal);
		//	}
		//}

		void UpdateTransforms()
		{
			// Calculate the final transformation matrix
			const Matrix finalTransform = scaleTransform * rotationTransform * translationTransform;

			// Clear any existing transformed positions and normals
			transformedPositions.clear();

			// Apply the final transform to each position and normal
			for (const Vector3& position : positions)
			{
				// Apply the transformation to the position
				Vector3 transformedPosition = finalTransform.TransformPoint(position);
				transformedPositions.push_back(transformedPosition);
			}

			//transformedNormals.clear();

			//for (const Vector3& normal : normals)
			//{
			//	// Apply the rotation part of the transformation to the normal
			//	Vector3 transformedNormal = rotationTransform.TransformVector(normal);
			//	transformedNormals.push_back(transformedNormal);
			//}

			for (const Vector3& position : positions)
			{
				transformedPositions.emplace_back(finalTransform.TransformPoint(position));
			}
			UpdateTransformedAAB(finalTransform);
		}

		void UpdateAABB()
		{
			if(!positions.empty())
			{
				minAABB = positions[0];
				maxAABB = positions[0];
				for (const Vector3& position : positions)
				{
					minAABB = Vector3::Min(position, minAABB);
					maxAABB = Vector3::Max(position, maxAABB);
				}
			}
		}

		void UpdateTransformedAAB(const Matrix& finalTransform)
		{
			Vector3 tMinAABB = finalTransform.TransformPoint(minAABB);
			Vector3 tMaxAABB = tMinAABB;

			Vector3 tAABB = finalTransform.TransformPoint(maxAABB.x, minAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(maxAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(minAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(maxAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(maxAABB);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			transformedMinAABB = tMinAABB;
			transformedMaxAABB = tMaxAABB;
		}
	};

	inline TriangleMesh::TriangleMesh(std::vector<Vector3> _positions, std::vector<int> _indices,
	                                  const std::vector<Vector3>& _normals, TriangleCullMode _cullMode):
		positions(std::move(_positions)), indices(std::move(_indices)), /*normals(_normals),*/ cullMode(_cullMode)
	{
		UpdateTransforms();
	}
#pragma endregion
#pragma region LIGHT
	enum class LightType
	{
		Point,
		Directional
	};

	struct Light
	{
		Vector3 origin{};
		Vector3 direction{};
		ColorRGB color{};
		float intensity{};

		LightType type{};
	};
#pragma endregion
#pragma region MISC
	struct Ray
	{
		Vector3 origin{};
		Vector3 direction{};

		float min{ 0.0001f };
		float max{ FLT_MAX };
	};

	struct HitRecord
	{
		Vector3 point{};
		Vector3 normal{};
		float t = FLT_MAX;

		bool didHit{ false };
		unsigned char materialIndex{ 0 };
	};
#pragma endregion
}