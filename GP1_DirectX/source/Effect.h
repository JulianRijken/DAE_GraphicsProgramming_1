#pragma once
namespace dae
{
	class Texture;
}

using namespace dae;

class Effect final
{
	static inline constexpr char TECHNIQUE_NAME[] = "DefaultTechnique";

public:

	Effect(ID3D11Device* devicePtr, const std::wstring& effectFileName);
	~Effect();

	ID3DX11Effect* GetEffectPtr() const { return m_EffectPtr; }
	ID3DX11EffectTechnique* GetTechniquePtr() const { return m_TechniquePtr; }

	void UpdateViewProjectionMatrix(const Matrix& viewProjectionMatrix) const;
	void UpdateMeshWorldMatrix(const Matrix& meshWorldMatrix) const;
	void SetDiffuseMap(const Texture* texturePtr) const;

private:

	ID3DX11Effect* m_EffectPtr{};
	ID3DX11EffectTechnique* m_TechniquePtr{};

	// External loaded in to the shader
	ID3DX11EffectMatrixVariable* m_ViewProjectionMatrixVarPtr{};
	ID3DX11EffectMatrixVariable* m_MeshWorldMatrixVarPtr{};
	ID3DX11EffectShaderResourceVariable* m_DiffuseMapVarPtr{};


	static ID3DX11Effect* LoadEffect(ID3D11Device* devicePtr, const std::wstring& effectFileName);
};

