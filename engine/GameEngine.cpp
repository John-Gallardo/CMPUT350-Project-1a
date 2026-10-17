#include "GameEngine.h"

#include <iostream>
#include <memory>
#include <vector>

#include "CollisionObject.h"
#include "DrawContext.h"
#include <SFML/Graphics.hpp>

/// @brief
namespace CMPUT350 {
#include "FontData.h"

namespace {

// Pose is a position and rotation. The rotation is in radians, counterclockwise, with 0 along the
// +x axis
struct Pose {
    Point2D origin;
    float rotation;
};

// Transform a point from local space to world space, given the pose of the local space
Point2D ToWorld(Point2D p, Pose pose) {
    float c = std::cos(pose.rotation);
    float s = std::sin(pose.rotation);
    return {p.x * c - p.y * s + pose.origin.x, p.x * s + p.y * c + pose.origin.y};
}
// Undo ToWorld so a hit point can be handed back in the object's local space
Point2D ToLocal(Point2D p, Pose pose) {
    float dx = p.x - pose.origin.x;
    float dy = p.y - pose.origin.y;
    float c = std::cos(pose.rotation);
    float s = std::sin(pose.rotation);
    return {dx * c + dy * s, -dx * s + dy * c};
}
// Convert a rectangle from local space to world space
Rect BoundsToWorld(const Rect& local, Pose pose) {
    Point2D c0 = ToWorld(local.topLeft, pose);
    Point2D c1 = ToWorld(Point2D(local.topLeft.x + local.width, local.topLeft.y), pose);
    Point2D c2 = ToWorld(Point2D(local.topLeft.x, local.topLeft.y + local.height), pose);
    Point2D c3 =
        ToWorld(Point2D(local.topLeft.x + local.width, local.topLeft.y + local.height), pose);
    Rect box(c0, c1);
    box |= c2;
    box |= c3;
    return box;
}
// returns true if the two rectangles are separated, false if they overlap or touch
bool BoxesSeparated(const Rect& a, const Rect& b) {
    return a.topLeft.x + a.width < b.topLeft.x || a.topLeft.x > b.topLeft.x + b.width ||
           a.topLeft.y + a.height < b.topLeft.y || a.topLeft.y > b.topLeft.y + b.height;
}

// line and circle collision detection
bool LineHitsCircle(const Line& line, const Circle& circle, float& tHit, Point2D& hit) {
    Point2D d = line.p2 - line.p1;
    Point2D f = line.p1 - circle.center;
    float radius2 = circle.radius * circle.radius;
    if (f.Dot(f) <= radius2) {
        tHit = 0.f;
        hit = line.p1;
        return true;
    }
    float a = d.Dot(d);
    if (a < 1e-8f) return false;
    float b = 2.f * f.Dot(d);
    float c = f.Dot(f) - radius2;
    float disc = b * b - 4.f * a * c;
    if (disc < 0.f) return false;
    float root = std::sqrt(disc);
    float t1 = (-b - root) / (2.f * a);
    float t2 = (-b + root) / (2.f * a);
    float t = 2.f;
    if (t1 >= 0.f && t1 <= 1.f) t = t1;
    if (t2 >= 0.f && t2 <= 1.f && t2 < t) t = t2;
    if (t > 1.f) return false;
    tHit = t;
    hit = line.p1 + d * t;
    return true;
}

// Line and rect collision detection. Returns true if the line hits the rect
bool LineHitsRect(const Line& line, const Rect& rect, float& tHit, Point2D& hit) {
    if (rect.IsInside(line.p1)) {
        tHit = 0.f;
        hit = line.p1;
        return true;
    }
    Point2D d = line.p2 - line.p1;
    float tMin = 0.f;
    float tMax = 1.f;
    float start[2] = {line.p1.x, line.p1.y};
    float delta[2] = {d.x, d.y};
    float low[2] = {rect.topLeft.x, rect.topLeft.y};
    float high[2] = {rect.topLeft.x + rect.width, rect.topLeft.y + rect.height};
    for (int axis = 0; axis < 2; axis++) {
        if (std::fabs(delta[axis]) < 1e-8f) {
            if (start[axis] < low[axis] || start[axis] > high[axis]) return false;
            continue;
        }
        float t1 = (low[axis] - start[axis]) / delta[axis];
        float t2 = (high[axis] - start[axis]) / delta[axis];
        if (t1 > t2) std::swap(t1, t2);
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) return false;
    }
    tHit = tMin;
    hit = line.p1 + d * tHit;
    return true;
}

// Returns true if the circle and rect overlap or touch
bool CircleHitsRect(const Circle& circle, const Rect& rect) {
    float x = std::clamp(circle.center.x, rect.topLeft.x, rect.topLeft.x + rect.width);
    float y = std::clamp(circle.center.y, rect.topLeft.y, rect.topLeft.y + rect.height);
    float dx = circle.center.x - x;
    float dy = circle.center.y - y;
    return dx * dx + dy * dy <= circle.radius * circle.radius;
}

// A contact point is a hit, and if the line is involved, the first point along the line that hits
struct Contact {
    bool hit{false};
    bool hasPoint{false};  // only lines have one contact point
    float t{0.f};
    Point2D world{};
};

// One shape from A against one shape from B.
// Circles and rects only say hit/miss. A line also returns the first point along its direction that
// hits the other shape, if any
Contact TestShapes(const Shape& a, Pose poseA, const Shape& b, Pose poseB) {
    Contact contact;
    if (a.t == ShapeType::kLine || b.t == ShapeType::kLine) {
        const Shape& lineShape = a.t == ShapeType::kLine ? a : b;
        Pose linePose = a.t == ShapeType::kLine ? poseA : poseB;
        const Shape& other = a.t == ShapeType::kLine ? b : a;
        Pose otherPose = a.t == ShapeType::kLine ? poseB : poseA;
        Line world(ToWorld(lineShape.shape.line.p1, linePose),
                   ToWorld(lineShape.shape.line.p2, linePose));
        float t = 0.f;
        Point2D worldHit;
        bool ok = false;
        if (other.t == ShapeType::kLine) {
            Line otherWorld(ToWorld(other.shape.line.p1, otherPose),
                            ToWorld(other.shape.line.p2, otherPose));
            ok = world.Crosses(otherWorld, worldHit);
            Point2D d = world.p2 - world.p1;
            float len2 = d.Dot(d);
            t = len2 < 1e-8f ? 0.f : (worldHit - world.p1).Dot(d) / len2;
        } else if (other.t == ShapeType::kCircle) {
            Circle circle(ToWorld(other.shape.circle.center, otherPose), other.shape.circle.radius);
            ok = LineHitsCircle(world, circle, t, worldHit);
        } else {
            // Move the line into the rect's local space. t is unchanged by that move.
            Line local(ToLocal(world.p1, otherPose), ToLocal(world.p2, otherPose));
            Point2D localHit;
            ok = LineHitsRect(local, other.shape.rect, t, localHit);
            if (ok) worldHit = ToWorld(localHit, otherPose);
        }
        if (!ok) return contact;
        contact.hit = true;
        contact.hasPoint = true;
        contact.t = t;
        contact.world = worldHit;
        return contact;
    }
    if (a.t == ShapeType::kCircle && b.t == ShapeType::kCircle) {
        Circle ca(ToWorld(a.shape.circle.center, poseA), a.shape.circle.radius);
        Circle cb(ToWorld(b.shape.circle.center, poseB), b.shape.circle.radius);
        contact.hit = ca.center.Distance(cb.center) <= ca.radius + cb.radius;
        return contact;
    }
    if (a.t == ShapeType::kCircle && b.t == ShapeType::kRect) {
        Circle local(ToLocal(ToWorld(a.shape.circle.center, poseA), poseB), a.shape.circle.radius);
        contact.hit = CircleHitsRect(local, b.shape.rect);
        return contact;
    }
    if (b.t == ShapeType::kCircle && a.t == ShapeType::kRect) {
        Circle local(ToLocal(ToWorld(b.shape.circle.center, poseB), poseA), b.shape.circle.radius);
        contact.hit = CircleHitsRect(local, a.shape.rect);
        return contact;
    }
    // Two rects. A corner inside the other rect, or an edge crossing, counts as a hit.
    Rect ar = a.shape.rect;
    Rect br = b.shape.rect;
    Point2D aLocal[4] = {
        ar.topLeft,
        Point2D(ar.topLeft.x + ar.width, ar.topLeft.y),
        Point2D(ar.topLeft.x + ar.width, ar.topLeft.y + ar.height),
        Point2D(ar.topLeft.x, ar.topLeft.y + ar.height),
    };
    Point2D bLocal[4] = {
        br.topLeft,
        Point2D(br.topLeft.x + br.width, br.topLeft.y),
        Point2D(br.topLeft.x + br.width, br.topLeft.y + br.height),
        Point2D(br.topLeft.x, br.topLeft.y + br.height),
    };
    Point2D aWorld[4];
    Point2D bWorld[4];
    for (int i = 0; i < 4; i++) {
        aWorld[i] = ToWorld(aLocal[i], poseA);
        bWorld[i] = ToWorld(bLocal[i], poseB);
        if (br.IsInside(ToLocal(aWorld[i], poseB)) || ar.IsInside(ToLocal(bWorld[i], poseA))) {
            contact.hit = true;
            return contact;
        }
    }
    for (int i = 0; i < 4; i++) {
        Line edgeA(aWorld[i], aWorld[(i + 1) % 4]);
        for (int j = 0; j < 4; j++) {
            Point2D cross;
            if (edgeA.Crosses(Line(bWorld[j], bWorld[(j + 1) % 4]), cross)) {
                contact.hit = true;
                return contact;
            }
        }
    }
    return contact;
}
}  // namespace

GameEngine::GameEngine(unsigned int width, unsigned int height, const std::string& name)
    : mWindow{std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), name)} {
    mWindow->setFramerateLimit(30);
    mFont = std::make_shared<sf::Font>();
    if (!mFont->openFromMemory(_font, _font_len)) {
        std::cerr << "WARNING: Font did not load.\n";
    }

    // NOTE: we need the font for these two so they're initialized here
    mScreenContext = DrawContext(mWindow, mFont);
    mGUIContext = DrawContext(mWindow, mFont);
    mGameContext.mEngineView = this;
    mGameContext.ScreenContext = &mScreenContext;
    mGameContext.GUIContext = &mGUIContext;
    mGameContext.mNotificationManager = &mNotificationManager;
    mGameContext.currentFrame = 0;
}

GameEngine::~GameEngine() {
    // Cleanup resources
    mWindow->close();
}

/**
 * @brief Adds a game object to a temporary list, mCreatedObjects, to be added to the next frame
 * @param gameObject The game object we want to add in the next frame.
 */
void GameEngine::AddGameObject(std::shared_ptr<GameObject> gameObject) {
    mCreatedObjects.push_back(gameObject);
}

/**
 * @method Run
 * @arguments None
 * @description Gives control to the game engine. Will not return until the game window is closed or
 * all objects have been destroyed.
 */
void GameEngine::Run() {
    while (true)  // window is open
    {
        // 0. Remove any objects that are now dead
        // Reference: https://en.cppreference.com/cpp/container/vector/erase2
        auto isDead = [](const auto& gameObject) { return !gameObject->IsAlive(); };
        std::erase_if(mGameObjects, isDead);

        // 1. Activate and initialize any objects added during the last frame
        // NOTE: use another createdObjects vector because game objects can call AddGameObject
        // causing iterator invalidation if we went through mCreatedObjects
        auto createdObjects{mCreatedObjects};
        mCreatedObjects.clear();
        for (const auto& gameObject : createdObjects) {
            // NOTE: give a weak ptr since the object can't build a weak_ptr to itself for objects
            // it creates! it only has 'this' which is a raw ptr
            mGameContext.CurrObject = gameObject;
            gameObject->Initialize(&mGameContext);
            mGameObjects.push_back(gameObject);
        }

        // 2. Process events
        // Reference: CMPUT350 Lab 2 Exercise's processInputs()
        for (auto event{mWindow->pollEvent()}; event.has_value(); event = mWindow->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                mWindow->close();
                return;
            }

            // NOTE: Switched to key press instead of text entered because of GameEngine.h/.cpp spec
            // 1. process pressed event
            const auto* pressed{event->getIf<sf::Event::KeyPressed>()};
            if (pressed) {
                const int scancode{static_cast<int>(pressed->scancode)};
                // send notifications ONLY if key is one of the keys we want to send notifications
                // for in keyPressed
                if (mKeyDownNotifications.contains(scancode)) {
                    const auto& notifications{mKeyDownNotifications[scancode]};
                    // loop through all associated notifications with this key press
                    for (const auto& notification : notifications) {
                        mNotificationManager.Notify(notification);
                    }
                }
            }

            // 2. process released event
            const auto* released{event->getIf<sf::Event::KeyReleased>()};
            if (released) {
                const int scancode{static_cast<int>(released->scancode)};
                // again, send notifications ONLY for same idea as above
                if (mKeyUpNotifications.contains(scancode)) {
                    const auto& notifications{mKeyUpNotifications[scancode]};
                    // loop through all associated notifications with this key release event
                    for (const auto& notification : notifications) {
                        mNotificationManager.Notify(notification);
                    }
                }
            }
        }

        // 3. Update game objects
        for (const auto& gameObject : mGameObjects) {
            gameObject->Update(&mGameContext);
        }

        // 4. Process collision events
        std::vector<std::shared_ptr<CollisionObject>> collisionObjects;
        for (const auto& gameObject : mGameObjects) {
            if (auto collisionObject = std::dynamic_pointer_cast<CollisionObject>(gameObject)) {
                if (collisionObject->IsAlive()) collisionObjects.push_back(collisionObject);
            }
        }

        for (size_t i = 0; i < collisionObjects.size(); i++) {
            auto a = collisionObjects[i];
            if (!a->IsAlive()) continue;
            // Get the world bounds of a and b, and if they don't overlap, skip the expensive shape
            // tests
            Pose poseA{a->GetLocation(), a->GetRotation()};
            Rect worldA = BoundsToWorld(a->GetBounds(), poseA);

            for (size_t j = i + 1; j < collisionObjects.size(); j++) {
                auto b = collisionObjects[j];
                if (!a->IsAlive() || !b->IsAlive()) continue;

                Pose poseB{b->GetLocation(), b->GetRotation()};
                Rect worldB = BoundsToWorld(b->GetBounds(), poseB);
                if (BoxesSeparated(worldA, worldB)) continue;

                const auto& shapesA = a->GetShapes();
                const auto& shapesB = b->GetShapes();

                // If either object has no shapes, we consider them to be touching and call
                // CollisionEnter without a point
                if (shapesA.empty() || shapesB.empty()) {
                    a->CollisionEnter(b);
                    if (b->IsAlive()) b->CollisionEnter(a);
                    continue;
                }

                bool touching = false;
                bool hasPoint = false;
                float bestT = 2.f;
                Point2D bestWorld;
                for (const Shape& shapeA : shapesA) {
                    for (const Shape& shapeB : shapesB) {
                        Contact contact = TestShapes(shapeA, poseA, shapeB, poseB);
                        if (!contact.hit) continue;
                        touching = true;
                        if (contact.hasPoint && contact.t < bestT) {
                            hasPoint = true;
                            bestT = contact.t;
                            bestWorld = contact.world;
                        }
                    }
                }
                if (!touching) continue;

                // A line produces one point, converted back into each object's local space.
                // Circles and rects use the no-point CollisionEnter
                if (hasPoint) {
                    a->CollisionEnter(b, ToLocal(bestWorld, poseA));
                    if (b->IsAlive()) b->CollisionEnter(a, ToLocal(bestWorld, poseB));
                } else {
                    a->CollisionEnter(b);
                    if (b->IsAlive()) b->CollisionEnter(a);
                }
            }
        }

        // 5. Late updates
        for (const auto& gameObject : mGameObjects) {
            gameObject->LateUpdate(&mGameContext);
        }

        // Clear window
        mWindow->clear();

        // grab all graphics objects for rendering
        std::vector<std::shared_ptr<GraphicsObject>> graphicsObjects{};
        for (const auto& gameObject : mGameObjects) {
            // Check if is a graphics object
            std::shared_ptr<GraphicsObject> graphicsObject{
                std::dynamic_pointer_cast<GraphicsObject>(gameObject)};
            if (graphicsObject == nullptr) continue;
            graphicsObjects.push_back(graphicsObject);
        }

        // 6. Render background
        for (const auto& graphicsObject : graphicsObjects) {
            mScreenContext.SetContextOffset(graphicsObject->GetLocation());
            mScreenContext.SetContextRotation(graphicsObject->GetRotation());
            graphicsObject->RenderBackground(&mGameContext);
        }

        // 7. Render foreground
        for (const auto& graphicsObject : graphicsObjects) {
            mScreenContext.SetContextOffset(graphicsObject->GetLocation());
            mScreenContext.SetContextRotation(graphicsObject->GetRotation());
            graphicsObject->RenderForeground(&mGameContext);
        }

        // 8. HUD and UI rendering. Reset context to default for these
        mScreenContext.SetContextOffset(Point2D(0.f, 0.f));
        mScreenContext.SetContextRotation(0.f);
        for (const auto& gameObject : mGameObjects) {
            gameObject->RenderUI(&mGameContext);
        }

        // Actually render to window
        mWindow->display();

        // Move onto next frame by incrementing currentFrame
        mGameContext.currentFrame++;
    }
}

/**
 * @brief Add string notification associated to this particular key press for key down
 */
void GameEngine::InstallKeyDownNotification(int key, const std::string& notification) {
    mKeyDownNotifications[key].push_back(notification);
}

/**
 * @brief Add string notification associated to this particular key press for key up
 */
void GameEngine::InstallKeyUpNotification(int key, const std::string& notification) {
    mKeyUpNotifications[key].push_back(notification);
}

/**
 * @brief Returns mGameObject's cbegin iterator
 */
EngineView::const_iterator GameEngine::cbegin() const { return mGameObjects.cbegin(); }

/**
 * @brief Returns mGameObject's cend iterator
 */
EngineView::const_iterator GameEngine::cend() const { return mGameObjects.cend(); }

/**
 * @brief Returns mGameObject's begin iterator
 */
EngineView::const_iterator GameEngine::begin() const { return mGameObjects.begin(); }

/**
 * @brief Returns mGameObject's end iterator
 */
EngineView::const_iterator GameEngine::end() const { return mGameObjects.end(); }

// Sample code for processing events

// bool GameEngine::ProcessEvents(GameContext *context)
//{
//	while (const std::optional event = mWindow->pollEvent())
//	{
//		if (event->is<sf::Event::Closed>())
//		{
//		}
//		else if (event->is<sf::Event::Resized>())
//		{
//		}
//		else if (const auto* keyPressed = event->getIf<sf::Event::TextEntered>())
//		{
//			// use keyPressed->unicode to get character
//		}
//	}
// }

}  // namespace CMPUT350
