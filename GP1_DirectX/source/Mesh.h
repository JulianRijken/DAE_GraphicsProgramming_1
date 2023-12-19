#pragma once
#include <map>

namespace dae
{
	class Texture;
}

class Effect;

struct VertexModel;
struct TransformedVertex;
struct Material;


class Mesh
{
public:

	// Loads mesh from obj file
	Mesh(ID3D11Device* devicePtr, const std::string& objName,const std::vector<Material*>& materials);

	// Loads mesh from obj file and mtl file. Requires global material map to store material ptrs
	Mesh(ID3D11Device* devicePtr, const std::string& objName, const std::string& mtlName, std::map<std::string, Material*>& materialMap);

	// Used for basic hand made vertices and indices
	Mesh(ID3D11Device* devicePtr, const std::vector<VertexModel>& vertices, const std::vector<uint32_t>& indices, const std::vector<Material*>& materials);


    ~Mesh();

    void Render(ID3D11DeviceContext* deviceContextPtr,const dae::Matrix& viewProjectionMatrix) const;

	void SetPosition(dae::Vector3 translate);
	void SetScale(dae::Vector3 scale);
	void SetYawRotation(float yaw);

	void AddYawRotation(float yawDelta);

private:

	void InitializeEffect(ID3D11Device* devicePtr);
	void InitializeMesh(ID3D11Device* devicePtr);

	void UpdateWorldMatrix();


	std::vector<Material*> m_MaterialPtrs;

    Effect* m_EffectPtr{ nullptr };

    ID3D11Buffer* m_VertexBufferPtr{nullptr};
    ID3D11Buffer* m_IndexBufferPtr{ nullptr };
    ID3D11InputLayout* m_InputLayoutPtr{ nullptr };

    std::vector<VertexModel> m_ModelVertices{};
    std::vector<uint32_t> m_Indices{};
    UINT m_IndicesCount{0};

	dae::Matrix	m_WorldMatrix
	{
		{1.0f,0.0f,0.0f,0.0f},
		{0.0f,1.0f,0.0f,0.0f},
		{0.0f,0.0f,1.0f,0.0f},
		{0.0f,0.0f,0.0f,1.0f}
	};

	float m_YawRotation;
	dae::Vector3 m_Scale;
	dae::Vector3 m_Position;
};


struct VertexModel
{
	dae::Vector3 position;
	dae::Vector3 color;
	dae::Vector2 textureUV;
	dae::Vector3 tangent;
	dae::Vector3 normal;
	int materialIndex;
};



struct Material
{
	dae::Texture* diffuse = nullptr;
	dae::Texture* opacity = nullptr;
	dae::Texture* normal = nullptr;
	dae::Texture* specular = nullptr;
	dae::Texture* gloss = nullptr;
};