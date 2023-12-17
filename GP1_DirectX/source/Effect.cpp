#include "pch.h"
#include "Effect.h"

#include <fstream>

#include "Texture.h"

Effect::Effect(ID3D11Device* devicePtr, const std::wstring& effectFileName)
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


	// Get diffuse color variable
	m_DiffuseMapVarPtr = m_EffectPtr->GetVariableByName("g_DiffuseMap")->AsShaderResource();
	if (not m_DiffuseMapVarPtr->IsValid())
		std::wcout << L"g_DiffuseMap is not valid" << std::endl;


	if(not m_TechniquePtr->IsValid())
		std::wcout << L"Technique is not valid" << std::endl;
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
	if (m_EffectPtr == nullptr || m_MeshWorldMatrixVarPtr == nullptr)
		return;

	m_MeshWorldMatrixVarPtr->SetMatrix(reinterpret_cast<const float*>(&meshWorldMatrix));
}

void Effect::SetDiffuseMap(const Texture* texturePtr) const
{
	if (m_EffectPtr == nullptr || texturePtr == nullptr)
		return;
	
	m_DiffuseMapVarPtr->SetResource(texturePtr->GetShaderResource());
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
