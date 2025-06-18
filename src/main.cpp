#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "../include/Extrusion.hpp"
#include "../include/Mesh.hpp"
#include "../include/BezierCurveData.hpp"
#include "../include/Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "../external/imgui/imgui.h"
#include "../external/imgui/backends/imgui_impl_glfw.h"
#include "../external/imgui/backends/imgui_impl_opengl3.h"

Camera camera;
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;
GLFWwindow* window = nullptr;

Mesh extrudedMesh;
bool showExtrusion = false;
bool revolutionMode = false;

std::vector<BezierCurveData> curves;
int currentCurveIndex = -1;
BezierMethod currentMethod = BezierMethod::DeCasteljau;
int p_courbe = 100;
bool rotating = false;
glm::vec3 lightPosition = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor = glm::vec3(0.8f, 0.5f, 0.2f);
int renderMode = 0; // 0 = plein, 1 = filaire

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        rotating = (action == GLFW_PRESS);
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        camera.lastX = (float)x;
        camera.lastY = (float)y;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float x = (2.0f * xpos) / width - 1.0f;
        float y = 1.0f - (2.0f * ypos) / height;
        if (currentCurveIndex != -1) {
            curves[currentCurveIndex].controlPoints.push_back(glm::vec2(x, y));
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (rotating) {
        camera.processMouseMovement((float)xpos, (float)ypos);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processScroll((float)yoffset);
}

void processInput(GLFWwindow* window) {
    float panAmount = 5.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.processPan(1.0f * panAmount, 0);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.processPan(-1.0f * panAmount, 0);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.processPan(0, -1.0f * panAmount);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.processPan(0, 1.0f * panAmount);
}

std::vector<glm::vec2> generateCurvePoints(const BezierCurveData& curve, BezierMethod method, int p_courbe) {
    std::vector<glm::vec2> result;
    if (curve.controlPoints.size() < 2) return {};
    for (int i = 0; i <= p_courbe; ++i) {
        float t = i / (float)p_courbe;
        result.push_back(curve.evaluate(t, method));
    }
    return result;
}

void drawCurve2D() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    for (int i = 0; i < curves.size(); ++i) {
        auto pts = generateCurvePoints(curves[i], currentMethod, p_courbe);
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_LINE_STRIP);
        for (auto& pt : pts) glVertex2f(pt.x, pt.y);
        glEnd();

        glPointSize(5.0f);
        glBegin(GL_POINTS);
        for (auto& pt : curves[i].controlPoints) glVertex2f(pt.x, pt.y);
        glEnd();
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void drawMesh(const Mesh& mesh) {
    glPushMatrix();
    glColor3f(objectColor.r, objectColor.g, objectColor.b);
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        unsigned int idx = mesh.indices[i];
        if (idx < mesh.normals.size()) {
            const glm::vec3& normal = mesh.normals[idx];
            glNormal3f(normal.x, normal.y, normal.z);
        }
        const glm::vec3& v = mesh.vertices[idx];
        glVertex3f(v.x, v.y, v.z);
    }
    glEnd();
    glPopMatrix();
}

void drawAxes() {
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(1,0,0);
    glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,1,0);
    glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,1);
    glEnd();
    glEnable(GL_LIGHTING);
}

void setupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void cleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

int main() {
    if (!glfwInit()) return -1;

    window = glfwCreateWindow(WIDTH, HEIGHT, "Bezier Debug", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    setupImGui();

    curves.emplace_back();
    currentCurveIndex = 0;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Contrôle");
        if (ImGui::RadioButton("De Casteljau", currentMethod == BezierMethod::DeCasteljau))
            currentMethod = BezierMethod::DeCasteljau;
        if (ImGui::RadioButton("Formule directe", currentMethod == BezierMethod::DirectFormula))
            currentMethod = BezierMethod::DirectFormula;

        if (ImGui::Button("Nouvelle courbe")) {
            curves.emplace_back();
            currentCurveIndex = (int)curves.size() - 1;
        }

        if (currentCurveIndex >= 0 && currentCurveIndex < curves.size()) {
            ImGui::Text("Courbe active : %d", currentCurveIndex);
            if (ImGui::Button("Fermer C0")) curves[currentCurveIndex].closeCurveC0();
            ImGui::SameLine();
            if (ImGui::Button("Fermer C1")) curves[currentCurveIndex].closeCurveC1();
            ImGui::SameLine();
            if (ImGui::Button("Fermer C2")) curves[currentCurveIndex].closeCurveC2();
        }

        static float height = 1.0f;
        static float scaleTop = 1.0f;
        static int slices = 36;
        ImGui::SliderFloat("Hauteur", &height, 0.1f, 5.0f);
        ImGui::SliderFloat("Echelle top", &scaleTop, 0.1f, 2.0f);
        ImGui::SliderInt("Révol. segments", &slices, 3, 100);
        ImGui::Checkbox("Mode révolution", &revolutionMode);

        if (ImGui::Button("Générer extrusion") && currentCurveIndex != -1) {
            auto profile2D = generateCurvePoints(curves[currentCurveIndex], currentMethod, p_courbe);
            if (revolutionMode)
                extrudedMesh = extrudeRevolution(profile2D, slices);
            else
                extrudedMesh = extrudeLinear(profile2D, height, scaleTop);
            showExtrusion = true;
        }

        ImGui::Checkbox("Afficher extrusion", &showExtrusion);
        ImGui::SeparatorText("Lumière");
        ImGui::SliderFloat3("Position lumière", &lightPosition.x, -5.0f, 5.0f);
        ImGui::ColorEdit3("Couleur objet", &objectColor.x);

        ImGui::SeparatorText("Rendu");
        ImGui::RadioButton("Mode plein", &renderMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Mode filaire", &renderMode, 1);

        ImGui::End();

        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLfloat light_pos[] = { lightPosition.x, lightPosition.y, lightPosition.z, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

        if (renderMode == 0)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else if (renderMode == 1)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        if (showExtrusion) {
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.getViewMatrix();
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(&projection[0][0]);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(&view[0][0]);

            drawAxes();
            drawMesh(extrudedMesh);
        }

        drawCurve2D();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    cleanupImGui();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}


