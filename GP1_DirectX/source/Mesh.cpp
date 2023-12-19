#include "pch.h"
#include "Mesh.h"

#include <cassert>

#include "Effect.h"
#include "Utils.h"


using namespace dae;

// Direct load
Mesh::Mesh(ID3D11Device* devicePtr, const std::vector<VertexModel>& vertices, const std::vector<uint32_t>& indices, const std::vector<Material*>& materials) :
	m_MaterialPtrs(materials),
	m_ModelVertices(vertices),
	m_Indices(indices),
	m_IndicesCount(static_cast<UINT>(m_Indices.size()))
{
	InitializeEffect(devicePtr);
	InitializeMesh(devicePtr);
}

// Mesh parse
Mesh::Mesh(ID3D11Device* devicePtr, const std::string& objName, const std::vector<Material*>& materials) :
	m_MaterialPtrs(materials)
{
	// Retrieves the vertices and indices
	Utils::ParseOBJ(devicePtr, objName, m_ModelVertices, m_Indices);

	// Update indices count
	m_IndicesCount = static_cast<UINT>(m_Indices.size());

	InitializeEffect(devicePtr);
	InitializeMesh(devicePtr);
}

// Mesh and materials parse
Mesh::Mesh(ID3D11Device* devicePtr, const std::string& objName, const std::string& mtlName,std::map<std::string, Material*>& materialMap)
{
	// Parse MTL
	std::map<std::string, Material*> parsedMaterials{};
	Utils::ParseMTL(devicePtr,mtlName, parsedMaterials);

	// Inset parsedMaterials
	materialMap.insert(parsedMaterials.begin(), parsedMaterials.end());

	// Parse OBJ
	std::vector<std::string> mappedMaterials{};
	Utils::ParseOBJ(devicePtr, objName, m_ModelVertices, m_Indices, mappedMaterials);

	// Update indices count
	m_IndicesCount = static_cast<UINT>(m_Indices.size());

	// Insert material pointers based on the index given from mappedMaterials
	m_MaterialPtrs.resize(mappedMaterials.size());

	for (size_t i{}; i < mappedMaterials.size(); i++)
	{
		std::cout << std::boolalpha << "Index: " << i << " mapped to: " << mappedMaterials[i] << " Material Exists: " << (materialMap[mappedMaterials[i]] != nullptr) << std::endl;
		m_MaterialPtrs[i] = materialMap[mappedMaterials[i]];
	}


	InitializeEffect(devicePtr);
	InitializeMesh(devicePtr);
}




Mesh::~Mesh()
{
	m_InputLayoutPtr->Release();
	m_InputLayoutPtr = nullptr;

	m_VertexBufferPtr->Release();
	m_VertexBufferPtr = nullptr;

	m_IndexBufferPtr->Release();
	m_IndexBufferPtr = nullptr;

	delete m_EffectPtr;
	m_EffectPtr = nullptr;
}

void Mesh::Render(ID3D11DeviceContext* deviceContextPtr,const Matrix& viewProjectionMatrix) const
{
	m_EffectPtr->UpdateViewProjectionMatrix(viewProjectionMatrix);
	m_EffectPtr->UpdateMeshWorldMatrix(m_WorldMatrix);

	m_EffectPtr->SetDiffuseMap(m_MaterialPtrs[0]->diffuse);
	m_EffectPtr->SetNormalMap(m_MaterialPtrs[0]->normal);
	m_EffectPtr->SetSpecularMap(m_MaterialPtrs[0]->specular);
	m_EffectPtr->SetGlossMap(m_MaterialPtrs[0]->gloss);

	deviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContextPtr->IASetInputLayout(m_InputLayoutPtr);

	constexpr UINT stride = sizeof(VertexModel);
	constexpr UINT offset = 0;

	deviceContextPtr->IASetVertexBuffers(0, 1, &m_VertexBufferPtr, &stride, &offset);
	deviceContextPtr->IASetIndexBuffer(m_IndexBufferPtr, DXGI_FORMAT_R32_UINT,0);

	D3DX11_TECHNIQUE_DESC techDesc{};
	m_EffectPtr->GetTechniquePtr()->GetDesc(&techDesc);
	for (UINT passIndex = 0; passIndex < techDesc.Passes; passIndex++)
	{
		m_EffectPtr->GetTechniquePtr()->GetPassByIndex(passIndex)->Apply(0, deviceContextPtr);
		deviceContextPtr->DrawIndexed(m_IndicesCount, 0, 0);
	}
}

void Mesh::InitializeEffect(ID3D11Device* devicePtr)
{
	m_EffectPtr = new Effect(devicePtr, EFFECT_FILE_PATH);
}

void Mesh::InitializeMesh(ID3D11Device* devicePtr)
{
	///////////////////////
	// Create scoped variable used for setup
	///////////////////////
	HRESULT result{};
	D3D11_BUFFER_DESC bd{};
	D3D11_SUBRESOURCE_DATA initData{};


	///////////////////////
	// Create vertex layout
	///////////////////////
	static constexpr uint32_t vertexElementCount{ 3 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[vertexElementCount]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = sizeof(float) * 3;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = sizeof(float) * 6;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = sizeof(float) * 6;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	//vertexDesc[3].SemanticName = "TEXCOORD";
	//vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	//vertexDesc[3].AlignedByteOffset = sizeof(float) * 6;
	//vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	//vertexDesc[4].SemanticName = "TEXCOORD";
	//vertexDesc[4].Format = DXGI_FORMAT_R32G32_FLOAT;
	//vertexDesc[4].AlignedByteOffset = sizeof(float) * 6;
	//vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	//vertexDesc[5].SemanticName = "TEXCOORD";
	//vertexDesc[5].Format = DXGI_FORMAT_R32G32_FLOAT;
	//vertexDesc[5].AlignedByteOffset = sizeof(float) * 6;
	//vertexDesc[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	//vertexDesc[5].SemanticName = "M";
	//vertexDesc[5].Format = DXGI_FORMAT_R32G32_FLOAT;
	//vertexDesc[5].AlignedByteOffset = sizeof(float) * 6;
	//vertexDesc[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	///////////////////////
	// Create input layout
	///////////////////////
	D3DX11_PASS_DESC passDesc{};
	ID3DX11EffectTechnique* techniquePtr = m_EffectPtr->GetTechniquePtr();
	techniquePtr->GetPassByIndex(0)->GetDesc(&passDesc);

	result = devicePtr->CreateInputLayout(
		vertexDesc,
		vertexElementCount,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_InputLayoutPtr
	);


	assert(result == S_OK);


	///////////////////////
	// Create vertex buffer
	///////////////////////
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(VertexModel) * static_cast<uint32_t>(m_ModelVertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = m_ModelVertices.data();
	result = devicePtr->CreateBuffer(&bd, &initData, &m_VertexBufferPtr);

	assert(result == S_OK && "Creating vertex buffer failed");


	///////////////////////
	// Create index buffer
	///////////////////////
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_IndicesCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = m_Indices.data();
	result = devicePtr->CreateBuffer(&bd, &initData, &m_IndexBufferPtr);

	assert(result == S_OK && "Creating index buffer failed");
}

void Mesh::UpdateWorldMatrix()
{
	const Matrix translationMatrix
	{
		{1.0f,0.0f,0.0f},
		{0.0f,1.0f,0.0f},
		{0.0f,0.0f,1.0f},
		{m_Position}
	};

	const Matrix rotateMatrix
	{
		{ std::cos(m_YawRotation), 0, -std::sin(m_YawRotation), 0},
		{0.f, 1.f, 0.f, 0.f},
		{std::sin(m_YawRotation),0.f, std::cos(m_YawRotation), 0.f},
		{0.f, 0.f, 0.f, 1.f}
	};

	const Matrix scaleMatrix
	{
		{m_Scale.x,0.f,0.f,0.f},
		{0.f,m_Scale.y,0.f,0.f},
		{0.f,0.f,m_Scale.z,0.f},
		{0.f,0.f,0.f,1.f}
	};

	m_WorldMatrix = scaleMatrix * rotateMatrix * translationMatrix;
}



void Mesh::SetPosition(Vector3 translate)
{
	m_Position = translate;
	UpdateWorldMatrix();
}

void Mesh::SetScale(Vector3 scale)
{
	m_Scale = scale;
	UpdateWorldMatrix();
}

void Mesh::SetYawRotation(float yawRotation)
{
	m_YawRotation = yawRotation;
	UpdateWorldMatrix();
}

void Mesh::AddYawRotation(float yawDelta)
{
	m_YawRotation += yawDelta;
	UpdateWorldMatrix();
}