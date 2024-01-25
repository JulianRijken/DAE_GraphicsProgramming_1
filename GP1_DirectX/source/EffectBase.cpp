#include "pch.h"
#include "EffectBase.h"

#include <cassert>
#include <filesystem>
#include <fstream>

using namespace dae;

EffectBase::EffectBase(ID3D11Device* devicePtr, const std::wstring& effectFileName) :
	m_DevicePtr(devicePtr)
{
	////////////////
	// Load effect
	////////////////
	if (not std::filesystem::exists(effectFileName))
		std::wcout << "File does not exist" << std::endl;

	m_EffectPtr = LoadEffect(devicePtr, effectFileName);
	m_TechniquePtr = m_EffectPtr->GetTechniqueByName(TECHNIQUE_NAME);
	IS_VALID_EFFECT(m_TechniquePtr)


	////////////////
	// Bind variable
	////////////////
	BIND(m_SampleStateVarPtr, "g_TextureSampler", AsSampler)

	BIND(m_WorldViewProjectionMatrixVarPtr, "g_WorldViewProjection", AsMatrix)
	BIND(m_MeshWorldMatrixVarPtr, "g_MeshWorldMatrix", AsMatrix)
	BIND(m_CameraOriginVarPtr, "g_CameraOrigin", AsVector)
	BIND(m_LightDirectionVarPtr, "g_LightDirection", AsVector)

	BIND(m_DiffuseMapVarPtr, "g_DiffuseMap", AsShaderResource)
	BIND(m_NormalMapVarPtr, "g_NormalMap", AsShaderResource)
	BIND(m_SpecularMapVarPtr, "g_SpecularMap", AsShaderResource)
	BIND(m_GlossMapVarPtr, "g_GlossMap", AsShaderResource)
}

EffectBase::~EffectBase()
{
	m_EffectPtr->Release();
	m_EffectPtr = nullptr;
}

ID3DX11Effect* EffectBase::LoadEffect(ID3D11Device* devicePtr, const std::wstring& effectFileName)
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


void EffectBase::SetSampleState(int state) const
{
	if (not m_SampleStateVarPtr->IsValid())
		return;
	
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
    
	default:
		std::cout << "Wrong state change" << std::endl;
		break;
	}
	
	ID3D11SamplerState* newSampleState = nullptr;
	const HRESULT result = m_DevicePtr->CreateSamplerState(&samplerDesc, &newSampleState);
	
	if(FAILED(result))
		std::cout << "Failed to create sampler state" << std::endl;
	
	m_SampleStateVarPtr->SetSampler(0, newSampleState);

	if(newSampleState)
		newSampleState->Release();
}

void EffectBase::SetViewProjectionMatrix(const Matrix& viewProjectionMatrix) const
{
	if (not m_WorldViewProjectionMatrixVarPtr->IsValid())
		return;

	m_WorldViewProjectionMatrixVarPtr->SetMatrix(reinterpret_cast<const float*>(&viewProjectionMatrix));
}
void EffectBase::SetMeshWorldMatrix(const Matrix& meshWorldMatrix) const
{
	if (not m_MeshWorldMatrixVarPtr->IsValid())
		return;

	m_MeshWorldMatrixVarPtr->SetMatrix(reinterpret_cast<const float*>(&meshWorldMatrix));
}
void EffectBase::SetCameraOrigin(const Vector3& origin) const
{
	if (not m_CameraOriginVarPtr->IsValid())
		return;

	m_CameraOriginVarPtr->SetFloatVector(reinterpret_cast<const float*>(&origin));
}
void EffectBase::SetLightDirection(const Vector3& lightDirection) const
{
	if (not m_LightDirectionVarPtr->IsValid())
		return;

	m_LightDirectionVarPtr->SetFloatVector(reinterpret_cast<const float*>(&lightDirection));
}

void EffectBase::SetDiffuseMap(const Texture* texturePtr) const
{
	if (not m_DiffuseMapVarPtr->IsValid() || texturePtr == nullptr)
		return;

	m_DiffuseMapVarPtr->SetResource(texturePtr->GetShaderResource());
}
void EffectBase::SetNormalMap(const Texture* texturePtr) const
{
	if (not m_NormalMapVarPtr->IsValid() || texturePtr == nullptr)
		return;

	m_NormalMapVarPtr->SetResource(texturePtr->GetShaderResource());
}
void EffectBase::SetSpecularMap(const Texture* texturePtr) const
{
	if (not m_SpecularMapVarPtr->IsValid() || texturePtr == nullptr)
		return;

	m_SpecularMapVarPtr->SetResource(texturePtr->GetShaderResource());
}
void EffectBase::SetGlossMap(const Texture* texturePtr) const
{
	if (not m_GlossMapVarPtr->IsValid() || texturePtr == nullptr)
		return;

	m_GlossMapVarPtr->SetResource(texturePtr->GetShaderResource());
}
