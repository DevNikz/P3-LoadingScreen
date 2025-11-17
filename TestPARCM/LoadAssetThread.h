#pragma once
#ifndef RTHREAD_H
#define RTHREAD_H

#include "IWorkerAction.h"
#include "IExecutionEvent.h"
#include "TextureManager.h"

class LoadAssetThread : public IWorkerAction {
public:
	LoadAssetThread();
	LoadAssetThread(IExecutionEvent* _callback);
	LoadAssetThread(int _id, IExecutionEvent* _callback);
	~LoadAssetThread();

private:
	int id;

	void OnStartTask() override;
	IExecutionEvent* onFinished;
};

#endif

