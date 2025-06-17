//
// Created by robai on 17/06/2025.
//
#include "../include/Camera.hpp"
#include "../external/glm/glm/gtc/matrix_transform.hpp"
//#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

glm::vec3 Camera::getPosition() const {
    float x = radius * sin(phi) * cos(theta);
    float y = radius * cos(phi);
    float z = radius * sin(phi) * sin(theta);
    return target + glm::vec3(x, y, z);
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(getPosition(), target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::processMouseMovement(float xpos, float ypos) {
    float sensitivity = 0.005f;
    float dx = xpos - lastX;
    float dy = ypos - lastY;

    theta -= dx * sensitivity;
    phi -= dy * sensitivity;

    // Clamp phi pour éviter les renversements
    const float epsilon = 0.1f;
    phi = glm::clamp(phi, epsilon, glm::pi<float>() - epsilon);

    lastX = xpos;
    lastY = ypos;
}
