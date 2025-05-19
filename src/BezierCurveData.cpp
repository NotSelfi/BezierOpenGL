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


