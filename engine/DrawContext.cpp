#include "DrawContext.h"

#include <cmath>

#include "MathUtil.h"

namespace CMPUT350 {

// stores the window and font for drawing, and provides functions to draw text, shapes, and lines on
// the window
DrawContext::DrawContext(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<sf::Font> font)
    : mWindow(window), mFont(font) {}

// rotates point p by mRotation and then adds mOffset to it
Point2D DrawContext::Transform(Point2D p) {
    float c = std::cos(mRotation);
    float s = std::sin(mRotation);
    return {p.x * c - p.y * s + mOffset.x, p.x * s + p.y * c + mOffset.y};
}

// reverse the transform, subtract mOffset and then rotate by -mRotation
Point2D DrawContext::ReverseTransform(Point2D p) {
    float dx = p.x - mOffset.x;
    float dy = p.y - mOffset.y;
    float c = std::cos(mRotation);
    float s = std::sin(mRotation);
    return {dx * c + dy * s, -dx * s + dy * c};
}

// rotates rectangle r by mRotation and then adds mOffset to it, returning the bounding box of the
// rotated rectangle
Rect DrawContext::Transform(Rect r) {
    Point2D c0 = Transform(r.topLeft);
    Point2D c1 = Transform(Point2D(r.topLeft.x + r.width, r.topLeft.y));
    Point2D c2 = Transform(Point2D(r.topLeft.x, r.topLeft.y + r.height));
    Point2D c3 = Transform(Point2D(r.topLeft.x + r.width, r.topLeft.y + r.height));
    Rect box(c0, c1);
    box |= c2;
    box |= c3;
    return box;
}

void DrawContext::SetContextOffset(Point2D p) { mOffset = p; }

void DrawContext::SetContextRotation(float rotation) { mRotation = rotation; }

// draw text cenetered on p
void DrawContext::DrawCenteredText(const std::string& text, int pixelSize, Point2D p, RGBColor c) {
    sf::Text label(*mFont, text, static_cast<unsigned int>(pixelSize));
    label.setFillColor(sf::Color(c.r, c.g, c.b));
    // Set the origin to the center of the text bounds
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                     textBounds.position.y + textBounds.size.y / 2.0f});
    Point2D world = Transform(p);
    label.setPosition({world.x, world.y});
    label.setRotation(sf::radians(mRotation));
    mWindow->draw(label);
}

// draw text with the top-left corner at p
void DrawContext::DrawText(const std::string& text, int pixelSize, Point2D p, RGBColor c) {
    sf::Text label(*mFont, text, static_cast<unsigned int>(pixelSize));
    label.setFillColor(sf::Color(c.r, c.g, c.b));
    // Set the origin to the top-left corner of the text bounds
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setOrigin(textBounds.position);
    Point2D world = Transform(p);
    label.setPosition({world.x, world.y});
    label.setRotation(sf::radians(mRotation));
    mWindow->draw(label);
}

// draw a filled circle centered at p
void DrawContext::DrawCircle(Point2D p, float radius, RGBColor c) {
    Point2D world = Transform(p);
    sf::CircleShape circle(radius);
    circle.setFillColor(sf::Color(c.r, c.g, c.b));
    circle.setOrigin({radius, radius});
    circle.setPosition({world.x, world.y});
    mWindow->draw(circle);
}

// draw a filled rectangle
void DrawContext::DrawRect(Rect r, RGBColor c) {
    Point2D corners[4] = {
        Transform(r.topLeft),
        Transform(Point2D(r.topLeft.x + r.width, r.topLeft.y)),
        Transform(Point2D(r.topLeft.x + r.width, r.topLeft.y + r.height)),
        Transform(Point2D(r.topLeft.x, r.topLeft.y + r.height)),
    };
    sf::ConvexShape polygon(4);
    for (int i = 0; i < 4; i++) polygon.setPoint(i, {corners[i].x, corners[i].y});
    polygon.setFillColor(sf::Color(c.r, c.g, c.b));
    mWindow->draw(polygon);
}

// draw a rectangle outline
void DrawContext::FrameRect(Rect r, float width, RGBColor c) {
    Point2D tl = r.topLeft;
    Point2D tr(r.topLeft.x + r.width, r.topLeft.y);
    Point2D br(r.topLeft.x + r.width, r.topLeft.y + r.height);
    Point2D bl(r.topLeft.x, r.topLeft.y + r.height);
    DrawLine(tl, tr, width, c);
    DrawLine(tr, br, width, c);
    DrawLine(br, bl, width, c);
    DrawLine(bl, tl, width, c);
}

// draw a line from from to to with the given width
void DrawContext::DrawLine(Point2D from, Point2D to, float width, RGBColor c) {
    from = Transform(from);
    to = Transform(to);
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float length = static_cast<float>(from.Distance(to));
    // if length is 0, then the line is a point, so we can just return
    if (length == 0.f) {
        return;
    }
    // get the angle of rotation
    float angle = std::atan2(dy, dx);

    sf::ConvexShape polygon(4);
    float halfWidth = width / 2.f;
    polygon.setPoint(0, {0.f, -halfWidth});
    polygon.setPoint(1, {length, -halfWidth});
    polygon.setPoint(2, {length, halfWidth});
    polygon.setPoint(3, {0.f, halfWidth});
    polygon.setFillColor(sf::Color(c.r, c.g, c.b));
    // set the line to start from middle of the left side of the polygon
    polygon.setPosition({from.x, from.y});
    polygon.setRotation(sf::radians(angle));
    mWindow->draw(polygon);
}

int DrawContext::GetWindowWidth() { return mWindow->getSize().x; }

int DrawContext::GetWindowHeight() { return mWindow->getSize().y; }

}  // namespace CMPUT350
