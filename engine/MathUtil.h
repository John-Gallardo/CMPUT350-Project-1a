#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <cmath>
#include <iostream>

namespace CMPUT350 {

struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
    double Distance(const Point2D &other) const {
        // Distance formula
        return std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
    }
    Point2D operator+(const Point2D &other) const {
        // Vector addition
        return {x + other.x, y + other.y};
    }
    Point2D operator+(const float &other) const {
        // Vector addition with a float
        return {x + other, y + other};
    }
    Point2D operator-(const Point2D &other) const {
        // Vector subtraction
        return {x - other.x, y - other.y};
    }
    Point2D operator-(const float &other) const {
        // Vector subtraction with a float
        return {x - other, y - other};
    }
    Point2D operator*(const float &scalar) const {
        // Typical vector and scalar multiplication
        return {x * scalar, y * scalar};
    }
    Point2D &operator+=(const float &scalar) {
        // Vector addition with a float
        x += scalar;
        y += scalar;
        return *this;
    }
    Point2D &operator+=(const Point2D &other) {
        // Vector addition
        x += other.x;
        y += other.y;
        return *this;
    }
    Point2D &operator-=(const Point2D &other) {
        // Vector subtraction
        x -= other.x;
        y -= other.y;
        return *this;
    }
    bool operator==(const Point2D &other) const {
        // Components must be equal
        return x == other.x && y == other.y;
    }
    Point2D &operator*=(const int &scalar) {
        // Vector and scalar multiplication
        x *= scalar;
        y *= scalar;
        return *this;
    }
    Point2D &operator/=(const int &scalar) {
        // Vector and scalar division
        // NOTE: are we supposed to check for a division by 0?
        x /= scalar;
        y /= scalar;
        return *this;
    }
    float operator*(const Point2D &other) const {
        // NOTE: I assume this is a dot product since it returns a float
        return x * other.x + y * other.y;
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
        float length = std::sqrt(x*x + y*y);
        x /= length;
        y /= length;
    }
};

static std::ostream &operator<<(std::ostream &os, const Point2D &p) {
    // TODO: write this code
    return os;
}

static Point2D operator*(float number, const Point2D &rhs) {
    // TODO: write this code
    return rhs;
}

struct Line {
    Point2D p1, p2;

    Line(Point2D p1 = {0, 0}, Point2D p2 = {0, 0}) : p1(p1), p2(p2) {}
    Line(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}
    float Length() const {
        // TODO: write this code
        return 0;
    }
    Point2D ClosestPoint(const Point2D &p) const {
        // TODO: write this code
        return p;
    }
    bool Crosses(Line other, Point2D &crossingPoint) const {
        // TODO: write this code
        return false;
    }
};

static std::ostream &operator<<(std::ostream &os, const Line &l) {
    // TODO: write this code
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
        : topLeft(Point2D(top, left)), width(width), height(height) {}

    Rect(Point2D tl = {0, 0}, int w = 0, int h = 0) : topLeft(tl), width(w), height(h) {}

    // Creates bounding box around p1 and p2 with positive width/height
    Rect(Point2D p1, Point2D p2)
        : topLeft(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
          width(fabs(p1.x - p2.x)),
          height(fabs(p1.y - p2.y)) {}

    Rect(Point2D center, float radius)
        : topLeft(center.x - radius, center.y - radius), width(2 * radius), height(2 * radius) {}

    Rect &operator|=(const Rect &other) {
        // TODO: write this code
        return *this;
    }
    Rect &operator|=(const Point2D &other) {
        // TODO: write this code
        return *this;
    }
    Rect &operator|=(const Line &other) {
        // TODO: write this code
        return *this;
    }
    Rect &operator&=(const Rect &other) {
        // TODO: write this code
        return *this;
    }
    Rect &operator+=(const Point2D &other) {
        // TODO: write this code
        return *this;
    }
    Rect operator+(const Point2D &other) const {
        // TODO: write this code
        return *this;
    }
    void Inset(int inset) {
        // TODO: write this code
    }
    bool IsInside(const Point2D &p) const {
        // TODO: write this code
        return false;
    }
};

static std::ostream &operator<<(std::ostream &os, const Rect &l) {
    // TODO: write this code
    return os;
}

}  // namespace CMPUT350

#endif  // MATHUTIL_H
