#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

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

	void beginBackgroundLoad(int albumIndex);            
	bool isReadyToFinalize() const;
	void finalizeLoadedResources();


	void requestNextAlbum();
	 void requestPrevAlbum();
	 bool hasPendingAlbumRequest() const;
	 int consumePendingAlbumRequest();                   

	 int getCurrentAlbumIndex() const;

private:
	struct Album {
		std::string title;
		std::string texturePath;
		std::string soundPath;
	};

	void populateAlbums();
	void stopPlaybackIfPlaying();

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
	bool active = false;


	sf::Texture albumTexture;
	sf::Sprite albumSprite;
	sf::SoundBuffer albumBuffer;
	sf::Sound albumSound;


	sf::Texture vinylTexture;
	sf::Sprite vinylSprite;
	float vinylScale = 0.6f;
	float vinylRadius = 120.0f;
	float vinylSpinSpeed = 30.0f; 

	float albumRadius = 120.0f;
	float albumScale = 1.0f;
	float albumVolume = 50.0f;

	sf::Clock frameClock;

	sf::Font font;
	sf::Text albumText;
	sf::Text fpsText;

	float fpsAccum = 0.0f;
	int fpsFrameCount = 0;
	float fpsUpdateInterval = 0.5f;
	int fpsValue = 0;

	std::vector<Album> albums;
	int currentAlbumIndex = 0;

	std::thread loaderThread;
	mutable std::mutex pendingMutex;
	sf::Image pendingAlbumImage;                  
	sf::SoundBuffer pendingSoundBuffer;            
	std::atomic_bool pendingSoundBufferValid{ false };

	std::atomic_bool loadingInProgress{ false };
	std::atomic_bool loadingFinished{ false };
	std::atomic_bool resourcesFinalized{ false };
	

	std::atomic<int> pendingRequestedAlbumIndex{ -1 }; 
	std::atomic<int> loadingAlbumIndex{ -1 };          

	std::vector<ParallaxLayer> parallaxLayers;
	float parallaxBaseSpeed = 20.0f;
	int assetLoadDelayMs = 500;
};