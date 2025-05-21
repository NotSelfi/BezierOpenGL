#include "BezierCurveData.hpp"
#include <cmath>

glm::vec2 BezierCurveData::evaluate(float t, BezierMethod method) const {
    return (method == BezierMethod::DeCasteljau) ?
        deCasteljau(t) :
        evaluateDirect(t);
}

glm::vec2 BezierCurveData::deCasteljau(float t) const {
    std::vector<glm::vec2> temp = controlPoints;
    while (temp.size() > 1) {
        std::vector<glm::vec2> next;
        for (size_t i = 0; i < temp.size() - 1; ++i) {
            glm::vec2 p = (1 - t) * temp[i] + t * temp[i + 1];
            next.push_back(p);
        }
        temp = next;
    }
    return temp[0];
}


/// B(t) = Σ_{i=0}^{n} C(n, i) * (1 - t)^{n - i} * t^i * P_i
glm::vec2 BezierCurveData::evaluateDirect(float t) const {
    int n = static_cast<int>(controlPoints.size()) - 1;
    glm::vec2 result(0.0f);

    for (int i = 0; i <= n; ++i) {
        int binCoeff = binomialCoefficient(n, i);
        float term = binCoeff * std::pow(1 - t, n - i) * std::pow(t, i);
        result += term * controlPoints[i];
    }

    return result;
}

int BezierCurveData::binomialCoefficient(int n, int k) const {
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    int res = 1;
    for (int i = 1; i <= k; ++i) {
        res = res * (n - i + 1) / i;
    }
    return res;
}

void BezierCurveData::applyTransformation(const glm::mat3& matrix) {
    for (auto& point : controlPoints) {
        glm::vec3 p(point, 1.0f);
        p = matrix * p;
        point = glm::vec2(p.x, p.y);
    }
}

void BezierCurveData::duplicateLastPoint() {
    if (!controlPoints.empty()) {
        controlPoints.push_back(controlPoints.back());
    }
}

bool BezierCurveData::isClosed(float epsilon) const {
    if (controlPoints.size() < 3) return false;
    return glm::distance(controlPoints.front(), controlPoints.back()) < epsilon;
}

void BezierCurveData::closeCurveC0() {
    if (controlPoints.empty()) return;
    controlPoints.push_back(controlPoints.front());
}

void BezierCurveData::closeCurveC1() {
    if (controlPoints.size() < 2) return;
    glm::vec2 last = controlPoints.back();
    glm::vec2 beforeLast = controlPoints[controlPoints.size() - 2];
    glm::vec2 dir = glm::normalize(last - beforeLast);
    glm::vec2 penultimate = controlPoints[1];
    glm::vec2 newStart = controlPoints.front();
    glm::vec2 reflected = newStart + dir * glm::distance(newStart, penultimate);
    controlPoints.push_back(reflected);
    controlPoints.push_back(newStart);
}

void BezierCurveData::closeCurveC2() {
    if (controlPoints.size() < 3) return;
    glm::vec2 p0 = controlPoints[controlPoints.size() - 3];
    glm::vec2 p1 = controlPoints[controlPoints.size() - 2];
    glm::vec2 p2 = controlPoints.back();

    glm::vec2 accel = p2 - 2.0f * p1 + p0;
    glm::vec2 newP1 = controlPoints[1];
    glm::vec2 newP2 = controlPoints[0];

    glm::vec2 mirrored = 2.0f * newP1 - newP2 + accel;
    controlPoints.push_back(mirrored);
    controlPoints.push_back(newP1);
    controlPoints.push_back(newP2);
}

void BezierCurveData::connectC0(BezierCurveData& next) {
    if (controlPoints.empty()) return;
    next.controlPoints.front() = controlPoints.back();
}

void BezierCurveData::connectC1(BezierCurveData& next) {
    if (controlPoints.size() < 2 || next.controlPoints.size() < 2) return;

    // Assure la continuité C0
    next.controlPoints.front() = controlPoints.back();

    // Alignement des tangentes
    glm::vec2 last = controlPoints.back();
    glm::vec2 prev = controlPoints[controlPoints.size() - 2];
    glm::vec2 dir = glm::normalize(last - prev);

    float len = glm::distance(next.controlPoints[0], next.controlPoints[1]);
    next.controlPoints[1] = next.controlPoints[0] + dir * len;
}

void BezierCurveData::connectC2(BezierCurveData& next) {
    if (controlPoints.size() < 3 || next.controlPoints.size() < 3) return;

    // C0
    next.controlPoints.front() = controlPoints.back();

    // C1
    glm::vec2 d1 = glm::normalize(controlPoints.back() - controlPoints[controlPoints.size() - 2]);
    float len1 = glm::distance(next.controlPoints[0], next.controlPoints[1]);
    next.controlPoints[1] = next.controlPoints[0] + d1 * len1;

    // C2 (accélération)
    glm::vec2 a1 = controlPoints[controlPoints.size() - 3];
    glm::vec2 a2 = controlPoints[controlPoints.size() - 2];
    glm::vec2 a3 = controlPoints.back();

    glm::vec2 acc = a3 - 2.0f * a2 + a1;

    glm::vec2 b2 = next.controlPoints[1];
    next.controlPoints[2] = 2.0f * b2 - next.controlPoints[0] + acc;
}


