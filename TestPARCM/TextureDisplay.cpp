#include "TextureDisplay.h"
#include <iostream>
#include "TextureManager.h"
#include "BaseRunner.h"
#include "GameObjectManager.h"
#include "IconObject.h"
TextureDisplay::TextureDisplay(): AGameObject("TextureDisplay")
{
	
}

void TextureDisplay::initialize()
{
	threadPool.StartScheduling();
}

void TextureDisplay::processInput(sf::Event event)
{
	
}

void TextureDisplay::OnFinishedExecution() {
	//this->spawnObject();

	//Spawning by batches
	int start = 0; int end = 0;
	for (int i = 0; i < 4; i++) {
		IETThread::sleep(1000);
		start = i * (TextureManager::getInstance()->getNumLoadedStreamTextures() / 4);
		end = start + (TextureManager::getInstance()->getNumLoadedStreamTextures() / 4);
		for (int j = start; j < end; j++) this->spawnObject();
	}
}

void TextureDisplay::update(sf::Time deltaTime)
{
	this->ticks += BaseRunner::TIME_PER_FRAME.asMilliseconds();

	//Single Streaming
	/*
	if (ticks > STREAMING_LOAD_DELAY) {
		int texCount = TextureManager::getInstance()->getNumLoadedStreamTextures();

		if (texCount < 200) {
			std::cout << texCount << std::endl;
			LoadAssetThread* asset = new LoadAssetThread(texCount, this);
			threadPool.ScheduleTask(asset);
		}
		
		ticks = 0;
	}
	*/

	//Batch Loader
	if (!this->startedStreaming && ticks > STREAMING_LOAD_DELAY) {
		this->startedStreaming = true;
		this->ticks = 0.0f;
		LoadAssetThread* batch = new LoadAssetThread(150, this);
		threadPool.ScheduleTask(batch);
	}
}


void TextureDisplay::spawnObject()
{
	String objectName = "Icon_" + to_string(this->iconList.size());
	IconObject* iconObj = new IconObject(objectName, this->iconList.size());
	this->iconList.push_back(iconObj);

	//set position
	int IMG_WIDTH = 68; int IMG_HEIGHT = 68;
	float x = this->columnGrid * IMG_WIDTH;
	float y = this->rowGrid * IMG_HEIGHT;
	iconObj->setPosition(x, y);

	std::cout << "Set position: " << x << " " << y << std::endl;

	this->columnGrid++;
	if(this->columnGrid == this->MAX_COLUMN)
	{
		this->columnGrid = 0;
		this->rowGrid++;
	}
	GameObjectManager::getInstance()->addObject(iconObj);
}