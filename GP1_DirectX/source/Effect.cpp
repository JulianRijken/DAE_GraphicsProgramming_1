#include "pch.h"
#include "Effect.h"

#include <cassert>
#include <fstream>

#include "Texture.h"

using namespace dae;

Effect::Effect(ID3D11Device* devicePtr, const std::wstring& effectFileName) :
	m_DevicePtr(devicePtr)
{
	const std::ifstream file(effectFileName);
	if (!file)
		std::wcout << "File does not exist" << std::endl;

	m_EffectPtr = LoadEffect(devicePtr, effectFileName);
	m_TechniquePtr = m_EffectPtr->GetTechniqueByName(TECHNIQUE_NAME);

	// Get view matrix variable
	m_ViewProjectionMatrixVarPtr = m_EffectPtr->GetVariableByName("g_WorldViewProjection")->AsMatrix();
	if (not m_ViewProjectionMatrixVarPtr->IsValid())
		std::wcout << L"g_WorldViewProjection is not valid" << std::endl;

	// Get model matrix variable
	m_MeshWorldMatrixVarPtr = m_EffectPtr->GetVariableByName("g_MeshWorldMatrix")->AsMatrix();
	if (not m_MeshWorldMatrixVarPtr->IsValid())
		std::wcout << L"g_MeshWorldMatrix is not valid" << std::endl;

	// Setup sample state
	m_SampleStateVariable = m_EffectPtr->GetVariableByName("g_TextureSampler")->AsSampler();
	if (not m_SampleStateVariable->IsValid())
		std::wcout << L"g_TextureSampler is not valid" << std::endl;

	BindTexture(m_DiffuseMapVarPtr, "g_DiffuseMap");
	BindTexture(m_NormalMapVarPtr, "g_NormalMap");
	BindTexture(m_SpecularMapVarPtr, "g_SpecularMap");
	BindTexture(m_GlossMapVarPtr, "g_GlossMap");


	if(not m_TechniquePtr->IsValid())
		std::wcout << L"Technique is not valid" << std::endl;
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

void Effect::SetDiffuseMap(const Texture* texturePtr) const
{
	if (m_EffectPtr == nullptr || texturePtr == nullptr) return;
	m_DiffuseMapVarPtr->SetResource(texturePtr->GetShaderResource());
}

void Effect::SetNormalMap(const Texture* texturePtr) const
{
	if (m_EffectPtr == nullptr || texturePtr == nullptr) return;
	m_NormalMapVarPtr->SetResource(texturePtr->GetShaderResource());
}

void Effect::SetSpecularMap(const Texture* texturePtr) const
{
	if (m_EffectPtr == nullptr || texturePtr == nullptr) return;
	m_SpecularMapVarPtr->SetResource(texturePtr->GetShaderResource());
}

void Effect::SetGlossMap(const Texture* texturePtr) const
{
	if (m_EffectPtr == nullptr || texturePtr == nullptr) return;
	m_GlossMapVarPtr->SetResource(texturePtr->GetShaderResource());
}

void Effect::SetSampleState(int state) const
{
	D3D11_SAMPLER_DESC samplerDesc{};


	switch (state)
	{
	case 0:
		samplerDesc =
		{
			.Filter = D3D11_FILTER_ANISOTROPIC,
			.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
			.ComparisonFunc = D3D11_COMPARISON_NEVER
		};

		break;

	case 1:
		samplerDesc =
		{
			.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
			.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
			.ComparisonFunc = D3D11_COMPARISON_NEVER
		};

		break;

	case 2:
		samplerDesc =
		{
			.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
			.ComparisonFunc = D3D11_COMPARISON_NEVER
		};

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
