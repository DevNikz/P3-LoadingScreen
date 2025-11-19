#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <iostream>

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

};

