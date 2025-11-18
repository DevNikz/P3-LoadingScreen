// PlayButtonScene.h
#pragma once
#include <SFML/Graphics.hpp>
#include <string>
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
    sf::RenderWindow* window;
    sf::Texture buttonTextureNormal;
    sf::Texture buttonTextureClicked;
    sf::Sprite buttonSprite;
    bool isButtonPressed = false;

    sf::Font font;
    sf::Text titleText;
    sf::Text playText;

    sf::FloatRect buttonBounds;

    float titlePadding = 48.0f;

    bool loadingRequested = false;
};