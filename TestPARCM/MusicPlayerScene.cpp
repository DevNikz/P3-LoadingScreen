#include "MusicPlayerScene.h"
#include <algorithm>

MusicPlayerScene::MusicPlayerScene(sf::RenderWindow* window) : window(window) {


	const std::string fontPath = "Media/Sansation.ttf";
	if (!font.loadFromFile(fontPath)) {
		std::cerr << "MusicPlayerScene: failed to load font: " << fontPath << '\n';
	}

	albumText.setFont(font);
	albumText.setCharacterSize(28);
	albumText.setFillColor(sf::Color::White);
	albumText.setString("Now Playing: Christmas");
}

MusicPlayerScene::~MusicPlayerScene() {
	if (loaderThread.joinable()) {
		loadingInProgress = false;
		loaderThread.join();
	}
}

void MusicPlayerScene::beginBackgroundLoad() {
	bool expected = false;
	if (!loadingInProgress.compare_exchange_strong(expected, true)) {
		return;
	}

	loadingFinished = false;
	resourcesFinalized = false;

	loaderThread = std::thread([this]() {
		sf::Image img;
		if (!img.loadFromFile(albumTexturePath)) {
			std::cerr << "MusicPlayerScene: background loader failed to load image: " << albumTexturePath << '\n';
		}
		{
			std::lock_guard<std::mutex> lk(pendingMutex);
			pendingAlbumImage = std::move(img);
		}

		loadingFinished = true;
		loadingInProgress = false;
	});
}

bool MusicPlayerScene::isReadyToFinalize() const {
	return loadingFinished.load() && !resourcesFinalized.load();
}

void MusicPlayerScene::finalizeLoadedResources() {
	if (!isReadyToFinalize()) return;

	sf::Image img;
	{
		std::lock_guard<std::mutex> lk(pendingMutex);
		img = std::move(pendingAlbumImage);
		pendingAlbumImage = sf::Image(); 
	}

	if (img.getSize().x > 0 && img.getSize().y > 0) {
		if (!albumTexture.loadFromImage(img)) {
			std::cerr << "MusicPlayerScene: finalize failed to create texture from image\n";
		} else {
			albumSprite.setTexture(albumTexture);
			sf::FloatRect b = albumSprite.getLocalBounds();
			albumSprite.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
			albumSprite.setScale(albumScale, albumScale);
			albumRadius = (std::max(b.width, b.height) * albumScale) / 2.0f;
		}
	} else {
		if (!albumTexture.loadFromFile(albumTexturePath)) {
			std::cerr << "MusicPlayerScene: fallback failed to load album texture: " << albumTexturePath << '\n';
		} else {
			albumSprite.setTexture(albumTexture);
			sf::FloatRect b = albumSprite.getLocalBounds();
			albumSprite.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
			albumSprite.setScale(albumScale, albumScale);
			albumRadius = (std::max(b.width, b.height) * albumScale) / 2.0f;
		}
	}

	if (!albumMusic.openFromFile(albumMusicPath)) {
		std::cerr << "MusicPlayerScene: failed to load album music: " << albumMusicPath << '\n';
	} else {
		albumMusic.setVolume(albumVolume);
		albumMusic.setLoop(true);
	}

	resourcesFinalized = true;

	if (loaderThread.joinable()) {
		loaderThread.join();
	}
}

void MusicPlayerScene::start() {
	if (active) return;

	if (!resourcesFinalized.load()) {
		finalizeLoadedResources();
	}

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

	if (resourcesFinalized.load()) {
		albumMusic.play();
	}

	frameClock.restart();
}

void MusicPlayerScene::stop() {
	active = false;
	if (albumMusic.getStatus() == sf::Music::Playing) {
		albumMusic.stop();
	}
}

bool MusicPlayerScene::isActive() const {
	return active;
}

void MusicPlayerScene::handleEvent(const sf::Event& /*event*/) {
	if (!active) return;

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