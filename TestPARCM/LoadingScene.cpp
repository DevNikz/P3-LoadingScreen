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

    ellipsisClock.restart();
    frameClock.restart();
    ellipsisCount = 0;
    angularVelocity = 0.0f;
    draggingVinyl = false;
}

LoadingScene::~LoadingScene() {}

void LoadingScene::start() {
    if (active) return;
    active = true;

    sf::Vector2u ws = window->getSize();
    sf::Vector2f center(static_cast<float>(ws.x) / 2.0f, static_cast<float>(ws.y) / 2.0f);
    vinylSprite.setPosition(center);

    if (vinylTexture.getSize().x > 0 && vinylTexture.getSize().y > 0) {
        sf::FloatRect b = vinylSprite.getLocalBounds();
        vinylSprite.setScale(vinylScale, vinylScale);
        vinylRadius = (std::max(b.width, b.height) * vinylScale) / 2.0f;
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
}

void LoadingScene::stop() {
    active = false;
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
            draggingVinyl = false;
        }
    }
}

void LoadingScene::draw() {
    if (!active) return;

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

    const float extraTextPadding = 60.0f;
    loadingText.setPosition(center.x, center.y + vinylRadius + extraTextPadding);

    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(static_cast<float>(ws.x), static_cast<float>(ws.y)));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window->draw(overlay);

    window->draw(vinylSprite);

    window->draw(loadingText);
}