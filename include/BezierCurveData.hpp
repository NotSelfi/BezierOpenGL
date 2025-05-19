#pragma once
#include <vector>
#include <glm/glm.hpp>

enum class BezierMethod {
    DeCasteljau,
    DirectFormula
};

class BezierCurveData {
public:
    std::vector<glm::vec2> controlPoints;

    BezierCurveData() = default;

    glm::vec2 evaluate(float t, BezierMethod method = BezierMethod::DeCasteljau) const;
    void applyTransformation(const glm::mat3& matrix);
    void duplicateLastPoint();


private:
    glm::vec2 deCasteljau(float t) const;
    glm::vec2 evaluateDirect(float t) const;
    int binomialCoefficient(int n, int k) const;
};
