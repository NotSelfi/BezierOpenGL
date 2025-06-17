//
// Created by robai on 17/06/2025.
//

#ifndef MESH_H
#define MESH_H
#pragma once
#include <vector>
#include "../external/glm/glm/glm.hpp"

struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
};
#endif //MESH_H
