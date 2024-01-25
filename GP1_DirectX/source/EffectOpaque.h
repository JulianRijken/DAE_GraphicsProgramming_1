#pragma once
#include "EffectBase.h"

namespace dae
{
	class Light;
}

class EffectOpaque final : public EffectBase
{
public:
	EffectOpaque(ID3D11Device* devicePtr, const std::wstring& effectFileName);

	void SetUseNormalMap(bool useNormalMap) const { m_UseNormalMapVarPtr->SetRawValue(&useNormalMap, 0, sizeof(bool)); }
	void SetDiffuseStrengthKd(float diffuseStrengthKd) const { m_DiffuseStrengthKdPtr->SetFloat(diffuseStrengthKd); }
	void SetSampledPhongExponent(float sampledPhongExponent) const { m_SampledPhongExponentPtr->SetFloat(sampledPhongExponent); }
	void SetSpecularKs(float specularKs) const { m_SpecularKsPtr->SetFloat(specularKs); }
	void SetAmbientColor(const dae::Vector3& ambientColor) const { m_AmbientColor->SetFloatVector(reinterpret_cast<const float*>(&ambientColor)); }

	void SetLights(const std::vector<dae::Light>& lights) const;
private:
	ID3DX11EffectVariable* m_UseNormalMapVarPtr{};
	
	ID3DX11EffectScalarVariable* m_DiffuseStrengthKdPtr{};
	ID3DX11EffectScalarVariable* m_SampledPhongExponentPtr{};
	ID3DX11EffectScalarVariable* m_SpecularKsPtr{};

	ID3DX11EffectVectorVariable* m_AmbientColor{};

	ID3DX11EffectConstantBuffer* m_LightBufferPtr{};

};
