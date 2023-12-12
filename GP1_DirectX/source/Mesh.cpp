#include "pch.h"
#include "Mesh.h"

#include <cassert>

#include "Effect.h"


Mesh::Mesh(ID3D11Device* devicePtr, const std::vector<ModelVertex>& modelVertices, const std::vector<Uint32>& indices) :
	m_EffectPtr(new Effect{ devicePtr, L"Resources/PosCol3D.fx" }),
	m_InputLayoutPtr(nullptr),
	m_VertexBuffer(nullptr),
	m_IndexBuffer(nullptr),
	m_ModelVertices(modelVertices),
	m_Indices(indices),
	m_IndicesCount(static_cast<UINT>(m_Indices.size()))
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
	static constexpr uint32_t vertexElementCount{ 2 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[vertexElementCount]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = sizeof(float) * 3;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;



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
	bd.ByteWidth = sizeof(ModelVertex) * static_cast<uint32_t>(modelVertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = modelVertices.data();
	result = devicePtr->CreateBuffer(&bd, &initData, &m_VertexBuffer);

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
	result = devicePtr->CreateBuffer(&bd, &initData, &m_IndexBuffer);

	assert(result == S_OK && "Creating index buffer failed");
}

Mesh::~Mesh()
{
	m_InputLayoutPtr->Release();
	m_InputLayoutPtr = nullptr;

	m_VertexBuffer->Release();
	m_VertexBuffer = nullptr;

	m_IndexBuffer->Release();
	m_IndexBuffer = nullptr;

	delete m_EffectPtr;
	m_EffectPtr = nullptr;
}

void Mesh::Render(ID3D11DeviceContext* deviceContextPtr,const Matrix& viewProjectionMatrix) const
{
	m_EffectPtr->UpdateViewProjectionMatrix(viewProjectionMatrix);


	deviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContextPtr->IASetInputLayout(m_InputLayoutPtr);

	constexpr UINT stride = sizeof(ModelVertex);
	constexpr UINT offset = 0;

	deviceContextPtr->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	deviceContextPtr->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT,0);

	D3DX11_TECHNIQUE_DESC techDesc{};
	m_EffectPtr->GetTechniquePtr()->GetDesc(&techDesc);
	for (UINT passIndex = 0; passIndex < techDesc.Passes; passIndex++)
	{
		m_EffectPtr->GetTechniquePtr()->GetPassByIndex(passIndex)->Apply(0, deviceContextPtr);
		deviceContextPtr->DrawIndexed(m_IndicesCount, 0, 0);
	}
}	
