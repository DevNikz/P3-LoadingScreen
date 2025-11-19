#include "MusicPlayerScene.h"

MusicPlayerScene::MusicPlayerScene(sf::RenderWindow* window) : window(window) {

	//Load album texture
	const std::string albumTexturePath = "Media/Textures/MichaelBuble-Christmas(2011)-Cover.png";
	if (!albumTexture.loadFromFile(albumTexturePath)) {
		std::cerr << "MusicPlayerScene: failed to load album texture: " << albumTexturePath << '\n';
	}
	else {
		albumSprite.setTexture(albumTexture);
		sf::FloatRect b = albumSprite.getLocalBounds();
		albumSprite.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
		albumSprite.setScale(albumScale, albumScale);
		albumRadius = (std::max(b.width, b.height) * albumScale) / 2.0f;
	}

	//Load album song
	/*
	const std::string albumMusicPath = "Media/Music/";
	if (!albumMusic.openFromFile(albumMusicPath)) {
		std::cerr << "MusicPlayerScene: failed to load album music: " << albumMusicPath << '\n';
	}
	else {
		albumMusic.setVolume(albumVolume);
		albumMusic.setLoop(true);
	}
	*/

	//Load font
	const std::string fontPath = "Media/Sansation.ttf";
	if (!font.loadFromFile(fontPath)) {
		std::cerr << "LoadingScene: failed to load font: " << fontPath << '\n';
	}

	albumText.setFont(font);
	albumText.setCharacterSize(28);
	albumText.setFillColor(sf::Color::White);
	albumText.setString("Now Playing: Christmas");

	
}

MusicPlayerScene::~MusicPlayerScene() {}

void MusicPlayerScene::start() {
	if (active) return;
	active = true;

	sf::Vector2u ws = window->getSize();
    sf::Vector2f center(static_cast<float>(ws.x) / 2.0f, static_cast<float>(ws.y) / 2.0f);
	albumSprite.setPosition(center);

	if (albumTexture.getSize().x > 0 && albumTexture.getSize().y > 0) {
		sf::FloatRect b = albumSprite.getLocalBounds();
		albumSprite.setScale(albumScale, albumScale);
		albumRadius = (std::max(b.width, b.height) * albumScale) / 2.0f;
	}

	const float extraTextPadding = 50.0f;

	sf::FloatRect loadRect = albumText.getLocalBounds();
	albumText.setOrigin(loadRect.left + loadRect.width / 2.0f, loadRect.top + loadRect.height / 2.0f);
	albumText.setPosition(center.x, center.y + albumRadius + extraTextPadding);

	//Music
	const std::string albumMusicPath = "Media/Music/Christmas.wav";
	if (!albumMusic.openFromFile(albumMusicPath)) {
		std::cerr << "MusicPlayerScene: failed to load album music: " << albumMusicPath << '\n';
	}
	else {
		albumMusic.setVolume(albumVolume);
		albumMusic.setLoop(true);
		albumMusic.play();
	}

	

	//sf::Music music;

	frameClock.restart();
}

void MusicPlayerScene::stop() {
	active = false;
}

bool MusicPlayerScene::isActive() const {
	return active;
}

void MusicPlayerScene::handleEvent(const sf::Event& event) {
	if (!active) return;

	//albumMusic.play();
}

void MusicPlayerScene::draw() {
	if (!active) return;

	float dt = frameClock.restart().asSeconds();

	sf::Vector2u ws = window->getSize();
	sf::Vector2f center(static_cast<float>(ws.x) / 2.0f, static_cast<float>(ws.y) / 2.0f);
	albumSprite.setPosition(center);

	const float extraTextPadding = 50.0f;
	albumText.setPosition(center.x, center.y + albumRadius + extraTextPadding);

	sf::RectangleShape overlay;
	overlay.setSize(sf::Vector2f(static_cast<float>(ws.x), static_cast<float>(ws.y)));
	overlay.setFillColor(sf::Color(0, 0, 0, 160));
	window->draw(overlay);
	window->draw(albumSprite);
    window->draw(albumText);
}