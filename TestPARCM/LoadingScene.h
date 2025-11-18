// LoadingScene.h
#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class LoadingScene {
public:
    LoadingScene(sf::RenderWindow* window);
    ~LoadingScene();

    void start();
    void stop();
    bool isActive() const;

    void handleEvent(const sf::Event& event);
    void draw();

private:
    sf::RenderWindow* window;
    bool active = false;


    sf::Texture vinylTexture;
    sf::Sprite vinylSprite;
    float vinylRadius = 120.0f;
    float vinylScale = 0.4f;             
    float basePassiveSpin = 20.0f;
    float angularVelocity = 0.0f;
    bool draggingVinyl = false;
    float lastMouseAngleDeg = 0.0f;


    sf::Clock dragClock;
    sf::Clock frameClock;

    sf::Font font;
    sf::Text loadingText;
    sf::Clock ellipsisClock;
    int ellipsisCount = 0;
    float ellipsisInterval = 0.5f;

    float dampingFactor = 0.995f;

    static float toDegrees(float radians) { return radians * 180.0f / 3.14159265358979323846f; }
    static float mouseAngleDeg(const sf::Vector2f& center, const sf::Vector2f& mousePos);
};