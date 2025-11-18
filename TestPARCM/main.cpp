#include <SFML/Graphics.hpp>
#include <iostream>
#include "PlayButtonScene.h"
#include "LoadingScene.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Music Player");
    PlayButtonScene playScene(&window);
    LoadingScene loadingScene(&window);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (loadingScene.isActive()) {
                loadingScene.handleEvent(event);
            } else {
                playScene.handleEvent(event);
            }

            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (!loadingScene.isActive() && playScene.isLoadingRequested()) {
            playScene.clearLoadingRequest();
            loadingScene.start();
        }

        window.clear();

        if (loadingScene.isActive()) {
            loadingScene.draw();
        } else {
            playScene.draw();
        }

        window.display();
    }
}
