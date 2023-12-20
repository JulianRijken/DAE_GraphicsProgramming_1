#include "pch.h"
#include "Effect.h"

#include <cassert>
#include <fstream>

#include "Mesh.h"
#include "Renderer.h"
#include "Texture.h"

using namespace dae;

Effect::Effect(ID3D11Device* devicePtr, const std::wstring& effectFileName) :
	m_DevicePtr(devicePtr)
{
	// Used for debugging validity 
	#define ISVALID(var, name) if (!var->IsValid()) std::cout << name << " is not valid" << std::endl;


	////////////////
	// Load effect
	////////////////
	const std::ifstream file(effectFileName);
	if (!file) std::wcout << "File does not exist" << std::endl;

	m_EffectPtr = LoadEffect(devicePtr, effectFileName);
	m_TechniquePtr = m_EffectPtr->GetTechniqueByName(TECHNIQUE_NAME);
	ISVALID(m_TechniquePtr, "m_TechniquePtr")


	////////////////
	// Bind variable
	////////////////
	m_ViewProjectionMatrixVarPtr = m_EffectPtr->GetVariableByName("g_WorldViewProjection")->AsMatrix();
	ISVALID(m_ViewProjectionMatrixVarPtr,"m_ViewProjectionMatrixVarPtr")

	m_MeshWorldMatrixVarPtr = m_EffectPtr->GetVariableByName("g_MeshWorldMatrix")->AsMatrix();
	ISVALID(m_MeshWorldMatrixVarPtr, "m_MeshWorldMatrixVarPtr")

	m_SampleStateVariable = m_EffectPtr->GetVariableByName("g_TextureSampler")->AsSampler();
	ISVALID(m_SampleStateVariable, "m_SampleStateVariable")

	m_LightDirection = m_EffectPtr->GetVariableByName("g_LightDirection")->AsVector();
	ISVALID(m_SampleStateVariable, "m_SampleStateVariable")


	m_CameraOrigin = m_EffectPtr->GetVariableByName("g_CameraOrigin")->AsVector();
	ISVALID(m_CameraOrigin, "g_CameraOrigin")

	//m_PiVariable = m_EffectPtr->GetVariableByName("g_PI");
	//ISVALID(m_PiVariable, "g_PI")


	BindTexture(m_DiffuseMapVarPtr, "g_DiffuseMapArray");
	BindTexture(m_NormalMapVarPtr, "g_NormalMapArray");
	BindTexture(m_SpecularMapVarPtr, "g_SpecularMapArray");
	BindTexture(m_GlossMapVarPtr, "g_GlossMapArray");

	SetSampleState(0);
}

void Effect::BindTexture(ID3DX11EffectShaderResourceVariable*& target, const std::string& name) const
{
	target = m_EffectPtr->GetVariableByName(name.c_str())->AsShaderResource();
	if (not target->IsValid())
		std::cout << "Bind texture failed! is not valid: " << name <<  std::endl;
}

Effect::~Effect()
{
	m_EffectPtr->Release();
	m_EffectPtr = nullptr;
}

void Effect::UpdateViewProjectionMatrix(const Matrix& viewProjectionMatrix) const
{
	if (m_EffectPtr == nullptr || m_ViewProjectionMatrixVarPtr == nullptr) 
		return;

	m_ViewProjectionMatrixVarPtr->SetMatrix(reinterpret_cast<const float*>(&viewProjectionMatrix));
}

void Effect::UpdateMeshWorldMatrix(const Matrix& meshWorldMatrix) const
{
	if (m_EffectPtr == nullptr || m_MeshWorldMatrixVarPtr == nullptr) return;
	m_MeshWorldMatrixVarPtr->SetMatrix(reinterpret_cast<const float*>(&meshWorldMatrix));
}


void Effect::SetDiffuseMaps(const std::vector<Material*>& materialPtr, const DefaultTextures& defaultTextures)
{
	m_ShaderResources.clear();
	m_ShaderResources.reserve(materialPtr.size());

	for (int materialIndex = 0; materialIndex < materialPtr.size() - 1; ++materialIndex)
	{
		const Texture* texture = materialPtr[materialIndex]->diffuse;

		if(texture == nullptr)
		{
			m_ShaderResources.emplace_back(defaultTextures.defaultWhiteTexture->GetShaderResource());
		}
		else
		{
			m_ShaderResources.emplace_back(texture->GetShaderResource());
		}
	}

	m_DiffuseMapVarPtr->SetResourceArray(m_ShaderResources.data(), 0, m_ShaderResources.size());
}

void Effect::SetNormalMap(const Texture* texturePtr, int materialIndex) const
{
	if (m_EffectPtr == nullptr || texturePtr == nullptr) return;
	ID3D11ShaderResourceView* resourceView = texturePtr->GetShaderResource();
	m_NormalMapVarPtr->SetResourceArray(&resourceView, 0, materialIndex);
}

void Effect::SetSpecularMap(const Texture* texturePtr, int materialIndex) const
{
	if (m_EffectPtr == nullptr || texturePtr == nullptr) return;
	ID3D11ShaderResourceView* resourceView = texturePtr->GetShaderResource();
	m_SpecularMapVarPtr->SetResourceArray(&resourceView, 0, materialIndex);
}

void Effect::SetGlossMap(const Texture* texturePtr, int materialIndex) const
{
	if (m_EffectPtr == nullptr || texturePtr == nullptr) return;
	ID3D11ShaderResourceView* resourceView = texturePtr->GetShaderResource();
	m_GlossMapVarPtr->SetResourceArray(&resourceView, 0, materialIndex);
}

void Effect::SetLightDirection(const Vector3& lightDirection) const
{
	m_LightDirection->SetFloatVector(reinterpret_cast<const float*>(&lightDirection));
}

void Effect::SetCameraOrigin(const dae::Vector3& origin) const
{
	m_CameraOrigin->SetFloatVector(reinterpret_cast<const float*>(&origin));
}

void Effect::SetSampleState(int state) const
{
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	switch (state)
	{
	case 0:
		std::cout << "filtering: ANISOTROPIC" << std::endl;
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		break;
	case 1:
		std::cout << "filtering: POINT" << std::endl;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	case 2:
		std::cout << "filtering: LINEAR" << std::endl;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	}


	ID3D11SamplerState* newSamplerState = nullptr;
	const HRESULT result = m_DevicePtr->CreateSamplerState(&samplerDesc, &newSamplerState);

	if (FAILED(result))
	{
		std::cout << "Failed to create sampler state: " << result << std::endl;
		return;
	}

	m_SampleStateVariable->SetSampler(0, newSamplerState);
}


ID3DX11Effect* Effect::LoadEffect(ID3D11Device* devicePtr, const std::wstring& effectFileName)
{
	HRESULT result;
	ID3D10Blob* errorBlobPtr{ nullptr };
	ID3DX11Effect* effectPtr{ nullptr };

	DWORD shaderFlags{ 0 };


#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile
	(
		effectFileName.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		devicePtr,
		&effectPtr,
		&errorBlobPtr
	);


	if (FAILED(result))
	{
		if (errorBlobPtr != nullptr)
		{
			const char* errorsPtr = static_cast<char*>(errorBlobPtr->GetBufferPointer());

			std::wstringstream stringStream;
			for (unsigned int i = 0; i < errorBlobPtr->GetBufferSize(); i++)
				stringStream << errorsPtr[i];

			errorBlobPtr->Release();
			errorBlobPtr = nullptr;

			std::wcout << stringStream.str() << std::endl;
		}
		else
		{
			std::wstringstream stringStream;
			stringStream << "EffectLoader failed to create affect from file!\nPath: " << effectFileName;
			std::wcout << stringStream.str() << std::endl;
			return nullptr;
		}
	}

	return effectPtr;
}
