#include <vector>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Vertex.h"
#include "TextureCubeApp.h"

void TextureCubeApp::loadModel() {
	// loadModelFromFile();
	loadTextureCube();
}

void TextureCubeApp::loadModelFromFile(const std::string fileName) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	std::string model_path = fileName.empty() ? MODEL_PATH : fileName;
	// Load mesh from file
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str())) {
		throw std::runtime_error(warn + err);
	}
	// Lookup table to register vertex's index (and hence remove duplicates)
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	// process the mesh into our data arrays
	for (const auto& shape : shapes) {
		// Since the triangulation feature has already made sure that there are three vertices
		// per face, We can now directly iterate over the vertices and dump them straight 
		// into our vertices vector:
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.normal = { 1.0f, 1.0f, 1.0f };
			// We have not seen this vertex before, push into the vertex array
			// and register it the lookup table
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(mVertices.size());
				mVertices.push_back(vertex);
			}
			// Query the look up table to get this vertex index
			mIndices.push_back(uniqueVertices[vertex]);
		}
	}
}

void TextureCubeApp::loadTextureCube() {
	// In case we had some previos model load
	mVertices.clear();
	mIndices.clear();

	std::vector<glm::vec3> positions(8);
	std::vector<glm::vec3> normals(6);
	std::vector<glm::vec2> textCoords(4);
	// Scale is to have a unit cube
	positions[0] = 0.5f * glm::vec3(-1.0f, -1.0f, -1.0f);
	positions[1] = 0.5f * glm::vec3(1.0f, -1.0f, -1.0f);
	positions[2] = 0.5f * glm::vec3(1.0f, 1.0f, -1.0f);
	positions[3] = 0.5f * glm::vec3(-1.0f, 1.0f, -1.0f);
	positions[4] = 0.5f * glm::vec3(-1.0f, -1.0f, 1.0f);
	positions[5] = 0.5f * glm::vec3(1.0f, -1.0f, 1.0f);
	positions[6] = 0.5f * glm::vec3(1.0f, 1.0f, 1.0f);
	positions[7] = 0.5f * glm::vec3(-1.0f, 1.0f, 1.0f);

	normals[0] = glm::vec3(0.0f, 0.0f, -1.0f);
	normals[1] = glm::vec3(0.0f, -1.0f, 0.0f);
	normals[2] = glm::vec3(-1.0f, 0.0f, 0.0f);
	normals[3] = glm::vec3(0.0f, 0.0f, 1.0f);
	normals[4] = glm::vec3(0.0f, 1.0f, 0.0f);
	normals[5] = glm::vec3(1.0f, 0.0f, 0.0f);

	textCoords[0] = glm::vec2(0.0f, 0.0f);
	textCoords[1] = glm::vec2(0.0f, 1.0f);
	textCoords[2] = glm::vec2(1.0f, 1.0f);
	textCoords[3] = glm::vec2(1.0f, 0.0f);

	//Assemble vertexes and fill indices
	Vertex v;
	//Back face of the cube
	v.pos = positions[0]; v.normal = normals[0]; v.texCoord = textCoords[2]; mVertices.push_back(v); //0
	v.pos = positions[1]; v.normal = normals[0]; v.texCoord = textCoords[1]; mVertices.push_back(v); //1
	v.pos = positions[2]; v.normal = normals[0]; v.texCoord = textCoords[0]; mVertices.push_back(v); //2
	v.pos = positions[3]; v.normal = normals[0]; v.texCoord = textCoords[3]; mVertices.push_back(v); //3
	mIndices.push_back(2);
	mIndices.push_back(1);
	mIndices.push_back(0);
	mIndices.push_back(3);
	mIndices.push_back(2);
	mIndices.push_back(0);
	//Bottom face of the cube
	v.pos = positions[0]; v.normal = normals[1]; v.texCoord = textCoords[3]; mVertices.push_back(v); //4
	v.pos = positions[1]; v.normal = normals[1]; v.texCoord = textCoords[0]; mVertices.push_back(v); //5
	v.pos = positions[5]; v.normal = normals[1]; v.texCoord = textCoords[1]; mVertices.push_back(v); //6
	v.pos = positions[4]; v.normal = normals[1]; v.texCoord = textCoords[2]; mVertices.push_back(v); //7
	mIndices.push_back(4);
	mIndices.push_back(5);
	mIndices.push_back(6);
	mIndices.push_back(4);
	mIndices.push_back(6);
	mIndices.push_back(7);
	//Left face of the cube
	v.pos = positions[0]; v.normal = normals[2]; v.texCoord = textCoords[1]; mVertices.push_back(v); //8
	v.pos = positions[3]; v.normal = normals[2]; v.texCoord = textCoords[0]; mVertices.push_back(v); //9
	v.pos = positions[4]; v.normal = normals[2]; v.texCoord = textCoords[2]; mVertices.push_back(v); //10
	v.pos = positions[7]; v.normal = normals[2]; v.texCoord = textCoords[3]; mVertices.push_back(v); //11
	mIndices.push_back(8);
	mIndices.push_back(10);
	mIndices.push_back(11);
	mIndices.push_back(8);
	mIndices.push_back(11);
	mIndices.push_back(9);
	//Top face of the cube
	v.pos = positions[2]; v.normal = normals[4]; v.texCoord = textCoords[3]; mVertices.push_back(v); //12
	v.pos = positions[3]; v.normal = normals[4]; v.texCoord = textCoords[0]; mVertices.push_back(v); //13
	v.pos = positions[6]; v.normal = normals[4]; v.texCoord = textCoords[2]; mVertices.push_back(v); //14
	v.pos = positions[7]; v.normal = normals[4]; v.texCoord = textCoords[1]; mVertices.push_back(v); //15
	mIndices.push_back(12);
	mIndices.push_back(13);
	mIndices.push_back(14);
	mIndices.push_back(13);
	mIndices.push_back(15);
	mIndices.push_back(14);
	//Right face of the cube
	v.pos = positions[1]; v.normal = normals[5]; v.texCoord = textCoords[2]; mVertices.push_back(v); //16
	v.pos = positions[2]; v.normal = normals[5]; v.texCoord = textCoords[3]; mVertices.push_back(v); //17
	v.pos = positions[5]; v.normal = normals[5]; v.texCoord = textCoords[1]; mVertices.push_back(v); //18
	v.pos = positions[6]; v.normal = normals[5]; v.texCoord = textCoords[0]; mVertices.push_back(v); //19
	mIndices.push_back(16);
	mIndices.push_back(17);
	mIndices.push_back(18);
	mIndices.push_back(17);
	mIndices.push_back(19);
	mIndices.push_back(18);
	//Front face of the cube
	v.pos = positions[4]; v.normal = normals[3]; v.texCoord = textCoords[1]; mVertices.push_back(v); //20
	v.pos = positions[5]; v.normal = normals[3]; v.texCoord = textCoords[2]; mVertices.push_back(v); //21
	v.pos = positions[6]; v.normal = normals[3]; v.texCoord = textCoords[3]; mVertices.push_back(v); //22
	v.pos = positions[7]; v.normal = normals[3]; v.texCoord = textCoords[0]; mVertices.push_back(v); //23
	mIndices.push_back(20);
	mIndices.push_back(21);
	mIndices.push_back(22);
	mIndices.push_back(20);
	mIndices.push_back(22);
	mIndices.push_back(23);
}