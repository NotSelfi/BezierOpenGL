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
    bool isClosed(float epsilon = 0.01f) const;
    void closeCurveC0();
    void closeCurveC1();
    void closeCurveC2();
    void connectC0(BezierCurveData& next);
    void connectC1(BezierCurveData& next);
    void connectC2(BezierCurveData& next);

private:
    glm::vec2 deCasteljau(float t) const;
    glm::vec2 evaluateDirect(float t) const;
    int binomialCoefficient(int n, int k) const;
};
