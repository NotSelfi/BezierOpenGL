//
// Created by robai on 17/06/2025.
//
#define GLM_ENABLE_EXPERIMENTAL

#include "../include/Extrusion.hpp"
#include "../external/glm/glm/glm.hpp"
#include "../external/glm/glm/gtx/transform.hpp"
#include "../external/glm/glm/gtc/constants.hpp"

float pi = 3.14159265358;

Mesh extrudeLinear(const std::vector<glm::vec2>& profile, float height, float scaleTop) {
    Mesh mesh;
    int n = profile.size();
    if (n < 3) return mesh;  // au moins un polygone

    // Étape 1 : base (z=0)
    for (const auto& p : profile)
        mesh.vertices.push_back(glm::vec3(p, 0.0f));

    // Étape 2 : top (z=height)
    for (const auto& p : profile)
        mesh.vertices.push_back(glm::vec3(p * scaleTop, height));

    // Étape 3 : faces latérales
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

    // Étape 4 : face inférieure (z=0)
    for (int i = 1; i < n - 1; ++i) {
        mesh.indices.push_back(0);
        mesh.indices.push_back(i);
        mesh.indices.push_back(i + 1);
    }

    // Étape 5 : face supérieure (z=height)
    for (int i = 1; i < n - 1; ++i) {
        mesh.indices.push_back(n);
        mesh.indices.push_back(n + i + 1);
        mesh.indices.push_back(n + i);
    }

    // Étape 6 : Normales
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

    for (auto& n : mesh.normals)
        n = glm::normalize(n);

    return mesh;
}

Mesh extrudeRevolution(const std::vector<glm::vec2>& profile, int slices = 36) {
    Mesh mesh;
    int n = profile.size();
    if (n < 2) return mesh;

    // Étape 1 : Générer les vertex
    for (int i = 0; i <= slices; ++i) {
        float theta = (float)i / slices * 2.0f * pi;
        float c = cos(theta);
        float s = sin(theta);
        for (const auto& p : profile) {
            glm::vec3 rotated(p.x * c, p.x * s, p.y);  // On tourne autour de l'axe Z
            mesh.vertices.push_back(rotated);
        }
    }

    // Étape 2 : Générer les faces
    int ring = n;
    for (int i = 0; i < slices; ++i) {
        for (int j = 0; j < n - 1; ++j) {
            int curr = i * ring + j;
            int next = (i + 1) * ring + j;

            mesh.indices.push_back(curr);
            mesh.indices.push_back(next);
            mesh.indices.push_back(curr + 1);

            mesh.indices.push_back(curr + 1);
            mesh.indices.push_back(next);
            mesh.indices.push_back(next + 1);
        }
    }

    // Étape 3 : Normales (comme d’habitude)
    mesh.normals.resize(mesh.vertices.size(), glm::vec3(0.0f));
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        auto i0 = mesh.indices[i], i1 = mesh.indices[i+1], i2 = mesh.indices[i+2];
        glm::vec3 v0 = mesh.vertices[i0], v1 = mesh.vertices[i1], v2 = mesh.vertices[i2];
        glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        mesh.normals[i0] += n;
        mesh.normals[i1] += n;
        mesh.normals[i2] += n;
    }
    for (auto& n : mesh.normals) n = glm::normalize(n);

    return mesh;
}
Mesh extrudeGeneralized(const std::vector<glm::vec2>& profile, const std::vector<glm::vec3>& path) {
    Mesh mesh;
    if (profile.empty() || path.size() < 2) return mesh;

    size_t profileSize = profile.size();
    size_t pathSize = path.size();

    // Génération des sommets
    for (size_t i = 0; i < pathSize; ++i) {
        glm::vec3 tangent;
        if (i < pathSize - 1)
            tangent = glm::normalize(path[i + 1] - path[i]);
        else
            tangent = glm::normalize(path[i] - path[i - 1]);

        // Orthonormal basis
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(tangent, up)) > 0.99f) {
            up = glm::vec3(1.0f, 0.0f, 0.0f); // éviter colinéarité
        }
        glm::vec3 side = glm::normalize(glm::cross(up, tangent));
        glm::vec3 normal = glm::normalize(glm::cross(tangent, side));

        glm::mat3 frame(side, normal, tangent); // colonne = axes

        for (const auto& p : profile) {
            glm::vec3 local(p.x, p.y, 0.0f);
            glm::vec3 worldPos = path[i] + frame * local;
            mesh.vertices.push_back(worldPos);
        }
    }

    // Génération des indices
    for (size_t i = 0; i < pathSize - 1; ++i) {
        for (size_t j = 0; j < profileSize; ++j) {
            int curr = i * profileSize + j;
            int next = curr + profileSize;
            int next_j = (j + 1) % profileSize;
            int curr_next = i * profileSize + next_j;
            int next_next = curr_next + profileSize;

            mesh.indices.push_back(curr);
            mesh.indices.push_back(curr_next);
            mesh.indices.push_back(next);

            mesh.indices.push_back(next);
            mesh.indices.push_back(curr_next);
            mesh.indices.push_back(next_next);
        }
    }

    // Calculs des normales
    mesh.computeNormals();
    return mesh;
}
