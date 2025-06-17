#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "../include/Extrusion.hpp"
#include "../include/Mesh.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "../external/glm/glm/glm.hpp"
#include "../include/BezierCurveData.hpp"
#include "../include/Camera.hpp"



Camera camera;
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;
Mesh extrudedMesh;
bool showExtrusion = false;

std::vector<BezierCurveData> curves;
int currentCurveIndex = -1;

BezierMethod currentMethod = BezierMethod::DeCasteljau;
int p_courbe = 100;

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

void drawMesh(const Mesh& mesh) {
    glColor3f(0.8f, 0.5f, 0.2f); // couleur
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        unsigned int idx = mesh.indices[i];
        const glm::vec3& v = mesh.vertices[idx];
        glVertex3f(v.x, v.y, v.z);
    }
    glEnd();
}

// Affiche les points sous forme de ligne
void drawLineStrip(const std::vector<glm::vec2>& points, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_LINE_STRIP);
    for (const auto& p : points) {
        glVertex2f(p.x, p.y);
    }
    glEnd();
}

// Calcule le produit vectoriel entre OA et OB
float cross(const glm::vec2& O, const glm::vec2& A, const glm::vec2& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// Génère les points selon la méthode
std::vector<glm::vec2> generateCurvePoints(const BezierCurveData& curve, BezierMethod method, int p_courbe)
{
    std::vector<glm::vec2> result;
    for (int i = 0; i <= p_courbe; ++i) {
        float t = i / (float)p_courbe;
        result.push_back(curve.evaluate(t, method));
    }
    return result;
}

// Enveloppe convexe
std::vector<glm::vec2> grahamScan(std::vector<glm::vec2> points) {
    if (points.size() <= 3)
        return points;

    // Point le plus bas
    std::swap(points[0], *std::min_element(points.begin(), points.end(),
        [](const glm::vec2& a, const glm::vec2& b) {
            return a.y < b.y || (a.y == b.y && a.x < b.x);
        }));

    glm::vec2 p0 = points[0];

    // Trie les points depuis le point le plus bas
    std::sort(points.begin() + 1, points.end(),
        [p0](const glm::vec2& a, const glm::vec2& b) {
            float c = cross(p0, a, b);
            if (c == 0)
                return glm::distance(p0, a) < glm::distance(p0, b);
            return c > 0;
        });

    //Construction de l'enveloppe
    std::vector<glm::vec2> hull;
    for (const auto& pt : points) {
        while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull.back(), pt) <= 0)
            hull.pop_back();
        hull.push_back(pt);
    }

    return hull;
}

//Vérifie Intersection
bool doPolygonsIntersect(const std::vector<glm::vec2>& poly1, const std::vector<glm::vec2>& poly2) {
    auto check = [](const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2) {
        glm::vec2 r = a2 - a1;
        glm::vec2 s = b2 - b1;
        float denom = r.x * s.y - r.y * s.x;
        if (denom == 0.0f) return false;

        float t = ((b1 - a1).x * s.y - (b1 - a1).y * s.x) / denom;
        float u = ((b1 - a1).x * r.y - (b1 - a1).y * r.x) / denom;
        return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
    };

    // Comparer chaques segments
    for (int i = 0; i < poly1.size(); ++i) {
        glm::vec2 a1 = poly1[i];
        glm::vec2 a2 = poly1[(i + 1) % poly1.size()];
        for (int j = 0; j < poly2.size(); ++j) {
            glm::vec2 b1 = poly2[j];
            glm::vec2 b2 = poly2[(j + 1) % poly2.size()];
            if (check(a1, a2, b1, b2))
                return true;
        }
    }

    return false;
}

// Calcule l'intersection de deux segments -> TRUE = intersection trouvée
bool computeSegmentIntersection(const glm::vec2& p1, const glm::vec2& p2,
                                const glm::vec2& q1, const glm::vec2& q2,
                                glm::vec2& intersection)
{
    glm::vec2 r = p2 - p1;
    glm::vec2 s = q2 - q1;
    float denom = r.x * s.y - r.y * s.x;

    if (denom == 0.0f)
        return false; // segments parallèles

    glm::vec2 diff = q1 - p1;
    float t = (diff.x * s.y - diff.y * s.x) / denom;
    float u = (diff.x * r.y - diff.y * r.x) / denom;

    if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
    {
        intersection = p1 + t * r;
        return true;
    }

    return false;
}

// Remplit une forme fermée
void fillClosedCurve(const std::vector<glm::vec2>& points, float r, float g, float b)
{
    glColor4f(r, g, b, 0.3f); // avec un peu de transparence
    glBegin(GL_TRIANGLE_FAN);
    for (const auto& p : points)
        glVertex2f(p.x, p.y);
    glEnd();
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
                std::cout << "Point supprime sur courbe " << currentCurveIndex << " à l’indice " << i << std::endl;
                if (curves.empty())
                {
                    currentCurveIndex = -1;
                }
                break;
            }
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            camera.rotating = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            camera.lastX = (float)xpos;
            camera.lastY = (float)ypos;
        }
        else if (action == GLFW_RELEASE) {
            camera.rotating = false;
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
    if (camera.rotating) {
        camera.processMouseMovement((float)xpos, (float)ypos);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);

    else if (key == GLFW_KEY_KP_3)
    {
        currentMethod = (currentMethod == BezierMethod::DeCasteljau)
            ? BezierMethod::DirectFormula
            : BezierMethod::DeCasteljau;
        if (currentMethod == BezierMethod::DirectFormula) std::cout << "Utilisation de la formule directe" << std::endl;
        else std::cout << "Utilisation de la formule de De Casteljau" << std::endl;
    }

    else if (key == GLFW_KEY_EQUAL)
    {
        p_courbe = std::max(10, p_courbe + 10 );
        std::cout << "pas de la courbe = " << p_courbe << std::endl;
    }

    else if (key == GLFW_KEY_6)
    {
        p_courbe = std::max(10, p_courbe - 10);
        std::cout << "pas de la courbe = " << p_courbe << std::endl;
    }

    else if (key == GLFW_KEY_N)
    {

        curves.emplace_back();
        currentCurveIndex = (int)curves.size() - 1;
        std::cout << "Nouvelle courbe, index = " << currentCurveIndex << std::endl;
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

        if (key == GLFW_KEY_LEFT)
        {
            matrix[2][0] = -0.1f;
            std::cout << "Translation gauche de la courbe " << currentCurveIndex << std::endl;
        }
        if (key == GLFW_KEY_RIGHT)
        {
            matrix[2][0] = 0.1f;
            std::cout << "Translation droite de la courbe " << currentCurveIndex << std::endl;
        }
        if (key == GLFW_KEY_UP)
        {
            matrix[2][1] = 0.1f;
            std::cout << "Translation haut de la courbe " << currentCurveIndex << std::endl;
        }
        if (key == GLFW_KEY_DOWN)
        {
            matrix[2][1] = -0.1f;
            std::cout << "Translation bas de la courbe " << currentCurveIndex << std::endl;
        }

        if (key == GLFW_KEY_R){

            float theta = -glm::radians(10.0f);
            matrix = glm::mat3(
                glm::cos(theta), glm::sin(theta), 0.0f,
                -glm::sin(theta), glm::cos(theta), 0.0f,
                0.0f, 0.0f, 1.0f
                );
            std::cout << "Rotation horaire de la courbe " << currentCurveIndex << std::endl;
        }

        if (key == GLFW_KEY_T){

            float theta = glm::radians(10.0f);
            matrix = glm::mat3(
                glm::cos(theta), glm::sin(theta), 0.0f,
                -glm::sin(theta), glm::cos(theta), 0.0f,
                0.0f, 0.0f, 1.0f
                );
            std::cout << "Rotation antihoraire de la courbe " << currentCurveIndex << std::endl;
        }
        if (key == GLFW_KEY_Z){

            matrix[0][0] = 1.1f;
            matrix[1][1] = 1.1f;
            std::cout << "Scale up de la courbe " << currentCurveIndex << std::endl;
        }
        if (key == GLFW_KEY_S) {

            matrix[0][0] = 0.9f;
            matrix[1][1] = 0.9f;
            std::cout << "Scale down de la courbe " << currentCurveIndex << std::endl;
        }
        if (key == GLFW_KEY_H) {

            matrix[1][0] = 0.3f;
            std::cout << "Scale horizontal de la courbe " << currentCurveIndex << std::endl;
        }
        if (key == GLFW_KEY_V){

            matrix[0][1] = 0.3f;
            std::cout << "Scale vertical de la courbe " << currentCurveIndex << std::endl;
        }
        for (auto& pt : curves[currentCurveIndex].controlPoints)
        {
            glm::vec3 p = glm::vec3(pt, 1.0f);
            glm::vec3 q = matrix * p;
            pt = glm::vec2(q);
        }
        //std::cout << "Transformation de la courbe " << currentCurveIndex << std::endl;

    }
    else if (key == GLFW_KEY_M && currentCurveIndex != -1)
    {
        curves[currentCurveIndex].duplicateLastPoint();
        std::cout << "Duplication du dernier point de la courbe " << currentCurveIndex << std::endl;
    }

    else if (key == GLFW_KEY_I && curves.size() >= 2)
    {
        auto pts1 = generateCurvePoints(curves[0], currentMethod, p_courbe);
        auto pts2 = generateCurvePoints(curves[1], currentMethod, p_courbe);

        auto hull1 = grahamScan(pts1);
        auto hull2 = grahamScan(pts2);

        bool intersect = doPolygonsIntersect(hull1, hull2);
        std::cout << "Intersection : " << (intersect ? "OUI" : "NON") << std::endl;

        if (intersect)
        {
            glm::vec2 intersection;
            for (int i = 0; i < pts1.size() - 1; ++i) {
                glm::vec2 p1 = pts1[i];
                glm::vec2 p2 = pts1[i + 1];
                for (int j = 0; j < pts2.size() - 1; ++j) {
                    glm::vec2 q1 = pts2[j];
                    glm::vec2 q2 = pts2[j + 1];
                    if (computeSegmentIntersection(p1, p2, q1, q2, intersection)) {
                        std::cout << "Point d intersection : (" << intersection.x << ", " << intersection.y << ")\n";
                        goto done; // on s'arrête au 1er point trouvé
                    }
                }
            }
            done:;
        }
    }
    else if (key == GLFW_KEY_C && currentCurveIndex != -1) {
        curves[currentCurveIndex].closeCurveC0();
        std::cout << "Courbe fermée (C0)" << std::endl;
    }
    else if (key == GLFW_KEY_1 && currentCurveIndex != -1) {
        curves[currentCurveIndex].closeCurveC1();
        std::cout << "Courbe fermée (C1)" << std::endl;
    }
    else if (key == GLFW_KEY_2 && currentCurveIndex != -1) {
        curves[currentCurveIndex].closeCurveC2();
        std::cout << "Courbe fermée (C2)" << std::endl;
    }
    else if (key == GLFW_KEY_K && curves.size() >= 2)
    {
        int a = currentCurveIndex;
        int b = (a + 1) % curves.size();
        curves[a].connectC0(curves[b]);
        std::cout << "Raccord C0 entre " << a << " et " << b << std::endl;
    }
    else if (key == GLFW_KEY_L && curves.size() >= 2)
    {
        int a = currentCurveIndex;
        int b = (a + 1) % curves.size();
        curves[a].connectC1(curves[b]);
        std::cout << "Raccord C1 entre " << a << " et " << b << std::endl;
    }
    else if (key == GLFW_KEY_A && curves.size() >= 2)
    {
        int a = currentCurveIndex;
        int b = (a + 1) % curves.size();
        curves[a].connectC2(curves[b]);
        std::cout << "Raccord C2 entre " << a << " et " << b << std::endl;
    }
    else if (key == GLFW_KEY_B)
    {
        curves.clear();
        const int numCurves = 10;

        // Point de départ
        glm::vec2 start = glm::vec2(-0.9f, 0.0f);
        glm::vec2 delta = glm::vec2(0.2f, 0.1f);  // espacement

        // Première courbe manuelle
        BezierCurveData base;
        base.controlPoints.push_back(start);
        base.controlPoints.push_back(start + glm::vec2(0.1f, 0.2f));
        base.controlPoints.push_back(start + glm::vec2(0.2f, -0.2f));
        base.controlPoints.push_back(start + glm::vec2(0.3f, 0.0f));
        curves.push_back(base);

        // Les 9 suivantes raccordées C²
        for (int i = 1; i < numCurves; ++i) {
            BezierCurveData next;
            // Initialiser avec 4 points (à écraser)
            next.controlPoints = {
                glm::vec2(0), glm::vec2(0), glm::vec2(0), glm::vec2(0)
            };
            curves.push_back(next);
            curves[i - 1].connectC2(curves[i]);
        }

        currentCurveIndex = 0;
        std::cout << numCurves << " courbes raccordées C2 créées." << std::endl;
    }

    else if (key == GLFW_KEY_E && currentCurveIndex != -1)
    {
        const auto& curve = curves[currentCurveIndex];
        if (curve.controlPoints.size() >= 2) {
            auto profile2D = generateCurvePoints(curve, currentMethod, p_courbe);
            extrudedMesh = extrudeLinear(profile2D, 1.0f /* hauteur */, 1.0f /* scaleTop */);
            showExtrusion = true;
            std::cout << "Extrusion générée pour la courbe " << currentCurveIndex << std::endl;
        }
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

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    // glm::mat4 view = glm::lookAt(
    //     glm::vec3(0.0f, 0.0f, 3.0f),  // position caméra
    //     glm::vec3(0.0f, 0.0f, 0.0f),  // regarde vers
    //     glm::vec3(0.0f, 1.0f, 0.0f)   // haut
    // );

    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 mvp = projection * view;

    // Transmettre à OpenGL (version immediate mode)
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&projection[0][0]);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&view[0][0]);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ==== Affichage des courbes 2D ====
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(-1, 1, -1, 1, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        for (int i = 0; i < curves.size(); ++i)
        {
            float r = (i == currentCurveIndex) ? 1.0f : 0.6f;
            float g = (i == currentCurveIndex) ? 1.0f : 0.6f;
            float b = (i == currentCurveIndex) ? 0.0f : 0.6f;

            drawLineStrip(curves[i].controlPoints, r * 1.0f, g * 0.2f, b * 0.2f);

            if (curves[i].controlPoints.size() >= 2)
            {
                auto bezierPoints = generateCurvePoints(curves[i], currentMethod, p_courbe);
                if (curves[i].isClosed()) {
                    fillClosedCurve(bezierPoints, r, g, b);
                }
                drawLineStrip(bezierPoints, r, g, b);
            }
        }

        glPopMatrix(); // MODELVIEW
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        // ==== Affichage extrusion 3D ====
        if (showExtrusion) {
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.getViewMatrix();

            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(&projection[0][0]);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(&view[0][0]);

            drawMesh(extrudedMesh);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
