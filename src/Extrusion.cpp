//
// Created by robai on 17/06/2025.
//

#include "../include/Extrusion.hpp"

Mesh extrudeLinear(const std::vector<glm::vec2>& profile, float height, float scaleTop) {
    Mesh mesh;

    int n = profile.size();
    if (n < 2) return mesh;

    // Étape 1 : base (z=0)
    for (const auto& p : profile)
        mesh.vertices.push_back(glm::vec3(p, 0.0f));

    // Étape 2 : top (z=height), avec mise à l’échelle
    for (const auto& p : profile)
        mesh.vertices.push_back(glm::vec3(p * scaleTop, height));

    // Étape 3 : générer les faces latérales (triangles)
    for (int i = 0; i < n; ++i) {
        int next = (i + 1) % n;

        // Triangle 1
        mesh.indices.push_back(i);
        mesh.indices.push_back(next);
        mesh.indices.push_back(n + next);

        // Triangle 2
        mesh.indices.push_back(i);
        mesh.indices.push_back(n + next);
        mesh.indices.push_back(n + i);
    }

    // Étape 4 (optionnel) : générer les normales (approximatives)
    mesh.normals.resize(mesh.vertices.size(), glm::vec3(0.0f));
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        unsigned int i0 = mesh.indices[i];
        unsigned int i1 = mesh.indices[i + 1];
        unsigned int i2 = mesh.indices[i + 2];

        glm::vec3 v0 = mesh.vertices[i0];
        glm::vec3 v1 = mesh.vertices[i1];
        glm::vec3 v2 = mesh.vertices[i2];

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        mesh.normals[i0] += normal;
        mesh.normals[i1] += normal;
        mesh.normals[i2] += normal;
    }

    // Normalize les normales
    for (auto& n : mesh.normals)
        n = glm::normalize(n);

    return mesh;
}
