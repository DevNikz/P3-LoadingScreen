// LoadingScene.cpp
#include "LoadingScene.h"
#include <iostream>
#include <cmath>
#include <algorithm>

LoadingScene::LoadingScene(sf::RenderWindow* window) : window(window) {
    const std::string vinylPath = "Media/Textures/pngimg.com - vinyl_PNG18.png";
    if (!vinylTexture.loadFromFile(vinylPath)) {
        std::cerr << "LoadingScene: failed to load vinyl texture: " << vinylPath << '\n';
    }
    else {
        vinylSprite.setTexture(vinylTexture);
        sf::FloatRect b = vinylSprite.getLocalBounds();
        vinylSprite.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
        vinylSprite.setScale(vinylScale, vinylScale);
        vinylRadius = (std::max(b.width, b.height) * vinylScale) / 2.0f;
    }

    const std::string fontPath = "Media/Sansation.ttf";
    if (!font.loadFromFile(fontPath)) {
        std::cerr << "LoadingScene: failed to load font: " << fontPath << '\n';
    }

    loadingText.setFont(font);
    loadingText.setCharacterSize(28);
    loadingText.setFillColor(sf::Color::White);
    loadingText.setString("Loading");

    // Prepare background layers (three images). Each layer will have two sprites used to tile horizontally.
    bgLayers.clear();
    bgLayers.resize(3);
    const std::string bgPaths[3] = {
        "Media/Background/1.png",
        "Media/Background/2.png",
        "Media/Background/3.png"
    };
    // Far to near multipliers: far moves slowest, near moves fastest.
    const float multipliers[3] = { 0.35f, 0.65f, 1.0f };

    for (int i = 0; i < 3; ++i) {
        if (!bgLayers[i].texture.loadFromFile(bgPaths[i])) {
            std::cerr << "LoadingScene: failed to load background: " << bgPaths[i] << '\n';
            // texture remains empty; the sprites will not draw anything.
        }
        else {
            bgLayers[i].sprites[0].setTexture(bgLayers[i].texture);
            bgLayers[i].sprites[1].setTexture(bgLayers[i].texture);
            bgLayers[i].speedMultiplier = multipliers[i];
        }
    }

    ellipsisClock.restart();
    frameClock.restart();
    ellipsisCount = 0;
    angularVelocity = 0.0f;
    draggingVinyl = false;

    if (debugLogEnabled) {
        std::cerr << "LoadingScene: constructed\n";
    }
}

LoadingScene::~LoadingScene() {}

void LoadingScene::start() {
    if (active) {
        if (debugLogEnabled) std::cerr << "LoadingScene: start() called but already active\n";
        return;
    }
    active = true;

    hasLoggedDrawOnce = false;

    sf::Vector2u ws = window->getSize();
    sf::Vector2f center(static_cast<float>(ws.x) / 2.0f, static_cast<float>(ws.y) / 2.0f);
    vinylSprite.setPosition(center);

    if (vinylTexture.getSize().x > 0 && vinylTexture.getSize().y > 0) {
        sf::FloatRect b = vinylSprite.getLocalBounds();
        vinylSprite.setScale(vinylScale, vinylScale);
        vinylRadius = (std::max(b.width, b.height) * vinylScale) / 2.0f;
    }

    // Initialize background sprites positions & scaling to fit vertically.
    for (auto& layer : bgLayers) {
        if (layer.texture.getSize().y == 0) continue;
        // scale to fit window height
        float scaleY = static_cast<float>(ws.y) / static_cast<float>(layer.texture.getSize().y);
        layer.textureHeight = static_cast<float>(layer.texture.getSize().y) * scaleY;
        layer.textureWidth = static_cast<float>(layer.texture.getSize().x) * scaleY;
        for (int s = 0; s < 2; ++s) {
            layer.sprites[s].setScale(scaleY, scaleY);
            // origin top-left
            layer.sprites[s].setPosition(static_cast<float>(s) * layer.textureWidth, 0.0f);
        }
    }

    const float extraTextPadding = 60.0f;

    sf::FloatRect loadRect = loadingText.getLocalBounds();
    loadingText.setOrigin(loadRect.left + loadRect.width / 2.0f, loadRect.top + loadRect.height / 2.0f);
    loadingText.setPosition(center.x, center.y + vinylRadius + extraTextPadding);

    ellipsisClock.restart();
    frameClock.restart();
    dragClock.restart();
    ellipsisCount = 0;
    angularVelocity = basePassiveSpin;
    draggingVinyl = false;

    if (debugLogEnabled) {
        std::cerr << "LoadingScene: start()\n";
    }
}

void LoadingScene::stop() {
    active = false;
    if (debugLogEnabled) {
        std::cerr << "LoadingScene: stop()\n";
    }
}

bool LoadingScene::isActive() const {
    return active;
}

float LoadingScene::mouseAngleDeg(const sf::Vector2f& center, const sf::Vector2f& mousePos) {
    float dx = mousePos.x - center.x;
    float dy = mousePos.y - center.y;
    return toDegrees(std::atan2(dy, dx));
}

void LoadingScene::handleEvent(const sf::Event& event) {
    if (!active) return;

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f m(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
            sf::Vector2f center = vinylSprite.getPosition();
            float dx = m.x - center.x;
            float dy = m.y - center.y;
            if (dx * dx + dy * dy <= vinylRadius * vinylRadius) {
                draggingVinyl = true;
                lastMouseAngleDeg = mouseAngleDeg(center, m);
                dragClock.restart();
                if (debugLogEnabled) {
                    std::cerr << "LoadingScene: draggingVinyl started at (" << m.x << ", " << m.y << ")\n";
                }
            }
        }
    }
    else if (event.type == sf::Event::MouseMoved) {
        if (draggingVinyl) {
            sf::Vector2f m(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
            sf::Vector2f center = vinylSprite.getPosition();
            float angleNow = mouseAngleDeg(center, m);
            float delta = angleNow - lastMouseAngleDeg;
            while (delta > 180.0f) delta -= 360.0f;
            while (delta < -180.0f) delta += 360.0f;

            vinylSprite.setRotation(vinylSprite.getRotation() + delta);

            float dt = dragClock.getElapsedTime().asSeconds();
            if (dt <= 0.0001f) dt = 1.0f / 60.0f;
            angularVelocity = delta / dt;

            const float maxVel = 8000.0f;
            if (angularVelocity > maxVel) angularVelocity = maxVel;
            if (angularVelocity < -maxVel) angularVelocity = -maxVel;

            lastMouseAngleDeg = angleNow;
            dragClock.restart();
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (draggingVinyl && debugLogEnabled) {
                std::cerr << "LoadingScene: draggingVinyl stopped\n";
            }
            draggingVinyl = false;
        }
    }
}

void LoadingScene::draw() {
    if (!active) return;


    if (debugLogEnabled && !hasLoggedDrawOnce) {
        std::cerr << "LoadingScene: draw running (active)\n";
        hasLoggedDrawOnce = true;
    }

    float dt = frameClock.restart().asSeconds();

    vinylSprite.setRotation(vinylSprite.getRotation() + (basePassiveSpin + angularVelocity) * dt);

    angularVelocity *= std::pow(dampingFactor, dt * 60.0f);

    if (ellipsisClock.getElapsedTime().asSeconds() >= ellipsisInterval) {
        ellipsisClock.restart();
        ellipsisCount = (ellipsisCount + 1) % 4;
    }
    std::string dots;
    for (int i = 0; i < ellipsisCount; ++i) dots.push_back('.');
    loadingText.setString(std::string("Loading") + dots);
    sf::FloatRect loadRect = loadingText.getLocalBounds();
    loadingText.setOrigin(loadRect.left + loadRect.width / 2.0f, loadRect.top + loadRect.height / 2.0f);

    sf::Vector2u ws = window->getSize();
    sf::Vector2f center(static_cast<float>(ws.x) / 2.0f, static_cast<float>(ws.y) / 2.0f);
    vinylSprite.setPosition(center);

    // Recompute vertical centering for backgrounds if window size changed
    for (auto& layer : bgLayers) {
        if (layer.texture.getSize().y == 0) continue;
        float wantedScaleY = static_cast<float>(ws.y) / static_cast<float>(layer.texture.getSize().y);
        float computedHeight = static_cast<float>(layer.texture.getSize().y) * wantedScaleY;
        float computedWidth = static_cast<float>(layer.texture.getSize().x) * wantedScaleY;
        // if height/width changed due to window resize, update scale and positions
        if (std::abs(computedWidth - layer.textureWidth) > 0.5f || std::abs(computedHeight - layer.textureHeight) > 0.5f) {
            layer.textureHeight = computedHeight;
            layer.textureWidth = computedWidth;
            for (int s = 0; s < 2; ++s) {
                layer.sprites[s].setScale(wantedScaleY, wantedScaleY);
                // reposition sprites side-by-side (keep current X for smoothness)
                if (s == 0) {
                    // keep current x if already set, otherwise 0
                    if (layer.sprites[s].getPosition().x == 0.0f)
                        layer.sprites[s].setPosition(0.0f, 0.0f);
                }
                else {
                    layer.sprites[s].setPosition(layer.textureWidth, 0.0f);
                }
            }
        }
    }

    const float extraTextPadding = 60.0f;
    loadingText.setPosition(center.x, center.y + vinylRadius + extraTextPadding);

    // Compute global background pixel speed from vinyl rotation speed.
    // Positive (basePassiveSpin + angularVelocity) -> move left; negative -> move right.
    float globalPixelSpeed = (basePassiveSpin + angularVelocity) * bgSpeedFactor;

    // Update & draw background layers first (behind overlay and vinyl).
    for (auto& layer : bgLayers) {
        if (layer.texture.getSize().y == 0 || layer.textureWidth <= 0.0f) continue;

        // movement for this layer
        float move = globalPixelSpeed * layer.speedMultiplier * dt;

        // move both sprites horizontally
        for (int s = 0; s < 2; ++s) {
            layer.sprites[s].move(-move, 0.0f); // negative because positive globalPixelSpeed should move visuals left
        }

        // Tiling/wrapping logic using two sprites:
        // Keep each sprite within the range (-textureWidth, textureWidth)
        for (int s = 0; s < 2; ++s) {
            float x = layer.sprites[s].getPosition().x;
            // If sprite moved completely left out of view by one full texture, wrap it to the right.
            if (x <= -layer.textureWidth) {
                x += layer.textureWidth * 2.0f;
            }
            // If sprite moved too far right (e.g. when reversing), wrap it to the left.
            else if (x >= layer.textureWidth) {
                x -= layer.textureWidth * 2.0f;
            }
            layer.sprites[s].setPosition(x, layer.sprites[s].getPosition().y);
        }

        // Draw the two sprites for this layer
        window->draw(layer.sprites[0]);
        window->draw(layer.sprites[1]);
    }

    // Draw semi-transparent overlay on top of backgrounds
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(static_cast<float>(ws.x), static_cast<float>(ws.y)));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window->draw(overlay);

    // Draw vinyl and text on top
    window->draw(vinylSprite);

    window->draw(loadingText);
}