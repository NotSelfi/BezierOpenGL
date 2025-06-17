//
// Created by robai on 17/06/2025.
//

#ifndef CAMERA_H
#define CAMERA_H
#pragma once
#include "../external/glm/glm/glm.hpp"

class Camera {
public:
    float radius = 3.0f;
    float theta = 0.0f;
    float phi = 0.0f;
    glm::vec3 target = glm::vec3(0.0f);

    float lastX = 0.0f, lastY = 0.0f;
    bool rotating = false;

    glm::vec3 getPosition() const;
    glm::mat4 getViewMatrix() const;
    void processMouseMovement(float xpos, float ypos);
};
#endif //CAMERA_H
