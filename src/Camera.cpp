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

void Camera::processScroll(float yoffset) {
    radius -= yoffset * 0.3f;
    radius = glm::clamp(radius, 0.5f, 50.0f);  // Limites de zoom
}

void Camera::processPan(float dx, float dy) {
    float panSpeed = 0.002f * radius;

    // Vecteurs de base de la caméra
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 forward = glm::normalize(target - getPosition());
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    glm::vec3 camUp = glm::normalize(glm::cross(right, forward));

    target += -right * dx * panSpeed + camUp * dy * panSpeed;
}

