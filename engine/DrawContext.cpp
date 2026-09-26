#include "DrawContext.h"

namespace CMPUT350 {

DrawContext::DrawContext(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<sf::Font> font)
    : mWindow(window), mFont(font) {}

void DrawContext::DrawCenteredText(const std::string& text, int pixelSize, Point2D p, RGBColor c) {
    sf::Text label(*mFont, text, static_cast<unsigned int>(pixelSize));
    label.setFillColor(sf::Color(c.r, c.g, c.b));
    // Set the origin to the center of the text bounds
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                     textBounds.position.y + textBounds.size.y / 2.0f});
    label.setPosition({p.x, p.y});
    mWindow->draw(label);
}

void DrawContext::DrawText(const std::string& text, int pixelSize, Point2D p, RGBColor c) {
    sf::Text label(*mFont, text, static_cast<unsigned int>(pixelSize));
    label.setFillColor(sf::Color(c.r, c.g, c.b));
    // Set the origin to the top-left corner of the text bounds
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setOrigin(textBounds.position);
    label.setPosition({p.x, p.y});
    mWindow->draw(label);
}

void DrawContext::DrawCircle(Point2D p, float radius, RGBColor c) {
    sf::CircleShape circle(radius);
    circle.setFillColor(sf::Color(c.r, c.g, c.b));
    circle.setOrigin({radius, radius});
    circle.setPosition({p.x, p.y});
    mWindow->draw(circle);
}

void DrawContext::DrawRect(Rect r, RGBColor c) {
    sf::RectangleShape rectangle({r.width, r.height});
    rectangle.setFillColor(sf::Color(c.r, c.g, c.b));
    rectangle.setPosition({r.topLeft.x, r.topLeft.y});
    mWindow->draw(rectangle);
}

void DrawContext::FrameRect(Rect r, float width, RGBColor c) {
    sf::RectangleShape frameBox({r.width, r.height});
    // first fill the rectangle with transparent color, then set the outline color and thickness
    frameBox.setFillColor(sf::Color::Transparent);
    frameBox.setOutlineColor(sf::Color(c.r, c.g, c.b));
    // width is negative to draw the outline inside the rectangle
    frameBox.setOutlineThickness(-width);
    frameBox.setPosition({r.topLeft.x, r.topLeft.y});
    mWindow->draw(frameBox);
}

/**
 * @brief Draws a line between two points with a specified width and color.
 *
 * @param from The starting point of the line (Point2D).
 * @param to The ending point of the line (Point2D).
 * @param width The width of the line in pixels.
 * @param c The color of the line, specified as an RGBColor object.
 *
 * This function calculates the distance and angle between the two points
 * and uses a polygone shape to represent the line. The line is drawn
 * relative to the world offset and rendered onto the associated window.
 */
void DrawContext::DrawLine(Point2D from, Point2D to, float width, RGBColor c) {
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
