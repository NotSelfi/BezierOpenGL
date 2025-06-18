//
// Created by robai on 18/06/2025.
//
#include "../include/Mesh.hpp"

void Mesh::computeNormals() {
    normals.clear();
    normals.resize(vertices.size(), glm::vec3(0.0f));

    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        glm::vec3 v0 = vertices[i0];
        glm::vec3 v1 = vertices[i1];
        glm::vec3 v2 = vertices[i2];

        glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        normals[i0] += n;
        normals[i1] += n;
        normals[i2] += n;
    }

    for (auto& n : normals) {
        n = glm::normalize(n);
    }
}