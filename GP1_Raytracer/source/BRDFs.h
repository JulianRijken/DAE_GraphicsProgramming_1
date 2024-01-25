#pragma once
#include <cassert>
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			//todo: W3
			return { (cd * kd) / PI };
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//todo: W3
			return { (cd * kd) / PI };
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			// Wannes
			//ColorRGB reflectColor{ 1.f,1.f,1.f };
			//Vector3 reflect{ l - (2 * Vector3::Dot(n, l) * n) };
			//float cosAngle{ Vector3::Dot(reflect, v) };
			//return { ks * powf(std::max(0.f, cosAngle),exp) * reflectColor };

			// MAT
			//Vector3 reflect{ l - 2 * Vector3::Dot(n, l) * n };
			//float cosAlpha{ std::max(Vector3::Dot(reflect, v), 0.f) };
			//return ColorRGB(1, 1, 1) * ks * std::pow(cosAlpha, exp);

			const Vector3 reflectedRay = Vector3::Reflect(l, n);
			const float cosAlpha{ std::max(Vector3::Dot(reflectedRay,v),0.0f) };
			const float specularIntensity{ ks * std::powf(cosAlpha,exp) };
			return { specularIntensity * colors::White };
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			const float a{ 1.0f - std::max(0.0f, Vector3::Dot(h,v)) };
			return f0 + (1.0f - f0) * (a * a * a * a * a);
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			const float a{ Square(roughness)};
			const float dotNH = Vector3::Dot(n, h);
			const float denominator = Square(dotNH) * (Square(a) - 1.0f) + 1.0f;
			const float result = Square(a) / (PI * Square(denominator));

			return result;
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			const float a{Square(roughness) };
			const float kDirect{ Square(a + 1) / 8.0f };
			const float dotNV{ std::max(Vector3::Dot(n,v),0.0f) };

			return dotNV / (dotNV * (1.0f - kDirect) + kDirect);
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			return GeometryFunction_SchlickGGX(n, v, roughness) * GeometryFunction_SchlickGGX(n, l, roughness);
		}

	}
}