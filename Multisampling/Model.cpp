#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "MultisamplingApp.h"

void MultisamplingApp::loadModel() {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	// Load mesh from file
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
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