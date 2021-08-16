#include "ModelLoader.h"
#include "GameObject.h"
#include "Mesh.h"
#include "RapidXML/rapidxml.hpp"
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <unordered_map>

using namespace DirectX;
using namespace rapidxml;

constexpr char GEOMETRIES_TAG[19] = "library_geometries";
constexpr char GEOMETRY_TAG[9] = "geometry";
constexpr char MESH_TAG[5] = "mesh";
constexpr char SOURCE_TAG[7] = "source";
constexpr char FLOAT_ARRAY_TAG[12] = "float_array";
constexpr char TRIANGLES_TAG[10] = "triangles";
constexpr char P_TAG[2] = "p";

ModelLoader::ModelLoader(const std::string& path)
{
    std::ifstream file;
    file.open(path);

    file.seekg(0, file.end);
    int length = file.tellg();
    file.seekg(0, file.beg);

    char* xml = new char[length + 1];
    xml[length] = 0;

    file.read(xml, length);
    xmlDocument.parse<0>(xml);

    file.close();
}

ModelLoader::~ModelLoader()
{
    xmlDocument.clear();
}

void ModelLoader::LoadModel(GameObjectMap& gameObjects)
{
    xml_node<>* first_node = xmlDocument.first_node();
    xml_node<>* geometries_node = first_node->first_node(GEOMETRIES_TAG);
    xml_node<>* geometry_node = geometries_node->first_node(GEOMETRY_TAG);
    xml_node<>* mesh_node = geometry_node->first_node(MESH_TAG);
    xml_node<>* source_node = mesh_node->first_node(SOURCE_TAG);
    xml_node<>* float_array_node = source_node->first_node(FLOAT_ARRAY_TAG);
    //xml_attribute<>* float_array = source_node->first_attribute(FLOAT_ARRAY_TAG);

    std::unique_ptr<GameObject> gameObject = std::make_unique<GameObject>();
    std::vector<Vertex>& vertices = gameObject->mesh.vertices;
    std::vector<std::uint32_t>& indices = gameObject->mesh.indices;

    char* vertexArray = float_array_node->value();
    std::size_t vertexArraySize = float_array_node->value_size();
    std::string number;
    float vertex[3];
    std::size_t vertexNum = 0;

    for(std::size_t i = 0; i < vertexArraySize + 1; ++i)
    {
        if(vertexArray[i] != ' ' && vertexArray[i] != '\0')
            number.push_back(vertexArray[i]);
        else
        {
            vertex[vertexNum++] = std::stof(number);
            number.clear();
        }

        if(vertexNum == 3)
        {
            vertexNum = 0;
            vertices.push_back(Vertex({ XMFLOAT3(vertex[0], vertex[1], vertex[2]),
                                        XMFLOAT3(0.0f, 1.0f, 0.0f) }));
        }
    }

    //vertices[4].pos.x += 1.0f;
    xml_node<>* triangles_node = mesh_node->first_node(TRIANGLES_TAG);
    xml_node<>* p_node = triangles_node->first_node(P_TAG);

    char* indexArray = p_node->value();
    std::size_t indexArraySize = p_node->value_size();
    number.clear();
    std::size_t j = 1;

    for(std::size_t i = 0; i < indexArraySize + 1; ++i)
    {
        if(indexArray[i] != ' ' && indexArray[i] != '\0')
            number.push_back(indexArray[i]);
        else
        {
            if(j == 1)
                indices.push_back(std::atoi(number.c_str()));

            j = (j + 1) % 3;
            
            number.clear();
        }
    }

    /*
    aiVector3D vertex = mesh->mVertices[i];
    aiVector3D normal = mesh->mNormals[i]; // XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertices.push_back(Vertex({ XMFLOAT3(vertex.x, vertex.y, vertex.z),
                                XMFLOAT3(normal.x, normal.y, normal.z) }));

   
    XMFLOAT4X4& world = gameObject->objectVariables.world;
    world._11 = node->mTransformation.a1;
    world._12 = node->mTransformation.a2;
    world._13 = node->mTransformation.a3;
    world._14 = node->mTransformation.a4;
    world._21 = node->mTransformation.b1;
    world._22 = node->mTransformation.b2;
    world._23 = node->mTransformation.b3;
    world._24 = node->mTransformation.b4;
    world._31 = node->mTransformation.c1;
    world._32 = node->mTransformation.c2;
    world._33 = node->mTransformation.c3;
    world._34 = node->mTransformation.c4;
    world._41 = node->mTransformation.d1;
    world._42 = node->mTransformation.d2;
    world._43 = node->mTransformation.d3;
    world._44 = node->mTransformation.d4;
    */

    gameObjects.insert(std::make_pair("name", std::move(gameObject)));
}

std::vector<XMFLOAT3> ModelLoader::LoadFloat3Vector(const std::string& path)
{
    size_t count;
    std::ifstream file(path);
    file >> count;
    count /= 3;

    std::vector<XMFLOAT3> points;
    points.reserve(count);
    for(int i = 0; i < count; ++i)
    {
        XMFLOAT3 point;
        file >> point.x >> point.y >> point.z;
        points.push_back(point);
    }

    file.close();

    return points;
}