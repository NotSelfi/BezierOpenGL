//
// Created by robai on 17/06/2025.
//

#ifndef EXTRUSION_H
#define EXTRUSION_H
#define GLM_ENABLE_EXPERIMENTAL

#pragma once
#pragma once

#include <vector>
#include "../external/glm/glm/glm.hpp"
#include "Mesh.hpp"

Mesh extrudeLinear(const std::vector<glm::vec2>& profile, float height, float scaleTop);
Mesh extrudeRevolution(const std::vector<glm::vec2>& profile, int steps);
Mesh extrudeGeneralized(const std::vector<glm::vec2>& profile2D, const std::vector<glm::vec3>& path3D);

#endif //EXTRUSION_H
