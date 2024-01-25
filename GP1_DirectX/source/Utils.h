#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

#include "Mesh.h"
#include "GlobalSettings.h"
#include "Texture.h"


namespace dae
{
	namespace Utils
	{
		static bool ParseOBJ(const std::string& filename, std::vector<VertexModel>& vertices, std::vector<uint32_t>& indices, bool flipAxisAndWinding = true)
		{
			std::ifstream file(RESOURCES_PATH + filename);
			if (!file)
				return false;

			std::vector<Vector3> positions{};
			std::vector<Vector3> normals{};
			std::vector<Vector2> UVs{};

			vertices.clear();
			indices.clear();

			// TEMP
			int materialIndex{ 0 };


			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				// std::cout << "file command: " << sCommand << "Vertices: " << vertices.size() << "\n";


				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				if (sCommand == "usemtl")
				{
					std::string name;
					file >> name;

					materialIndex++;
				}
				else if (sCommand == "v")
				{
					//VertexModel
					float x, y, z;
					file >> x >> y >> z;

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// VertexModel TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// VertexModel Normal
					float x, y, z;
					file >> x >> y >> z;

					normals.emplace_back(x, y, z);
				}
				else if (sCommand == "f")
				{
					//if a face is read:
					//construct the 3 m_VerticesModel, add them to the vertex array
					//add three indices to the index array
					//add the material index as attibute to the attribute array
					//
					// Faces or triangles
					VertexModel vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.position = positions[iPosition - 1];

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.textureUV = UVs[iTexCoord - 1];

								vertex.materialIndex = materialIndex;
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.normal = normals[iNormal - 1];
							}
						}

						vertices.push_back(vertex);
						tempIndices[iFace] = uint32_t(vertices.size()) - 1;
						//indices.push_back(uint32_t(m_VerticesModel.size()) - 1);
					}

					indices.push_back(tempIndices[0]);
					if (flipAxisAndWinding)
					{
						indices.push_back(tempIndices[2]);
						indices.push_back(tempIndices[1]);
					}
					else
					{
						indices.push_back(tempIndices[1]);
						indices.push_back(tempIndices[2]);
					}
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');
			}

			//Cheap Tangent Calculations
			for (uint32_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t index0 = indices[i];
				uint32_t index1 = indices[static_cast<size_t>(i) + 1];
				uint32_t index2 = indices[static_cast<size_t>(i) + 2];

				const Vector3& p0 = vertices[index0].position;
				const Vector3& p1 = vertices[index1].position;
				const Vector3& p2 = vertices[index2].position;
				const Vector2& uv0 = vertices[index0].textureUV;
				const Vector2& uv1 = vertices[index1].textureUV;
				const Vector2& uv2 = vertices[index2].textureUV;

				const Vector3 edge0 = p1 - p0;
				const Vector3 edge1 = p2 - p0;
				const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
				const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
				float r = 1.f / Vector2::Cross(diffX, diffY);

				Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
				vertices[index0].tangent += tangent;
				vertices[index1].tangent += tangent;
				vertices[index2].tangent += tangent;
			}

			//Fix the tangents per vertex now because we accumulated
			for (VertexModel& vertex : vertices)
			{
				vertex.tangent = Vector3::Reject(vertex.tangent, vertex.normal).Normalized();

				if (flipAxisAndWinding)
				{
					vertex.position.z *= -1.f;
					vertex.normal.z *= -1.f;
					vertex.tangent.z *= -1.f;
				}
			}

			return true;
		}

		static bool ParseOBJ(const std::string& filename, std::vector<VertexModel>& vertices, std::vector<uint32_t>& indices, std::vector<std::string>& mappedMaterials, bool flipAxisAndWinding = true)
		{
			std::ifstream file(RESOURCES_PATH + filename);

			if (!file)
				return false;

			std::cout << "PARSING OBJ" << std::endl;

			std::vector<Vector3> positions{};
			std::vector<Vector3> normals{};
			std::vector<Vector2> UVs{};

			mappedMaterials.clear();
			vertices.clear();
			indices.clear();

			// Starts with no index as usemtl has to be called first
			int materialIndex{ -1 };

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;


				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				if (sCommand == "usemtl")
				{
					std::string name;
					file >> name;

					mappedMaterials.push_back(name);
					materialIndex++;

					std::cout << "usemtl: " << name << " Index: " << materialIndex << std::endl;
				}
				else if (sCommand == "v")
				{
					//VertexModel
					float x, y, z;
					file >> x >> y >> z;

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// VertexModel TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// VertexModel Normal
					float x, y, z;
					file >> x >> y >> z;

					normals.emplace_back(x, y, z);
				}
				else if (sCommand == "f")
				{
					//if a face is read:
					//construct the 3 m_VerticesModel, add them to the vertex array
					//add three indices to the index array
					//add the material index as attibute to the attribute array
					//
					// Faces or triangles
					VertexModel vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.position = positions[iPosition - 1];

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.textureUV = UVs[iTexCoord - 1];

								vertex.materialIndex = materialIndex;
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.normal = normals[iNormal - 1];
							}
						}

						vertices.push_back(vertex);
						tempIndices[iFace] = static_cast<uint32_t>(vertices.size()) - 1;
					}

					indices.push_back(tempIndices[0]);
					if (flipAxisAndWinding)
					{
						indices.push_back(tempIndices[2]);
						indices.push_back(tempIndices[1]);
					}
					else
					{
						indices.push_back(tempIndices[1]);
						indices.push_back(tempIndices[2]);
					}
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');
			}

			//Cheap Tangent Calculations
			for (uint32_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t index0 = indices[i];
				uint32_t index1 = indices[static_cast<size_t>(i) + 1];
				uint32_t index2 = indices[static_cast<size_t>(i) + 2];

				const Vector3& p0 = vertices[index0].position;
				const Vector3& p1 = vertices[index1].position;
				const Vector3& p2 = vertices[index2].position;
				const Vector2& uv0 = vertices[index0].textureUV;
				const Vector2& uv1 = vertices[index1].textureUV;
				const Vector2& uv2 = vertices[index2].textureUV;

				const Vector3 edge0 = p1 - p0;
				const Vector3 edge1 = p2 - p0;
				const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
				const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
				float r = 1.f / Vector2::Cross(diffX, diffY);

				Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
				vertices[index0].tangent += tangent;
				vertices[index1].tangent += tangent;
				vertices[index2].tangent += tangent;
			}

			//Fix the tangents per vertex now because we accumulated
			for (VertexModel& vertex : vertices)
			{
				vertex.tangent = Vector3::Reject(vertex.tangent, vertex.normal).Normalized();

				if (flipAxisAndWinding)
				{
					vertex.position.z *= -1.f;
					vertex.normal.z *= -1.f;
					vertex.tangent.z *= -1.f;
				}
			}

			std::cout << std::endl;

			return true;
		}

		static bool ParseMTL(ID3D11Device* devicePtr, const std::string& filename, std::map<std::string, Material*>& materials, const std::string& pathPrefix = "")
		{
			std::ifstream file(RESOURCES_PATH + pathPrefix + filename);

			if (!file)
				return false;

			std::cout << "PARSING MTL" << std::endl;

			materials.clear();

			std::string command;
			std::string currentMaterialName;

			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> command;

				//use conditional statements to process the different commands	
				if (command == "#")
				{
					// Ignore Comment
				}
				if (command == "newmtl")
				{
					std::string name;
					file >> name;

					std::cout << name << std::endl;

					// Add material to map
					if (!materials.contains(name))
						materials.insert({ name, new Material{} });

					currentMaterialName = name;
				}
				else if (command == "map_Kd")
				{
					std::string texturePath;
					file >> texturePath;

					std::string fullTexturePath = pathPrefix + texturePath;

					std::cout << "Set Kd: " << fullTexturePath << std::endl;
					materials[currentMaterialName]->diffuse = Texture::LoadFromFile(devicePtr,fullTexturePath);
				}
				else if (command == "map_d")
				{
					std::string texturePath;
					file >> texturePath;

					std::string fullTexturePath = pathPrefix + texturePath;
					
					std::cout << "Set d: " << fullTexturePath << std::endl;
					materials[currentMaterialName]->opacity = Texture::LoadFromFile(devicePtr,fullTexturePath);
				}
				else if (command == "bump")
				{
					std::string texturePath;
					file >> texturePath;

					std::string fullTexturePath = pathPrefix + texturePath;
					
					std::cout << "Set bump: " << fullTexturePath << std::endl;
					materials[currentMaterialName]->normal = Texture::LoadFromFile(devicePtr,fullTexturePath);
				}


				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');
			}


			std::cout << std::endl;
			return true;
		}
		
		static bool ParseObjAndCreateMeshes(ID3D11Device* devicePtr, EffectBase* effect, const std::string& fileName, std::vector<Mesh*>& meshes,
		                                    const std::map<std::string, Material*>& materials, const std::string& pathPrefix = "",
		                                    bool flipAxisAndWinding = true)
		{

			std::ifstream file(RESOURCES_PATH + pathPrefix + fileName);

			if (!file)
				return false;

			std::cout << "PARSING OBJ" << std::endl;


			// Clear the vector we are going to fill
			meshes.clear();
			
			std::vector<Vector3> positions{};
			std::vector<Vector3> normals{};
			std::vector<Vector2> UVs{};
			
			std::vector<VertexModel> vertices{};
 			std::vector<uint32_t> indices{};

			// Store the active material name
			std::string lastName{};
			std::string newName{};
			bool newMaterial{ false };
			
			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (not file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				

				if (sCommand == "usemtl")
				{
					file >> newName;
					newMaterial = true;
				}
				else if (sCommand == "v")
				{
					//VertexModel
					float x, y, z;
					file >> x >> y >> z;

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// VertexModel TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// VertexModel Normal
					float x, y, z;
					file >> x >> y >> z;

					normals.emplace_back(x, y, z);
				}
				else if (sCommand == "f")
				{
					//if a face is read:
					//construct the 3 m_VerticesModel, add them to the vertex array
					//add three indices to the index array
					//add the material index as attibute to the attribute array
					//
					// Faces or triangles
					VertexModel vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.position = positions[iPosition - 1];

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.textureUV = UVs[iTexCoord - 1];
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.normal = normals[iNormal - 1];
							}
						}

						vertices.push_back(vertex);
						tempIndices[iFace] = static_cast<uint32_t>(vertices.size()) - 1;
					}

					indices.push_back(tempIndices[0]);
					if (flipAxisAndWinding)
					{
						indices.push_back(tempIndices[2]);
						indices.push_back(tempIndices[1]);
					}
					else
					{
						indices.push_back(tempIndices[1]);
						indices.push_back(tempIndices[2]);
					}
				}
				
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');


				if(newMaterial)
				{
					//Cheap Tangent Calculations
					for (uint32_t i = 0; i < indices.size(); i += 3)
					{
						uint32_t index0 = indices[i];
						uint32_t index1 = indices[static_cast<size_t>(i) + 1];
						uint32_t index2 = indices[static_cast<size_t>(i) + 2];

						const Vector3& p0 = vertices[index0].position;
						const Vector3& p1 = vertices[index1].position;
						const Vector3& p2 = vertices[index2].position;
						const Vector2& uv0 = vertices[index0].textureUV;
						const Vector2& uv1 = vertices[index1].textureUV;
						const Vector2& uv2 = vertices[index2].textureUV;

						const Vector3 edge0 = p1 - p0;
						const Vector3 edge1 = p2 - p0;
						const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
						const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
						float r = 1.f / Vector2::Cross(diffX, diffY);

						Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
						vertices[index0].tangent += tangent;
						vertices[index1].tangent += tangent;
						vertices[index2].tangent += tangent;
					}

					//Fix the tangents per vertex now because we accumulated
					for (VertexModel& vertex : vertices)
					{
						vertex.tangent = Vector3::Reject(vertex.tangent, vertex.normal).Normalized();

						if (flipAxisAndWinding)
						{
							vertex.position.z *= -1.f;
							vertex.normal.z *= -1.f;
							vertex.tangent.z *= -1.f;
						}
					}

					
					if(!vertices.empty())
					if(!indices.empty())
					if(materials.contains(lastName))
					{
						Mesh* mesh = new Mesh{ devicePtr, effect, vertices, indices,{materials.at(lastName)}};
						meshes.push_back(mesh);
					}

					lastName = newName;

					vertices.clear();
					indices.clear();
					newMaterial = false;
				}
			}



		
			return true;
		}


		
		static float MapValueInRange(float value, float inRangeMin, float inRangeMax, float outRangeMin = 0.0f, float outRangeMax = 1.0f)
		{
			return (value - inRangeMin) * (outRangeMax - outRangeMin) / (inRangeMax - inRangeMin) + outRangeMin;
		}

		static float MapValueInRangeClamped(float value, float inRangeMin, float inRangeMax, float outRangeMin = 0.0f, float outRangeMax = 1.0f)
		{
			return std::ranges::clamp((value - inRangeMin) * (outRangeMax - outRangeMin) / (inRangeMax - inRangeMin) + outRangeMin, outRangeMin, outRangeMax);
		}


		static float Remap(float value, float inputMin, float inputMax, float outputMin, float outputMax)
		{
			if (value <= inputMin) return outputMin;
			if (value >= inputMax) return outputMax;
			return outputMin + (value - inputMin) * (outputMax - outputMin) / (inputMax - inputMin);
		}
	}
}
