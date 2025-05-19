#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include "../include/BezierCurveData.hpp"

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

std::vector<BezierCurveData> curves;
int currentCurveIndex = -1;

BezierMethod currentMethod = BezierMethod::DeCasteljau;
int resolution = 100;

GLFWwindow* window = nullptr;

int selectedPointIndex = -1;
bool isDragging = false;
float pointSelectRadius = 0.05f;

glm::vec2 screenToOpenGL(GLFWwindow* window, double xpos, double ypos)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float x = (2.0f * xpos) / width - 1.0f;
    float y = 1.0f - (2.0f * ypos) / height;

    return glm::vec2(x, y);
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (currentCurveIndex == -1) return; // Pas de courbe active

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 mousePos = screenToOpenGL(window, xpos, ypos);
    BezierCurveData& curve = curves[currentCurveIndex];

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            selectedPointIndex = -1;
            for (int i = 0; i < curve.controlPoints.size(); ++i)
            {
                if (glm::distance(curve.controlPoints[i], mousePos) < pointSelectRadius)
                {
                    selectedPointIndex = i;
                    isDragging = true;
                    break;
                }
            }

            if (selectedPointIndex == -1)
            {
                curve.controlPoints.push_back(mousePos);
                std::cout << "Ajout point sur courbe " << currentCurveIndex << ": " << mousePos.x << ", " << mousePos.y << std::endl;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            isDragging = false;
            selectedPointIndex = -1;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Suppression d’un point proche
        for (int i = 0; i < curve.controlPoints.size(); ++i)
        {
            if (glm::distance(curve.controlPoints[i], mousePos) < pointSelectRadius)
            {
                curve.controlPoints.erase(curve.controlPoints.begin() + i);
                std::cout << "Point supprimé sur courbe " << currentCurveIndex << " à l’indice " << i << std::endl;
                if (curves.empty())
                {
                    currentCurveIndex = -1;
                }
                break;
            }
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (isDragging && currentCurveIndex != -1 && selectedPointIndex != -1)
    {
        glm::vec2 newPos = screenToOpenGL(window, xpos, ypos);
        curves[currentCurveIndex].controlPoints[selectedPointIndex] = newPos;
    }
}

void drawLineStrip(const std::vector<glm::vec2>& points, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_LINE_STRIP);
    for (const auto& p : points) {
        glVertex2f(p.x, p.y);
    }
    glEnd();
}

std::vector<glm::vec2> generateCurvePoints(const BezierCurveData& curve, BezierMethod method, int resolution)
{
    std::vector<glm::vec2> result;
    for (int i = 0; i <= resolution; ++i) {
        float t = i / (float)resolution;
        result.push_back(curve.evaluate(t, method));
    }
    return result;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);

    else if (key == GLFW_KEY_F)
        currentMethod = (currentMethod == BezierMethod::DeCasteljau)
            ? BezierMethod::DirectFormula
            : BezierMethod::DeCasteljau;

    else if (key == GLFW_KEY_M && currentCurveIndex != -1)
        curves[currentCurveIndex].duplicateLastPoint();

    else if (key == GLFW_KEY_EQUAL)
        resolution += 10;

    else if (key == GLFW_KEY_MINUS)
        resolution = std::max(10, resolution - 10);

    else if (key == GLFW_KEY_N)
    {

        curves.emplace_back();
        currentCurveIndex = (int)curves.size() - 1;
        std::cout << "Nouvelle courbe créée, index = " << currentCurveIndex << std::endl;
    }
    else if (key == GLFW_KEY_W && !curves.empty())
    {
        currentCurveIndex = (currentCurveIndex + 1) % curves.size();
        std::cout << "Courbe active index = " << currentCurveIndex << std::endl;
    }
    else if (key == GLFW_KEY_X && !curves.empty())
    {
        currentCurveIndex = (currentCurveIndex - 1 + curves.size()) % curves.size();
        std::cout << "Courbe active index = " << currentCurveIndex << std::endl;
    }
    else if (key == GLFW_KEY_BACKSPACE && !curves.empty())
    {
        std::cout << "Suppression de la courbe " << currentCurveIndex << std::endl;
        curves.erase(curves.begin() + currentCurveIndex);

        if (curves.empty())
        {
            currentCurveIndex = -1;
        }
        else if (currentCurveIndex >= curves.size())
        {
            currentCurveIndex = (int)curves.size() - 1; // Sélectionne la précédente
        }
    }
    else if (currentCurveIndex != -1 &&
            (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT || key == GLFW_KEY_UP ||
             key == GLFW_KEY_DOWN || key == GLFW_KEY_R || key == GLFW_KEY_T ||
             key == GLFW_KEY_Z || key == GLFW_KEY_S || key == GLFW_KEY_H ||
             key == GLFW_KEY_V))
    {
        glm::mat3 matrix = glm::mat3(1.0f);

        if (key == GLFW_KEY_LEFT) matrix[2][0] = -0.1f;
        if (key == GLFW_KEY_RIGHT) matrix[2][0] = 0.1f;
        if (key == GLFW_KEY_UP) matrix[2][1] = 0.1f;
        if (key == GLFW_KEY_DOWN) matrix[2][1] = -0.1f;

        if (key == GLFW_KEY_R){

            float theta = -glm::radians(10.0f);
            matrix = glm::mat3(
                glm::cos(theta), glm::sin(theta), 0.0f,
                -glm::sin(theta), glm::cos(theta), 0.0f,
                0.0f, 0.0f, 1.0f
                );
        }

        if (key == GLFW_KEY_T){

            float theta = glm::radians(10.0f);
            matrix = glm::mat3(
                glm::cos(theta), glm::sin(theta), 0.0f,
                -glm::sin(theta), glm::cos(theta), 0.0f,
                0.0f, 0.0f, 1.0f
                );
        }
        if (key == GLFW_KEY_Z){

            matrix[0][0] = 1.1f;
            matrix[1][1] = 1.1f;
            std::cout << "Test scale up de la courbe " << currentCurveIndex << std::endl;
        }
        if (key == GLFW_KEY_S) {

            matrix[0][0] = 0.9f;
            matrix[1][1] = 0.9f;
            std::cout << "Test scale down de la courbe " << currentCurveIndex << std::endl;
        }
        if (key == GLFW_KEY_H) {

            matrix[1][0] = 0.3f;
        }
        if (key == GLFW_KEY_V){

            matrix[0][1] = 0.3f;
        }
        for (auto& pt : curves[currentCurveIndex].controlPoints)
        {
            glm::vec3 p = glm::vec3(pt, 1.0f);
            glm::vec3 q = matrix * p;
            pt = glm::vec2(q);
        }
        std::cout << "Transformation de la courbe " << currentCurveIndex << std::endl;

    }
    else if (key == GLFW_KEY_M && currentCurveIndex != -1)
    {
        curves[currentCurveIndex].duplicateLastPoint();
    }

    /*else if (currentCurveIndex != -1 && key == GLFW_KEY_1)
    {
        BezierCurveData& curve = curves[currentCurveIndex];
        curve.controlPoints.clear();
        curve.controlPoints.push_back(glm::vec2(0.0f, 0.0f));
        curve.controlPoints.push_back(glm::vec2(1.0f, 0.0f));
    }*/



}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    window = glfwCreateWindow(WIDTH, HEIGHT, "Multi Bezier Curves OpenGL", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // On crée une première courbe vide au démarrage
    curves.emplace_back();
    currentCurveIndex = 0;

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Affiche toutes les courbes
        for (int i = 0; i < curves.size(); ++i)
        {
            float r = (i == currentCurveIndex) ? 1.0f : 0.6f;
            float g = (i == currentCurveIndex) ? 1.0f : 0.6f;
            float b = (i == currentCurveIndex) ? 0.0f : 0.6f;

            // Points de contrôle
            drawLineStrip(curves[i].controlPoints, r * 1.0f, g * 0.2f, b * 0.2f);

            // Courbe Bézier
            if (curves[i].controlPoints.size() >= 2)
            {
                auto bezierPoints = generateCurvePoints(curves[i], currentMethod, resolution);
                drawLineStrip(bezierPoints, r, g, b);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
