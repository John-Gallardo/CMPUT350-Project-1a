#include "Bezier.h"

#include <algorithm>
#include <vector>

#include "MathUtil.h"

namespace CMPUT350 {

namespace {

// Find the segment of the Bezier curve that corresponds to the parameter t
bool FindSegment(const std::vector<Point2D>& pts, float t, size_t& first, float& u) {
    if (pts.size() < 4) return false;
    int segments = static_cast<int>((pts.size() - 1) / 3);
    t = std::clamp(t, 0.0f, static_cast<float>(segments));
    int seg = std::min(static_cast<int>(t), segments - 1);  // t == k belongs to the last segment
    first = static_cast<size_t>(seg) * 3;
    u = t - static_cast<float>(seg);
    return true;
}
}  // namespace

Bezier::Bezier(const std::vector<Point2D>& pts) : mPoints(pts) {}

Point2D Bezier::GetPoint(float t) const { return GetPoint(mPoints, t); }

Point2D Bezier::GetSlope(float t) const { return GetSlope(mPoints, t); }

Point2D Bezier::GetPoint(const std::vector<Point2D>& pts, float t) {
    size_t i = 0;
    float u = 0;
    if (!FindSegment(pts, t, i, u)) return pts.empty() ? Point2D{} : pts.front();
    float v = 1.0f - u;
    return pts[i] * (v * v * v) + pts[i + 1] * (3 * v * v * u) + pts[i + 2] * (3 * v * u * u) +
           pts[i + 3] * (u * u * u);
}

Point2D Bezier::GetSlope(const std::vector<Point2D>& pts, float t) {
    size_t i = 0;
    float u = 0;
    if (!FindSegment(pts, t, i, u)) return {};
    float v = 1.0f - u;
    return (pts[i + 1] - pts[i]) * (3 * v * v) + (pts[i + 2] - pts[i + 1]) * (6 * v * u) +
           (pts[i + 3] - pts[i + 2]) * (3 * u * u);
}

}  // namespace CMPUT350
