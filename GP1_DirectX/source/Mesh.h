#pragma once

class Effect;
using namespace dae;

struct ModelVertex;
struct TransformedVertex;

class Mesh
{
public:
    Mesh(ID3D11Device* devicePtr, const std::vector<ModelVertex>& modelVertices, const std::vector<Uint32>& indices);
    ~Mesh();

    void Render(ID3D11DeviceContext* deviceContextPtr,const Matrix& viewProjectionMatrix) const;

private:

    Effect* m_EffectPtr;

    ID3D11InputLayout* m_InputLayoutPtr;
    ID3D11Buffer* m_VertexBuffer;
    ID3D11Buffer* m_IndexBuffer;

    std::vector<ModelVertex> m_ModelVertices;
    std::vector<uint32_t> m_Indices;
    UINT m_IndicesCount;
};



struct ModelVertex
{
    Vector3 position;
    Vector3 color;
};

struct TransformedVertex
{
    Vector4 position;
    Vector3 color;
};
