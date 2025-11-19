#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

class MusicPlayerScene
{
public:
	MusicPlayerScene(sf::RenderWindow* window);
	~MusicPlayerScene();

	void start();
	void stop();
	bool isActive() const;

	void handleEvent(const sf::Event& event);
	void draw();


	void beginBackgroundLoad();                   
	bool isReadyToFinalize() const;                
	void finalizeLoadedResources();                

private:
	sf::RenderWindow* window;
	bool active = false;

	sf::Texture albumTexture;
	sf::Sprite albumSprite;
	sf::Music albumMusic;

	float albumRadius = 120.0f;
	float albumScale = 1.0f;
	float albumVolume = 50.0f;

	sf::Clock frameClock;

	sf::Font font;
	sf::Text albumText;

	std::thread loaderThread;
	mutable std::mutex pendingMutex;
	sf::Image pendingAlbumImage;                   
	std::atomic_bool loadingInProgress{false};
	std::atomic_bool loadingFinished{false};
	std::atomic_bool resourcesFinalized{false};

	const std::string albumTexturePath = "Media/Textures/MichaelBuble-Christmas(2011)-Cover.png";
	const std::string albumMusicPath = "Media/Music/Christmas.ogg";
};

