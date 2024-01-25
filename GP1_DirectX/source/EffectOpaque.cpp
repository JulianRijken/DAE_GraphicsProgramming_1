#include "pch.h"
#include "EffectOpaque.h"

#include "Light.h"

EffectOpaque::EffectOpaque(ID3D11Device* devicePtr, const std::wstring& effectFileName) :
EffectBase(devicePtr, effectFileName)
{
	BIND(m_UseNormalMapVarPtr,"g_UseNormalMap",AsScalar)
	
	BIND(m_DiffuseStrengthKdPtr,"g_DiffuseStrengthKd",AsScalar)
	BIND(m_SampledPhongExponentPtr,"g_SampledPhongExponent",AsScalar)
	BIND(m_SpecularKsPtr,"g_SpecularKs",AsScalar)
	BIND(m_AmbientColor,"g_AmbientColor",AsVector)

	m_LightBufferPtr = m_EffectPtr->GetConstantBufferByName("LightBuffer");
}

void EffectOpaque::SetLights(const std::vector<dae::Light>& lights) const
{
	if(not m_LightBufferPtr->IsValid())
		return;
	
	m_LightBufferPtr->SetRawValue(lights.data(), 0, static_cast<int>(sizeof(dae::Light)) * static_cast<int>(lights.size()));
}
