// PlayButtonScene.cpp
#include "PlayButtonScene.h"
#include <iostream>

PlayButtonScene::PlayButtonScene(sf::RenderWindow* window) : window(window) {
    const std::string normalPath = "Media/UI/Blue/Double/button_rectangle_depth_gradient.png";
    const std::string clickedPath = "Media/UI/Blue/Double/button_rectangle_depth_flat.png";
    const std::string fontPath = "Media/Sansation.ttf";

    if (!buttonTextureNormal.loadFromFile(normalPath)) {
        std::cerr << "Failed to load button normal texture: " << normalPath << '\n';
    }
    if (!buttonTextureClicked.loadFromFile(clickedPath)) {
        std::cerr << "Failed to load button clicked texture: " << clickedPath << '\n';
    }

    if (buttonTextureNormal.getSize().x > 0 && buttonTextureNormal.getSize().y > 0) {
        buttonSprite.setTexture(buttonTextureNormal);
        auto ts = buttonTextureNormal.getSize();
        buttonSprite.setOrigin(static_cast<float>(ts.x) / 2.0f, static_cast<float>(ts.y) / 2.0f);
    }
    else {
        buttonSprite.setTexture(buttonTextureNormal); 
    }


    {
        sf::Vector2u winSize = window->getSize();
        sf::Vector2f centerPos(static_cast<float>(winSize.x) / 2.0f, static_cast<float>(winSize.y) / 2.0f);
        buttonSprite.setPosition(centerPos);
    }

    buttonBounds = buttonSprite.getGlobalBounds();

    if (!font.loadFromFile(fontPath)) {
        std::cerr << "Failed to load font: " << fontPath << '\n';
    }


    titleText.setFont(font);
    titleText.setString("Music Player");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::White);
    sf::FloatRect titleRect = titleText.getLocalBounds();
    titleText.setOrigin(titleRect.left + titleRect.width / 2.0f, titleRect.top + titleRect.height / 2.0f);


    playText.setFont(font);
    playText.setString("Play");
    playText.setCharacterSize(32);
    playText.setFillColor(sf::Color::White);
    sf::FloatRect playRect = playText.getLocalBounds();
    playText.setOrigin(playRect.left + playRect.width / 2.0f, playRect.top + playRect.height / 2.0f);


    buttonBounds = buttonSprite.getGlobalBounds();
    sf::Vector2f btnPos = buttonSprite.getPosition();
    titleText.setPosition(btnPos.x, btnPos.y - (buttonBounds.height / 2.0f) - titlePadding);
    playText.setPosition(btnPos);
}

PlayButtonScene::~PlayButtonScene() {}

void PlayButtonScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
            buttonBounds = buttonSprite.getGlobalBounds();
            if (buttonBounds.contains(mousePos)) {
                isButtonPressed = true;
                if (buttonTextureClicked.getSize().x > 0 && buttonTextureClicked.getSize().y > 0) {
                    sf::Vector2f currentPos = buttonSprite.getPosition();
                    buttonSprite.setTexture(buttonTextureClicked);
                    auto ts = buttonTextureClicked.getSize();
                    buttonSprite.setOrigin(static_cast<float>(ts.x) / 2.0f, static_cast<float>(ts.y) / 2.0f);
                    buttonSprite.setPosition(currentPos);
                }
            }
        }
    }

    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
            buttonBounds = buttonSprite.getGlobalBounds();
            if (isButtonPressed && buttonBounds.contains(mousePos)) {
                loadingRequested = true;
            }

            isButtonPressed = false;
            if (buttonTextureNormal.getSize().x > 0 && buttonTextureNormal.getSize().y > 0) {
                sf::Vector2f currentPos = buttonSprite.getPosition();
                buttonSprite.setTexture(buttonTextureNormal);
                auto ts = buttonTextureNormal.getSize();
                buttonSprite.setOrigin(static_cast<float>(ts.x) / 2.0f, static_cast<float>(ts.y) / 2.0f);
                buttonSprite.setPosition(currentPos);
            }
        }
    }
}

void PlayButtonScene::draw() {
    buttonBounds = buttonSprite.getGlobalBounds();
    sf::Vector2f btnPos = buttonSprite.getPosition();
    playText.setPosition(btnPos);
    titleText.setPosition(btnPos.x, btnPos.y - (buttonBounds.height / 2.0f) - titlePadding);

    window->draw(titleText);
    window->draw(buttonSprite);
    window->draw(playText);
}

bool PlayButtonScene::isLoadingRequested() const {
    return loadingRequested;
}

void PlayButtonScene::clearLoadingRequest() {
    loadingRequested = false;
}