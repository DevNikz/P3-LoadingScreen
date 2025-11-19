#include <SFML/Graphics.hpp>
#include <iostream>
#include "PlayButtonScene.h"
#include "LoadingScene.h"
#include "MusicPlayerScene.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Music Player");
    window.setFramerateLimit(120); 
    PlayButtonScene playScene(&window);
    LoadingScene loadingScene(&window);
    MusicPlayerScene musicPlayerScene(&window);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (loadingScene.isActive()) {
                loadingScene.handleEvent(event);
            }
            else if (musicPlayerScene.isActive()) {
                musicPlayerScene.handleEvent(event);
            }
            else {
                playScene.handleEvent(event);
            }

            if (event.type == sf::Event::Closed) window.close();
        }

        if (!loadingScene.isActive() && playScene.isLoadingRequested()) {
            playScene.clearLoadingRequest();
            loadingScene.start();
            musicPlayerScene.beginBackgroundLoad(musicPlayerScene.getCurrentAlbumIndex());
        }

        if (!loadingScene.isActive() && musicPlayerScene.hasPendingAlbumRequest()) {
            int idx = musicPlayerScene.consumePendingAlbumRequest();
            if (idx >= 0) {
                loadingScene.start();
                musicPlayerScene.beginBackgroundLoad(idx);
            }
        }

        if (loadingScene.isActive() && musicPlayerScene.isReadyToFinalize()) {
            musicPlayerScene.finalizeLoadedResources();
            loadingScene.stop();
            musicPlayerScene.start();
        }

        window.clear();

        if (loadingScene.isActive()) loadingScene.draw();
        else if (musicPlayerScene.isActive()) musicPlayerScene.draw();
        else playScene.draw();

        window.display();
    }
}