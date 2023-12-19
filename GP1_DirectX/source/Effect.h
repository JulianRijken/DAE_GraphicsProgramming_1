#pragma once
namespace dae
{
	class Texture;
}


class Effect final
{
	static inline constexpr char TECHNIQUE_NAME[] = "DefaultTechnique";

public:

	Effect(ID3D11Device* devicePtr, const std::wstring& effectFileName);
	~Effect();

	[[nodiscard]] ID3DX11Effect* GetEffectPtr() const { return m_EffectPtr; }
	[[nodiscard]] ID3DX11EffectTechnique* GetTechniquePtr() const { return m_TechniquePtr; }

	void UpdateViewProjectionMatrix(const dae::Matrix& viewProjectionMatrix) const;
	void UpdateMeshWorldMatrix(const dae::Matrix& meshWorldMatrix) const;

	void SetDiffuseMap(const dae::Texture* texturePtr) const;
	void SetNormalMap(const dae::Texture* texturePtr) const;
	void SetSpecularMap(const dae::Texture* texturePtr) const;
	void SetGlossMap(const dae::Texture* texturePtr) const;


	void SetSampleState() const;


	void BindTexture(ID3DX11EffectShaderResourceVariable*& target,const std::string& name) const;

private:

	ID3D11Device* m_DevicePtr{};


	ID3DX11Effect* m_EffectPtr{};
	ID3DX11EffectTechnique* m_TechniquePtr{};

	// External loaded in to the shader
	ID3DX11EffectMatrixVariable* m_ViewProjectionMatrixVarPtr{};
	ID3DX11EffectMatrixVariable* m_MeshWorldMatrixVarPtr{};

	ID3DX11EffectSamplerVariable* m_SampleStateVariable{};

	ID3DX11EffectShaderResourceVariable* m_DiffuseMapVarPtr{};
	ID3DX11EffectShaderResourceVariable* m_NormalMapVarPtr{};
	ID3DX11EffectShaderResourceVariable* m_SpecularMapVarPtr{};
	ID3DX11EffectShaderResourceVariable* m_GlossMapVarPtr{};


	static ID3DX11Effect* LoadEffect(ID3D11Device* devicePtr, const std::wstring& effectFileName);
};

