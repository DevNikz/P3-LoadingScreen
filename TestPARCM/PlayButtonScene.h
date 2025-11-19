// PlayButtonScene.h
#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "AGameObject.h"

class PlayButtonScene {
public:
    PlayButtonScene(sf::RenderWindow* window);
    ~PlayButtonScene();
    void handleEvent(const sf::Event& event);
    void draw();

    bool isLoadingRequested() const;
    void clearLoadingRequest();

private:
    struct ParallaxLayer {
        sf::Texture texture;
        sf::Sprite spriteA;
        sf::Sprite spriteB;
        float speed = 0.0f;
        float scale = 1.0f;
        bool valid = false;
    };

    void loadParallaxLayers(const std::string& folderPath, int count);
    void layoutParallaxLayers();

    sf::RenderWindow* window;
    sf::Texture buttonTextureNormal;
    sf::Texture buttonTextureClicked;
    sf::Sprite buttonSprite;
    bool isButtonPressed = false;

    sf::Font font;
    sf::Text titleText;
    sf::Text playText;
    sf::Text fpsText; 

    float fpsAccum = 0.0f;
    int fpsFrameCount = 0;
    float fpsUpdateInterval = 0.5f;
    int fpsValue = 0;

    sf::FloatRect buttonBounds;

    float titlePadding = 48.0f;

    bool loadingRequested = false;

    std::vector<ParallaxLayer> parallaxLayers;
    float parallaxBaseSpeed = 12.0f;

    sf::Clock frameClock;
};