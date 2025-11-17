#include "LoadAssetThread.h"

LoadAssetThread::LoadAssetThread() {}

LoadAssetThread::~LoadAssetThread() {}

LoadAssetThread::LoadAssetThread(IExecutionEvent* _callback) {
	this->id = 0;
	this->onFinished = _callback; 
}

LoadAssetThread::LoadAssetThread(int _id, IExecutionEvent* _callback) {
	this->id = _id;
	this->onFinished = _callback;
}


void LoadAssetThread::OnStartTask() {
	TextureManager::getInstance()->loadStreamingAssets(this->id);
	this->onFinished->OnFinishedExecution();
	//delete this;
}