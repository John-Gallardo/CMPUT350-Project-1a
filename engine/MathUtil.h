#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <algorithm>
#include <cmath>
#include <iostream>

namespace CMPUT350 {

struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
    double Distance(const Point2D& other) const {
        // Distance formula
        float dx = x - other.x;
        float dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    Point2D operator+(const Point2D& other) const {
        // Vector addition
        return {x + other.x, y + other.y};
    }
    Point2D operator+(const float& other) const {
        // Vector addition with a float
        return {x + other, y + other};
    }
    Point2D operator-(const Point2D& other) const {
        // Vector subtraction
        return {x - other.x, y - other.y};
    }
    Point2D operator-(const float& other) const {
        // Vector subtraction with a float
        return {x - other, y - other};
    }
    Point2D operator*(const float& scalar) const {
        // Typical vector and scalar multiplication
        return {x * scalar, y * scalar};
    }
    Point2D& operator+=(const float& scalar) {
        // Vector addition with a float
        x += scalar;
        y += scalar;
        return *this;
    }
    Point2D& operator+=(const Point2D& other) {
        // Vector addition
        x += other.x;
        y += other.y;
        return *this;
    }
    Point2D& operator-=(const Point2D& other) {
        // Vector subtraction
        x -= other.x;
        y -= other.y;
        return *this;
    }
    bool operator==(const Point2D& other) const {
        // Components must be equal
        return x == other.x && y == other.y;
    }
    Point2D& operator*=(const int& scalar) {
        // Vector and scalar multiplication
        x *= scalar;
        y *= scalar;
        return *this;
    }
    Point2D& operator/=(const int& scalar) {
        // Vector and scalar division
        if (scalar == 0) {
            std::cerr << "Warning: scalar division by 0. x & y set to 0";
            x = 0;
            y = 0;
        } else {
            x /= scalar;
            y /= scalar;
        }

        return *this;
    }
    float operator*(const Point2D& other) const {
        // Dot product
        return x * other.x + y * other.y;
        // return Dot(other);
    }
    float Dot(Point2D b) const {
        // Dot product with object & b
        return x * b.x + y * b.y;
    }
    static float Dot(Point2D a, Point2D b) {
        // Dot product
        return a.x * b.x + a.y * b.y;
    }
    static float Cross(Point2D a, Point2D b) {
        // Cross product
        return a.x * b.y - a.y * b.x;
    }
    void Normalize() {
        // Normalize by dividing each component (x, y) by length
        float length = std::sqrt(x * x + y * y);
        if (length == 0) {
            std::cerr << "Warning: length is 0";
        } else {
            x /= length;
            y /= length;
        }
    }
};

static std::ostream& operator<<(std::ostream& os, const Point2D& p) {
    // Format is "(x, y)"
    os << '(' << p.x << ", " << p.y << ')';
    return os;
}

static Point2D operator*(float number, const Point2D& rhs) {
    return {number * rhs.x, number * rhs.y};
}

struct Line {
    Point2D p1, p2;

    Line(Point2D p1 = {0, 0}, Point2D p2 = {0, 0}) : p1(p1), p2(p2) {}
    Line(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}

    float Length() const { return static_cast<float>(p1.Distance(p2)); }

    Point2D ClosestPoint(const Point2D& p) const {
        Point2D d = p2 - p1;
        float lengthSquared = d.Dot(d);
        // If the line segment is a point, return p1
        if (lengthSquared == 0) {
            return p1;
        }
        // projecting the point onto the line and scaling it to the line segment
        float t = (p - p1).Dot(d) / lengthSquared;
        if (t < 0) {
            t = 0;
        } else if (t > 1) {
            t = 1;
        }
        return p1 + d * t;
    }

    bool Crosses(Line other, Point2D& crossingPoint) const {
        Point2D ab = p2 - p1;
        Point2D xy = other.p2 - other.p1;
        float linesCrossPdt = Point2D::Cross(ab, xy);
        // checking for parallel lines, if the cross product is 0, then the lines are parallel and
        // do not intersect
        if (std::fabs(linesCrossPdt) < 1e-8f) {
            return false;
        }
        Point2D qp = other.p1 - p1;

        float t = Point2D::Cross(qp, xy) / linesCrossPdt;
        float u = Point2D::Cross(qp, ab) / linesCrossPdt;
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            crossingPoint = p1 + ab * t;
            return true;
        }
        return false;
    }
};

static std::ostream& operator<<(std::ostream& os, const Line& l) {
    os << l.p1 << " -> " << l.p2;
    return os;
}

struct Circle {
    Point2D center;
    float radius;

    Circle(Point2D c = {0, 0}, float r = 0) : center(c), radius(r) {}

    Circle(float x, float y, float r) : center(x, y), radius(r) {}
};

struct Rect {
    Point2D topLeft;
    float width, height;

    Rect(float left, float top, float width, float height)
        : topLeft(Point2D(left, top)), width(width), height(height) {}

    Rect(Point2D tl = {0, 0}, int w = 0, int h = 0) : topLeft(tl), width(w), height(h) {}

    // Creates bounding box around p1 and p2 with positive width/height
    Rect(Point2D p1, Point2D p2)
        : topLeft(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
          width(fabs(p1.x - p2.x)),
          height(fabs(p1.y - p2.y)) {}

    Rect(Point2D center, float radius)
        : topLeft(center.x - radius, center.y - radius), width(2 * radius), height(2 * radius) {}

    Rect& operator|=(const Rect& other) {
        // expand this rectangle to include the other rectangle
        float left = std::min(topLeft.x, other.topLeft.x);
        float top = std::min(topLeft.y, other.topLeft.y);
        float right = std::max(topLeft.x + width, other.topLeft.x + other.width);
        float bottom = std::max(topLeft.y + height, other.topLeft.y + other.height);
        // Update the current rectangle to the new bounds
        topLeft = Point2D(left, top);
        width = right - left;
        height = bottom - top;
        return *this;
    }
    Rect& operator|=(const Point2D& other) {
        float left = std::min(topLeft.x, other.x);
        float top = std::min(topLeft.y, other.y);
        float right = std::max(topLeft.x + width, other.x);
        float bottom = std::max(topLeft.y + height, other.y);
        // update the AABB to include the point
        topLeft = Point2D(left, top);
        width = right - left;
        height = bottom - top;
        return *this;
    }
    Rect& operator|=(const Line& other) {
        // line is just two points, so we can use the point operator to expand the rectangle to
        // include both endpoints of the line
        *this |= other.p1;
        *this |= other.p2;
        return *this;
    }

    Rect& operator&=(const Rect& other) {
        // shrink this rectangle to the intersection of this rectangle and the other rectangle
        float left = std::max(topLeft.x, other.topLeft.x);
        float top = std::max(topLeft.y, other.topLeft.y);
        float right = std::min(topLeft.x + width, other.topLeft.x + other.width);
        float bottom = std::min(topLeft.y + height, other.topLeft.y + other.height);
        // If the rectangles do not intersect, set width and height to 0
        if (right < left || bottom < top) {
            width = 0;
            height = 0;

        } else {
            topLeft = Point2D(left, top);
            width = right - left;
            height = bottom - top;
        }
        return *this;
    }

    Rect& operator+=(const Point2D& other) {
        // Move the same rectangle by the given point
        topLeft += other;
        return *this;
    }

    Rect operator+(const Point2D& other) const {
        return Rect(topLeft.x + other.x, topLeft.y + other.y, width, height);
    }

    void Inset(int inset) {
        // shrink the rect byt given inset value
        topLeft.x += inset;
        topLeft.y += inset;
        width -= 2 * inset;
        height -= 2 * inset;
        if (width < 0) {
            width = 0;
        }
        if (height < 0) {
            height = 0;
        }
    }

    bool IsInside(const Point2D& p) const {
        // check if the point is inside the Rect
        return p.x >= topLeft.x && p.x <= topLeft.x + width && p.y >= topLeft.y &&
               p.y <= topLeft.y + height;
    }
};

static std::ostream& operator<<(std::ostream& os, const Rect& l) {
    os << '[' << l.topLeft << ' ' << l.width << 'x' << l.height << ']';
    return os;
}

}  // namespace CMPUT350

#endif  // MATHUTIL_H
