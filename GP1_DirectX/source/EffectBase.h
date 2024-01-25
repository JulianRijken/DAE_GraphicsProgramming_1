#pragma once
#include "Texture.h"

namespace dae
{
	class Texture;
}

#define IS_VALID_EFFECT(var) if (!var->IsValid()) std::cout << #var << " is not a valid bind (not used in shader just a warning :) )" << std::endl;
#define BIND(var,fxName, type) var = m_EffectPtr->GetVariableByName(fxName)->type(); IS_VALID_EFFECT(var)

class EffectBase
{

public:
	
	EffectBase(ID3D11Device* devicePtr, const std::wstring& effectFileName);
	virtual ~EffectBase();

	[[nodiscard]] ID3DX11EffectTechnique* GetTechniquePtr() const { return m_TechniquePtr; }
	
	void SetSampleState(int state) const;
	
	void SetViewProjectionMatrix(const dae::Matrix& viewProjectionMatrix) const;
	void SetMeshWorldMatrix(const dae::Matrix& meshWorldMatrix) const;
	void SetCameraOrigin(const dae::Vector3& origin) const;
	void SetLightDirection(const dae::Vector3& lightDirection) const;
	
	void SetDiffuseMap (const dae::Texture* texturePtr) const;
	void SetNormalMap  (const dae::Texture* texturePtr) const;
	void SetSpecularMap(const dae::Texture* texturePtr) const;
	void SetGlossMap   (const dae::Texture* texturePtr) const;

protected:
	
	ID3DX11Effect* m_EffectPtr{};

private:

	static ID3DX11Effect* LoadEffect(ID3D11Device* devicePtr, const std::wstring& effectFileName);

	
	static inline constexpr char TECHNIQUE_NAME[] = "DefaultTechnique";
	
	ID3D11Device* m_DevicePtr{};
	ID3DX11EffectTechnique* m_TechniquePtr{};

	ID3DX11EffectSamplerVariable* m_SampleStateVarPtr{};
	ID3D11SamplerState* m_ActiveSampleStatePtr{};

	ID3DX11EffectMatrixVariable* m_WorldViewProjectionMatrixVarPtr{};
	ID3DX11EffectMatrixVariable* m_MeshWorldMatrixVarPtr{};

	ID3DX11EffectShaderResourceVariable* m_DiffuseMapVarPtr{};
	ID3DX11EffectShaderResourceVariable* m_NormalMapVarPtr{};
	ID3DX11EffectShaderResourceVariable* m_SpecularMapVarPtr{};
	ID3DX11EffectShaderResourceVariable* m_GlossMapVarPtr{};

	ID3DX11EffectVectorVariable* m_LightDirectionVarPtr{};
	ID3DX11EffectVectorVariable* m_CameraOriginVarPtr{};
	
};

