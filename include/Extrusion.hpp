//
// Created by robai on 17/06/2025.
//

#ifndef EXTRUSION_H
#define EXTRUSION_H


#pragma once
#pragma once

#include <vector>
#include "../external/glm/glm/glm.hpp"
#include "Mesh.hpp"

Mesh extrudeLinear(const std::vector<glm::vec2>& profile, float height, float scaleTop);
Mesh extrudeRevolution(const std::vector<glm::vec2>& profile, int steps);


#endif //EXTRUSION_H
