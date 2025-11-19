#include "MusicPlayerScene.h"
#include <algorithm>
#include <chrono>
#include <thread>

void MusicPlayerScene::loadParallaxLayers(const std::string& folderPath, int count) {
	parallaxLayers.clear();
	parallaxLayers.resize(std::max(0, count));

	for (int i = 0; i < count; ++i) {
		std::string path = folderPath + "/" + std::to_string(i + 1) + ".png";
		ParallaxLayer &layer = parallaxLayers[i];
		if (!layer.texture.loadFromFile(path)) {
			std::cerr << "MusicPlayerScene: failed to load parallax texture: " << path << '\n';
			layer.valid = false;
			continue;
		}
		layer.valid = true;
		layer.spriteA.setTexture(layer.texture);
		layer.spriteB.setTexture(layer.texture);


		layer.speed = parallaxBaseSpeed * (static_cast<float>(i + 1) * 0.6f + 0.4f);
	}
}

void MusicPlayerScene::layoutParallaxLayers() {
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

void MusicPlayerScene::populateAlbums() {
	albums.push_back(Album{
		"Christmas - Michael Buble",
		"Media/Textures/MichaelBuble-Christmas(2011)-Cover.png",
		"Media/Music/Christmas.ogg"
		});

	albums.push_back(Album{
		"Gemini Rights - Steve Lacy",
		"Media/Textures/196589099631.jpg",
		"Media/Music/Steve Lacy - Gemini Rights (Full Album).wav"
		});

	albums.push_back(Album{
		"Twenty One Pilots - Breach",
		"Media/Textures/Twenty_One_Pilots_-_Breach.png",
		"Media/Music/twenty one pilots - Breach (FULL ALBUM) [40iR3-GCurk].ogg"
		});
}

MusicPlayerScene::MusicPlayerScene(sf::RenderWindow* window) : window(window) {
	populateAlbums();

	loadParallaxLayers("Media/Background/Clouds 7", 4);

	const std::string fontPath = "Media/Sansation.ttf";
	if (!font.loadFromFile(fontPath)) {
		std::cerr << "MusicPlayerScene: failed to load font: " << fontPath << '\n';
	}

	albumText.setFont(font);
	albumText.setCharacterSize(28);
	albumText.setFillColor(sf::Color::White);

	fpsText.setFont(font);
	fpsText.setCharacterSize(16);
	fpsText.setFillColor(sf::Color::Yellow);
	fpsText.setPosition(8.0f, 6.0f);
	fpsText.setString("FPS: 0");

	if (!albums.empty()) {
		albumText.setString(std::string("Now Playing: ") + albums[currentAlbumIndex].title);
	}
	else {
		albumText.setString("Now Playing:");
	}

	const std::string vinylPath = "Media/Textures/pngimg.com - vinyl_PNG18.png";
	if (!vinylTexture.loadFromFile(vinylPath)) {
		std::cerr << "MusicPlayerScene: failed to load vinyl texture: " << vinylPath << '\n';
	}
	else {
		vinylSprite.setTexture(vinylTexture);
		sf::FloatRect vb = vinylSprite.getLocalBounds();
		vinylSprite.setOrigin(vb.left + vb.width / 2.0f, vb.top + vb.height / 2.0f);
		vinylSprite.setScale(vinylScale, vinylScale);
		vinylRadius = (std::max(vb.width, vb.height) * vinylScale) / 2.0f;
	}
}

MusicPlayerScene::~MusicPlayerScene() {
	loadingInProgress = false;
	if (loaderThread.joinable()) {
		loaderThread.join();
	}
}

void MusicPlayerScene::stopPlaybackIfPlaying() {
	if (albumSound.getStatus() == sf::Sound::Playing) {
		albumSound.stop();
	}
}

void MusicPlayerScene::beginBackgroundLoad(int albumIndex) {
	if (albumIndex < 0 || albumIndex >= static_cast<int>(albums.size())) {
		std::cerr << "MusicPlayerScene::beginBackgroundLoad: invalid album index " << albumIndex << '\n';
		return;
	}

	bool expected = false;
	if (!loadingInProgress.compare_exchange_strong(expected, true)) {
		return;
	}

	stopPlaybackIfPlaying();

	loadingFinished = false;
	resourcesFinalized = false;
	pendingSoundBufferValid = false;

	loadingAlbumIndex = albumIndex;

	if (loaderThread.joinable()) {
		loaderThread.join();
	}

	Album albumToLoad = albums[albumIndex];

	loaderThread = std::thread([this, albumToLoad]() {
		sf::Image img;
		if (!img.loadFromFile(albumToLoad.texturePath)) {
			std::cerr << "MusicPlayerScene: background loader failed to load image: " << albumToLoad.texturePath << '\n';
		}
		{
			std::lock_guard<std::mutex> lk(pendingMutex);
			pendingAlbumImage = std::move(img);
		}


		if (assetLoadDelayMs > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(assetLoadDelayMs));
		}

		sf::SoundBuffer buf;
		if (!buf.loadFromFile(albumToLoad.soundPath)) {
			std::cerr << "MusicPlayerScene: background loader failed to decode sound: " << albumToLoad.soundPath << '\n';
			pendingSoundBufferValid = false;
		}
		else {
			std::lock_guard<std::mutex> lk(pendingMutex);
			pendingSoundBuffer = std::move(buf);
			pendingSoundBufferValid = true;
		}


		if (assetLoadDelayMs > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(assetLoadDelayMs));
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

	stopPlaybackIfPlaying();

	sf::Image img;
	{
		std::lock_guard<std::mutex> lk(pendingMutex);
		img = std::move(pendingAlbumImage);
		pendingAlbumImage = sf::Image(); 
	}

	if (img.getSize().x > 0 && img.getSize().y > 0) {
		if (!albumTexture.loadFromImage(img)) {
			std::cerr << "MusicPlayerScene: finalize failed to create texture from image\n";
		}
		else {
			albumSprite.setTexture(albumTexture);
			sf::FloatRect b = albumSprite.getLocalBounds();
			albumSprite.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
			albumSprite.setScale(albumScale, albumScale);
			albumRadius = (std::max(b.width, b.height) * albumScale) / 2.0f;
		}
	}
	else {
		int idx = loadingAlbumIndex.load();
		const std::string fallbackTex = (idx >= 0 && idx < static_cast<int>(albums.size())) ? albums[idx].texturePath : std::string();
		if (!fallbackTex.empty() && !albumTexture.loadFromFile(fallbackTex)) {
			std::cerr << "MusicPlayerScene: fallback failed to load album texture: " << fallbackTex << '\n';
		}
		else if (!fallbackTex.empty()) {
			albumSprite.setTexture(albumTexture);
			sf::FloatRect b = albumSprite.getLocalBounds();
			albumSprite.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
			albumSprite.setScale(albumScale, albumScale);
			albumRadius = (std::max(b.width, b.height) * albumScale) / 2.0f;
		}
	}

	bool bufferValid = pendingSoundBufferValid.load();
	{
		std::lock_guard<std::mutex> lk(pendingMutex);
		if (bufferValid) {
			albumBuffer = std::move(pendingSoundBuffer);
			pendingSoundBuffer = sf::SoundBuffer();
		}
		pendingSoundBufferValid = false;
	}

	if (bufferValid && albumBuffer.getSampleCount() > 0) {
		albumSound.setBuffer(albumBuffer);
		albumSound.setVolume(albumVolume);
		albumSound.setLoop(true);

		albumSound.play();
		std::cerr << "MusicPlayerScene: albumSound.play() called; sampleCount=" << albumBuffer.getSampleCount()
			<< " channels=" << albumBuffer.getChannelCount()
			<< " sampleRate=" << albumBuffer.getSampleRate()
			<< " status=" << static_cast<int>(albumSound.getStatus()) << '\n';
	}
	else {
		albumSound.resetBuffer();
		std::cerr << "MusicPlayerScene: no valid buffer to play after finalize\n";
	}

	int loadedIndex = loadingAlbumIndex.load();
	if (loadedIndex >= 0 && loadedIndex < static_cast<int>(albums.size())) {
		currentAlbumIndex = loadedIndex;
		albumText.setString(std::string("Now Playing: ") + albums[currentAlbumIndex].title);
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

	layoutParallaxLayers();

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

	if (vinylTexture.getSize().x > 0 && vinylTexture.getSize().y > 0) {
		vinylSprite.setPosition(center);
		vinylSprite.setScale(vinylScale, vinylScale);
	}

	if (resourcesFinalized.load() && albumBuffer.getSampleCount() > 0) {
		if (albumSound.getStatus() != sf::Sound::Playing) {
			albumSound.play();
		}
		std::cerr << "MusicPlayerScene::start: resourcesFinalized=" << resourcesFinalized.load()
			<< " sampleCount=" << albumBuffer.getSampleCount()
			<< " soundStatus=" << static_cast<int>(albumSound.getStatus()) << '\n';
	}
	else {
		std::cerr << "MusicPlayerScene::start: no buffer available to play (resourcesFinalized=" << resourcesFinalized.load()
			<< ", sampleCount=" << albumBuffer.getSampleCount() << ")\n";
	}

	fpsAccum = 0.0f;
	fpsFrameCount = 0;
	fpsValue = 0;
	fpsText.setString("FPS: 0");

	frameClock.restart();
}

void MusicPlayerScene::stop() {
	active = false;
	if (albumSound.getStatus() == sf::Sound::Playing) {
		albumSound.stop();
	}
}

bool MusicPlayerScene::isActive() const {
	return active;
}

void MusicPlayerScene::requestNextAlbum() {
	if (albums.empty()) return;
	int next = (currentAlbumIndex + 1) % static_cast<int>(albums.size());
	pendingRequestedAlbumIndex = next;
}

void MusicPlayerScene::requestPrevAlbum() {
	if (albums.empty()) return;
	int prev = (currentAlbumIndex - 1 + static_cast<int>(albums.size())) % static_cast<int>(albums.size());
	pendingRequestedAlbumIndex = prev;
}

bool MusicPlayerScene::hasPendingAlbumRequest() const {
	return pendingRequestedAlbumIndex.load() != -1;
}

int MusicPlayerScene::consumePendingAlbumRequest() {
	return pendingRequestedAlbumIndex.exchange(-1);
}

int MusicPlayerScene::getCurrentAlbumIndex() const {
	return currentAlbumIndex;
}

void MusicPlayerScene::handleEvent(const sf::Event& event) {
	if (!active) return;

	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::Right) {
			requestNextAlbum();
			return;
		}
		if (event.key.code == sf::Keyboard::Left) {
			requestPrevAlbum();
			return;
		}
	}
}

void MusicPlayerScene::draw() {
	if (!active) return;

	float dt = frameClock.restart().asSeconds();

	fpsAccum += dt;
	fpsFrameCount++;
	if (fpsAccum >= fpsUpdateInterval) {
		fpsValue = static_cast<int>(static_cast<float>(fpsFrameCount) / fpsAccum + 0.5f);
		fpsText.setString(std::string("FPS: ") + std::to_string(fpsValue));
		fpsAccum = 0.0f;
		fpsFrameCount = 0;
	}

	sf::Vector2u ws = window->getSize();
	sf::Vector2f center(static_cast<float>(ws.x) / 2.0f, static_cast<float>(ws.y) / 2.0f);

	for (auto &layer : parallaxLayers) {
		if (!layer.valid) continue;

		float moveX = -layer.speed * dt;
		layer.spriteA.move(moveX, 0.0f);
		layer.spriteB.move(moveX, 0.0f);

		float texW = static_cast<float>(layer.texture.getSize().x) * layer.scale;
		float aRight = layer.spriteA.getPosition().x + texW;
		float bRight = layer.spriteB.getPosition().x + texW;

		if (aRight < 0.0f) {
			float newX = layer.spriteB.getPosition().x + texW;
			layer.spriteA.setPosition(newX, layer.spriteA.getPosition().y);
		}
		if (bRight < 0.0f) {
			float newX = layer.spriteA.getPosition().x + texW;
			layer.spriteB.setPosition(newX, layer.spriteB.getPosition().y);
		}

		window->draw(layer.spriteA);
		window->draw(layer.spriteB);
	}

	if (vinylTexture.getSize().x > 0 && vinylTexture.getSize().y > 0) {
		vinylSprite.setPosition(center);
		vinylSprite.setRotation(vinylSprite.getRotation() + vinylSpinSpeed * dt);
	}

	albumSprite.setPosition(center);

	const float extraTextPadding = 50.0f;
	albumText.setPosition(center.x, center.y + albumRadius + extraTextPadding);

	sf::RectangleShape overlay;
	overlay.setSize(sf::Vector2f(static_cast<float>(ws.x), static_cast<float>(ws.y)));
	overlay.setFillColor(sf::Color(0, 0, 0, 160));
	window->draw(overlay);

	window->draw(fpsText);

	if (vinylTexture.getSize().x > 0 && vinylTexture.getSize().y > 0) {
		window->draw(vinylSprite);
	}
	window->draw(albumSprite);
	window->draw(albumText);
}