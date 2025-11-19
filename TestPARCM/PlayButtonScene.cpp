// PlayButtonScene.cpp
#include "PlayButtonScene.h"
#include <iostream>

PlayButtonScene::PlayButtonScene(sf::RenderWindow* window) : window(window) {
    loadParallaxLayers("Media/Background/Clouds 4", 4);

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

    fpsText.setFont(font);
    fpsText.setCharacterSize(16);
    fpsText.setFillColor(sf::Color::Yellow);
    fpsText.setPosition(8.0f, 6.0f);
    fpsText.setString("FPS: 0");

    layoutParallaxLayers();

    fpsAccum = 0.0f;
    fpsFrameCount = 0;
    fpsValue = 0;

    frameClock.restart();
}

PlayButtonScene::~PlayButtonScene() {}

void PlayButtonScene::loadParallaxLayers(const std::string& folderPath, int count) {
    parallaxLayers.clear();
    parallaxLayers.resize(count);

    for (int i = 0; i < count; ++i) {
        std::string path = folderPath + "/" + std::to_string(i + 1) + ".png";
        ParallaxLayer &layer = parallaxLayers[i];
        if (!layer.texture.loadFromFile(path)) {
            std::cerr << "PlayButtonScene: failed to load parallax texture: " << path << '\n';
            layer.valid = false;
            continue;
        }
        layer.valid = true;
        layer.spriteA.setTexture(layer.texture);
        layer.spriteB.setTexture(layer.texture);

        layer.speed = parallaxBaseSpeed * (static_cast<float>(i + 1) * 0.6f + 0.4f);
    }
}

void PlayButtonScene::layoutParallaxLayers() {
    if (!window) return;
    sf::Vector2u ws = window->getSize();
    if (ws.y == 0) return;

    for (auto &layer : parallaxLayers) {
        if (!layer.valid) continue;
        sf::Vector2u texSize = layer.texture.getSize();
        if (texSize.y == 0) continue;

        float scale = static_cast<float>(ws.y) / static_cast<float>(texSize.y);
        layer.scale = scale;
        layer.spriteA.setScale(scale, scale);
        layer.spriteB.setScale(scale, scale);

        float w = static_cast<float>(texSize.x) * scale;
        layer.spriteA.setPosition(0.0f, 0.0f);
        layer.spriteB.setPosition(w, 0.0f);
    }
}

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
    float dt = frameClock.restart().asSeconds();

    fpsAccum += dt;
    fpsFrameCount++;
    if (fpsAccum >= fpsUpdateInterval) {
        fpsValue = static_cast<int>(static_cast<float>(fpsFrameCount) / fpsAccum + 0.5f);
        fpsText.setString(std::string("FPS: ") + std::to_string(fpsValue));
        fpsAccum = 0.0f;
        fpsFrameCount = 0;
    }

    for (auto &layer : parallaxLayers) {
        if (!layer.valid) continue;

        float moveX = -layer.speed * dt;
        layer.spriteA.move(moveX, 0.0f);
        layer.spriteB.move(moveX, 0.0f);

        float texW = static_cast<float>(layer.texture.getSize().x) * layer.scale;
        float aRight = layer.spriteA.getPosition().x + texW;
        float bRight = layer.spriteB.getPosition().x + texW;

        if (aRight < 0.0f) {
            float newX = layer.spriteB.getPosition().x + layer.texture.getSize().x * layer.scale;
            layer.spriteA.setPosition(newX, layer.spriteA.getPosition().y);
        }
        if (bRight < 0.0f) {
            float newX = layer.spriteA.getPosition().x + layer.texture.getSize().x * layer.scale;
            layer.spriteB.setPosition(newX, layer.spriteB.getPosition().y);
        }

        window->draw(layer.spriteA);
        window->draw(layer.spriteB);
    }

    window->draw(fpsText);

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